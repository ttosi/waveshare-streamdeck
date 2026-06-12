#include "services/weather_service.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NetworkClient.h>
#include <WiFi.h>
#include <esp_sntp.h>

#include "config/secrets.h"

namespace {

SemaphoreHandle_t snapshot_mutex;
WeatherSnapshot current_snapshot = {};

void set_status(const char *status)
{
    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        strlcpy(current_snapshot.status, status, sizeof(current_snapshot.status));
        xSemaphoreGive(snapshot_mutex);
    }
    log_i("%s", status);
}

bool connect_wifi()
{
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(secrets::WIFI_SSID, secrets::WIFI_PASSPHRASE);

    const uint32_t started_at = millis();
    while (WiFi.status() != WL_CONNECTED
            && millis() - started_at < weather_config::WIFI_CONNECT_TIMEOUT_MS) {
        delay(250);
    }
    const bool connected = WiFi.status() == WL_CONNECTED;
    set_status(connected ? "Wi-Fi connected" : "Wi-Fi connection failed");
    return connected;
}

void set_connected(bool connected)
{
    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        current_snapshot.connected = connected;
        xSemaphoreGive(snapshot_mutex);
    }
}

bool fetch_weather()
{
    set_status("Requesting weather...");

    String url = "http://api.open-meteo.com/v1/forecast?latitude=";
    url += String(weather_config::LATITUDE, 7);
    url += "&longitude=";
    url += String(weather_config::LONGITUDE, 7);
    url += "&current=temperature_2m,relative_humidity_2m,dew_point_2m,weather_code,is_day,wind_speed_10m,wind_direction_10m";
    url += "&hourly=temperature_2m&forecast_hours=12";
    url += "&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max";
    url += "&temperature_unit=fahrenheit&timezone=America%2FChicago&forecast_days=4";

    NetworkClient client;

    HTTPClient http;
    http.setConnectTimeout(10000);
    http.setTimeout(15000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    if (!http.begin(client, url)) {
        set_status("Weather request setup failed");
        return false;
    }

    const int status = http.GET();
    if (status != HTTP_CODE_OK) {
        char message[64];
        snprintf(
            message, sizeof(message), "Weather HTTP error: %d %s",
            status, HTTPClient::errorToString(status).c_str()
        );
        set_status(message);
        http.end();
        return false;
    }

    const String payload = http.getString();
    http.end();
    if (payload.length() == 0) {
        set_status("Weather response was empty");
        return false;
    }

    JsonDocument document;
    const DeserializationError error = deserializeJson(document, payload);
    if (error) {
        char message[64];
        snprintf(
            message, sizeof(message), "Weather JSON error: %s (%u bytes)",
            error.c_str(), static_cast<unsigned>(payload.length())
        );
        set_status(message);
        log_i("Weather response: %.160s", payload.c_str());
        return false;
    }
    if (!document["current"].is<JsonObject>() || !document["daily"].is<JsonObject>()) {
        set_status("Weather response missing data");
        return false;
    }

    WeatherSnapshot next = {};
    next.valid = true;
    next.connected = true;
    next.status[0] = '\0';
    next.updated_at = static_cast<uint32_t>(time(nullptr));
    next.temperature = document["current"]["temperature_2m"] | 0.0F;
    next.humidity = document["current"]["relative_humidity_2m"] | 0.0F;
    next.dew_point = document["current"]["dew_point_2m"] | 0.0F;
    next.wind_speed = document["current"]["wind_speed_10m"] | 0.0F;
    next.wind_direction = document["current"]["wind_direction_10m"] | 0;
    next.weather_code = document["current"]["weather_code"] | 0;
    next.is_day = (document["current"]["is_day"] | 1) != 0;

    for (size_t index = 0; index < weather_config::FORECAST_DAY_COUNT; index++) {
        const char *date = document["daily"]["time"][index] | "";
        strlcpy(next.days[index].date, date, sizeof(next.days[index].date));
        next.days[index].weather_code = document["daily"]["weather_code"][index] | 0;
        next.days[index].precipitation_probability =
            document["daily"]["precipitation_probability_max"][index] | 0;
        next.days[index].high = document["daily"]["temperature_2m_max"][index] | 0.0F;
        next.days[index].low = document["daily"]["temperature_2m_min"][index] | 0.0F;
    }
    for (size_t index = 0; index < weather_config::HOURLY_POINT_COUNT; index++) {
        next.hourly_temperatures[index] = document["hourly"]["temperature_2m"][index] | next.temperature;
    }

    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        current_snapshot = next;
        xSemaphoreGive(snapshot_mutex);
    }
    log_i("Weather updated");
    return true;
}

void weather_task(void *)
{
    uint32_t retry_delay = 0;
    while (true) {
        const bool connected = connect_wifi();
        set_connected(connected);
        if (connected) {
            configTzTime(weather_config::TIMEZONE, weather_config::NTP_SERVER);
            esp_sntp_set_sync_interval(weather_config::REFRESH_INTERVAL_MS);
        }

        const bool updated = connected && fetch_weather();
        retry_delay = updated ? weather_config::REFRESH_INTERVAL_MS : weather_config::RETRY_INTERVAL_MS;
        vTaskDelay(pdMS_TO_TICKS(retry_delay));
    }
}

} // namespace

void weather_service_start()
{
    snapshot_mutex = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(weather_task, "weather", 12288, nullptr, 1, nullptr, 0);
}

bool weather_service_get_snapshot(WeatherSnapshot *snapshot)
{
    if (snapshot == nullptr || snapshot_mutex == nullptr) {
        return false;
    }
    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(20)) != pdTRUE) {
        return false;
    }
    *snapshot = current_snapshot;
    xSemaphoreGive(snapshot_mutex);
    return true;
}

const char *weather_condition_name(int weather_code)
{
    if (weather_code == 0) return "Clear";
    if (weather_code <= 2) return "Partly cloudy";
    if (weather_code == 3) return "Overcast";
    if (weather_code == 45 || weather_code == 48) return "Fog";
    if (weather_code >= 51 && weather_code <= 57) return "Drizzle";
    if (weather_code >= 61 && weather_code <= 67) return "Rain";
    if (weather_code >= 71 && weather_code <= 77) return "Snow";
    if (weather_code >= 80 && weather_code <= 82) return "Showers";
    if (weather_code >= 85 && weather_code <= 86) return "Snow showers";
    if (weather_code >= 95) return "Thunderstorms";
    return "Unknown";
}

const char *weather_icon_path(int weather_code, bool is_day)
{
    if (weather_code == 0) return is_day ? "A:/icons/weather/clear-day.bin" : "A:/icons/weather/clear-night.bin";
    if (weather_code <= 2) return is_day ? "A:/icons/weather/partly-cloudy-day.bin" : "A:/icons/weather/partly-cloudy-night.bin";
    if (weather_code == 3) return "A:/icons/weather/overcast.bin";
    if (weather_code == 45 || weather_code == 48) return "A:/icons/weather/fog.bin";
    if (weather_code >= 51 && weather_code <= 57) return "A:/icons/weather/drizzle.bin";
    if ((weather_code >= 71 && weather_code <= 77) || (weather_code >= 85 && weather_code <= 86)) {
        return "A:/icons/weather/snow.bin";
    }
    if (weather_code >= 95) return "A:/icons/weather/thunderstorms.bin";
    return "A:/icons/weather/rain.bin";
}
