#pragma once

#include <Arduino.h>

namespace weather_config {

constexpr double LATITUDE = 44.9630548;
constexpr double LONGITUDE = -93.1688582;

constexpr char TIMEZONE[] = "CST6CDT,M3.2.0,M11.1.0";
constexpr char NTP_SERVER[] = "pool.ntp.org";

constexpr uint32_t REFRESH_INTERVAL_MS = 30UL * 60UL * 1000UL;
constexpr uint32_t RETRY_INTERVAL_MS = 60UL * 1000UL;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 20UL * 1000UL;
constexpr size_t FORECAST_DAY_COUNT = 4;
constexpr size_t HOURLY_POINT_COUNT = 12;

} // namespace weather_config
