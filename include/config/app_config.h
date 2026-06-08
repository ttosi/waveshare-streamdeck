#pragma once

#include <Arduino.h>

namespace app_config {

constexpr uint32_t SCREEN_TIMEOUT_MS = 5 * 60 * 1000;
constexpr uint32_t WAKE_TOUCH_GUARD_MS = 750;
constexpr uint32_t SCREEN_TIMEOUT_CHECK_MS = 250;

constexpr uint32_t COLOR_BACKGROUND = 0x111318;
constexpr uint32_t COLOR_KEY = 0x272B33;
constexpr uint32_t COLOR_KEY_PRESSED = 0x3A404B;
constexpr uint32_t COLOR_KEY_BORDER = 0x454B56;
constexpr uint32_t COLOR_ACCENT = 0x3B82F6;
constexpr uint32_t COLOR_ACCENT_PRESSED = 0x2563EB;
constexpr uint32_t COLOR_ACCENT_BORDER = 0x60A5FA;
constexpr uint32_t COLOR_TEXT = 0xF4F7FB;

} // namespace app_config
