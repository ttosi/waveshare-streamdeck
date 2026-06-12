#pragma once

#include <Arduino.h>

#include "config/weather_config.h"

struct WeatherDay {
    char date[11];
    int weather_code;
    int precipitation_probability;
    float high;
    float low;
};

struct WeatherSnapshot {
    bool valid;
    bool connected;
    bool is_day;
    char status[64];
    uint32_t updated_at;
    float temperature;
    float humidity;
    float dew_point;
    float wind_speed;
    int wind_direction;
    int weather_code;
    WeatherDay days[weather_config::FORECAST_DAY_COUNT];
    float hourly_temperatures[weather_config::HOURLY_POINT_COUNT];
};

void weather_service_start();
bool weather_service_get_snapshot(WeatherSnapshot *snapshot);
const char *weather_condition_name(int weather_code);
const char *weather_icon_path(int weather_code, bool is_day);
