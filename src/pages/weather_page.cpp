#include "pages/weather_page.h"

#include <math.h>
#include <time.h>

#include "config/app_config.h"
#include "config/weather_config.h"
#include "services/image_assets.h"
#include "services/weather_service.h"

namespace {

struct ForecastWidgets {
    lv_obj_t *day;
    lv_obj_t *icon;
    lv_obj_t *precipitation;
    lv_obj_t *temperatures;
};

lv_obj_t *clock_label;
lv_obj_t *date_label;
lv_obj_t *date_bold_label;
lv_obj_t *status_label;
lv_obj_t *current_icon;
lv_obj_t *temperature_label;
lv_obj_t *condition_label;
lv_obj_t *details_label;
ForecastWidgets forecast[weather_config::FORECAST_DAY_COUNT];

void set_text_color(lv_obj_t *object, uint32_t color)
{
    lv_obj_set_style_text_color(object, lv_color_hex(color), 0);
}

lv_obj_t *create_label(lv_obj_t *parent, const lv_font_t *font, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, font, 0);
    set_text_color(label, color);
    return label;
}

lv_obj_t *create_card(lv_obj_t *parent)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_style_bg_color(card, lv_color_hex(app_config::COLOR_KEY), 0);
    lv_obj_set_style_border_color(card, lv_color_hex(app_config::COLOR_KEY_BORDER), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_pad_all(card, 8, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

void update_icon(lv_obj_t *image, const char *path)
{
    const lv_img_dsc_t *asset = load_image_asset(path);
    if (asset != nullptr) {
        lv_img_set_src(image, asset);
    }
}

void format_forecast_day(const char *date, size_t index, char *output, size_t output_size)
{
    if (index == 0) {
        strlcpy(output, "TODAY", output_size);
        return;
    }

    struct tm value = {};
    if (sscanf(date, "%d-%d-%d", &value.tm_year, &value.tm_mon, &value.tm_mday) != 3) {
        strlcpy(output, "---", output_size);
        return;
    }
    value.tm_year -= 1900;
    value.tm_mon -= 1;
    mktime(&value);
    strftime(output, output_size, "%a", &value);
    for (size_t character = 0; output[character] != '\0'; character++) {
        output[character] = toupper(static_cast<unsigned char>(output[character]));
    }
}

void refresh_page(lv_timer_t *)
{
    const time_t now = time(nullptr);
    struct tm local = {};
    if (now > 100000 && localtime_r(&now, &local) != nullptr) {
        char clock[16];
        char date[32];
        int hour = local.tm_hour % 12;
        if (hour == 0) {
            hour = 12;
        }
        lv_snprintf(clock, sizeof(clock), "%d:%02d %s", hour, local.tm_min, local.tm_hour < 12 ? "AM" : "PM");
        char weekday[12];
        char month[12];
        strftime(weekday, sizeof(weekday), "%A", &local);
        strftime(month, sizeof(month), "%B", &local);
        lv_snprintf(date, sizeof(date), "%s, %s %d", weekday, month, local.tm_mday);
        lv_label_set_text(clock_label, clock);
        lv_label_set_text(date_label, date);
        lv_label_set_text(date_bold_label, date);
    }

    WeatherSnapshot snapshot;
    if (!weather_service_get_snapshot(&snapshot)) {
        return;
    }
    if (!snapshot.valid) {
        lv_label_set_text(
            status_label,
            snapshot.status[0] != '\0'
                ? snapshot.status
                : (snapshot.connected ? "Updating weather..." : "Connecting to Wi-Fi...")
        );
        return;
    }

    char buffer[64];
    const int temperature = lroundf(snapshot.temperature);
    const int high = lroundf(snapshot.days[0].high);
    const int low = lroundf(snapshot.days[0].low);
    const int humidity = lroundf(snapshot.humidity);
    const int dew_point = lroundf(snapshot.dew_point);
    lv_label_set_text(
        status_label,
        snapshot.connected
            ? snapshot.status
            : "Offline - showing last update"
    );
    update_icon(current_icon, weather_icon_path(snapshot.weather_code, snapshot.is_day));
    lv_snprintf(buffer, sizeof(buffer), "%d°", temperature);
    lv_label_set_text(temperature_label, buffer);
    lv_label_set_text(condition_label, weather_condition_name(snapshot.weather_code));
    lv_snprintf(
        buffer, sizeof(buffer), "H %d°  L %d°     Humidity %d%%  Dew %d°",
        high, low, humidity, dew_point
    );
    lv_label_set_text(details_label, buffer);

    for (size_t index = 0; index < weather_config::FORECAST_DAY_COUNT; index++) {
        char day[8];
        format_forecast_day(snapshot.days[index].date, index, day, sizeof(day));
        lv_label_set_text(forecast[index].day, day);
        update_icon(forecast[index].icon, weather_icon_path(snapshot.days[index].weather_code, true));
        lv_snprintf(buffer, sizeof(buffer), "POP %d%%", snapshot.days[index].precipitation_probability);
        lv_label_set_text(forecast[index].precipitation, buffer);
        lv_snprintf(
            buffer, sizeof(buffer), "%d° / %d°",
            lroundf(snapshot.days[index].high), lroundf(snapshot.days[index].low)
        );
        lv_label_set_text(forecast[index].temperatures, buffer);
    }
}

} // namespace

void create_weather_page(lv_obj_t *page)
{
    clock_label = create_label(page, &lv_font_montserrat_48, app_config::COLOR_TEXT);
    lv_obj_align(clock_label, LV_ALIGN_TOP_LEFT, 33, 18);
    lv_label_set_text(clock_label, "--:-- --");

    date_label = create_label(page, &lv_font_montserrat_16, 0xAEB6C4);
    lv_obj_align(date_label, LV_ALIGN_TOP_RIGHT, -28, 32);
    lv_label_set_text(date_label, "Waiting for time...");

    date_bold_label = create_label(page, &lv_font_montserrat_16, 0xAEB6C4);
    lv_obj_align(date_bold_label, LV_ALIGN_TOP_RIGHT, -27, 32);
    lv_label_set_text(date_bold_label, "Waiting for time...");

    status_label = create_label(page, &lv_font_montserrat_12, 0x8C96A8);
    lv_obj_align(status_label, LV_ALIGN_TOP_RIGHT, -28, 58);
    lv_label_set_text(status_label, "Connecting to Wi-Fi...");

    lv_obj_t *current_card = create_card(page);
    lv_obj_set_size(current_card, 744, 172);
    lv_obj_align(current_card, LV_ALIGN_TOP_MID, 0, 92);

    current_icon = lv_img_create(current_card);
    lv_obj_align(current_icon, LV_ALIGN_LEFT_MID, 18, 0);

    temperature_label = create_label(current_card, &lv_font_montserrat_48, app_config::COLOR_TEXT);
    lv_obj_align(temperature_label, LV_ALIGN_LEFT_MID, 160, -30);
    lv_label_set_text(temperature_label, "--°");

    condition_label = create_label(current_card, &lv_font_montserrat_16, app_config::COLOR_TEXT);
    lv_obj_align(condition_label, LV_ALIGN_LEFT_MID, 164, 25);
    lv_label_set_text(condition_label, "Waiting for weather...");

    details_label = create_label(current_card, &lv_font_montserrat_16, 0xAEB6C4);
    lv_obj_align(details_label, LV_ALIGN_BOTTOM_LEFT, 164, -12);
    lv_label_set_text(details_label, "H --°  L --°     Humidity --%  Dew --°");

    constexpr int card_width = 174;
    constexpr int card_gap = 16;
    constexpr int row_width = card_width * weather_config::FORECAST_DAY_COUNT
        + card_gap * (weather_config::FORECAST_DAY_COUNT - 1);
    for (size_t index = 0; index < weather_config::FORECAST_DAY_COUNT; index++) {
        lv_obj_t *card = create_card(page);
        lv_obj_set_size(card, card_width, 174);
        lv_obj_align(card, LV_ALIGN_BOTTOM_LEFT, (800 - row_width) / 2 + index * (card_width + card_gap), -24);

        forecast[index].day = create_label(card, &lv_font_montserrat_14, 0xAEB6C4);
        lv_obj_align(forecast[index].day, LV_ALIGN_TOP_MID, 0, 2);
        lv_label_set_text(forecast[index].day, index == 0 ? "TODAY" : "---");

        forecast[index].icon = lv_img_create(card);
        lv_obj_align(forecast[index].icon, LV_ALIGN_CENTER, 0, -4);

        forecast[index].precipitation = create_label(card, &lv_font_montserrat_12, 0x8AB4F8);
        lv_obj_align(forecast[index].precipitation, LV_ALIGN_BOTTOM_MID, 0, -27);
        lv_label_set_text(forecast[index].precipitation, "POP --%");

        forecast[index].temperatures = create_label(card, &lv_font_montserrat_16, app_config::COLOR_TEXT);
        lv_obj_align(forecast[index].temperatures, LV_ALIGN_BOTTOM_MID, 0, -3);
        lv_label_set_text(forecast[index].temperatures, "--° / --°");
    }

    lv_timer_t *timer = lv_timer_create(refresh_page, 1000, nullptr);
    lv_timer_ready(timer);
}
