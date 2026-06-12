#include "services/screen_power.h"

#include "config/app_config.h"

static esp_panel::drivers::Backlight *screen_backlight;
static uint32_t last_touch_ms;
static uint32_t wake_guard_started_ms;
static bool screen_sleeping;
static bool timeout_enabled = true;

static void wake_screen()
{
    if (!screen_sleeping) {
        return;
    }

    screen_backlight->on();
    screen_sleeping = false;
    wake_guard_started_ms = lv_tick_get();
}

static void screen_timeout_cb(lv_timer_t *timer)
{
    (void)timer;

    if (timeout_enabled && !screen_sleeping
            && lv_tick_elaps(last_touch_ms) >= app_config::SCREEN_TIMEOUT_MS) {
        screen_backlight->off();
        screen_sleeping = true;
    }
}

void screen_power_init(esp_panel::drivers::Backlight *backlight)
{
    screen_backlight = backlight;
}

void screen_power_start_timeout()
{
    last_touch_ms = lv_tick_get();
    wake_guard_started_ms = last_touch_ms - app_config::WAKE_TOUCH_GUARD_MS;
    lv_timer_create(screen_timeout_cb, app_config::SCREEN_TIMEOUT_CHECK_MS, nullptr);
}

void screen_power_set_timeout_enabled(bool enabled)
{
    timeout_enabled = enabled;
    last_touch_ms = lv_tick_get();
    if (!enabled) {
        wake_screen();
    }
}

void screen_power_note_touch()
{
    last_touch_ms = lv_tick_get();
    wake_screen();
}

bool screen_power_should_consume_click()
{
    return lv_tick_elaps(wake_guard_started_ms) < app_config::WAKE_TOUCH_GUARD_MS;
}

void screen_power_event_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_PRESSED) {
        screen_power_note_touch();
    }
}
