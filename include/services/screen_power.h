#pragma once

#include <esp_display_panel.hpp>
#include <lvgl.h>

void screen_power_init(esp_panel::drivers::Backlight *backlight);
void screen_power_start_timeout();
void screen_power_set_timeout_enabled(bool enabled);
void screen_power_event_cb(lv_event_t *event);
void screen_power_note_touch();
bool screen_power_should_consume_click();
