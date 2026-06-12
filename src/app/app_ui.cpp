#include "app/app_ui.h"

#include <lvgl.h>

#include "config/app_config.h"
#include "pages/blender_page.h"
#include "pages/inkscape_page.h"
#include "pages/system_page.h"
#include "pages/weather_page.h"
#include "services/screen_power.h"
#include "ui/ui_common.h"

using PageBuilder = void (*)(lv_obj_t *page, USBHIDKeyboard *keyboard);

struct PageDefinition {
    PageBuilder build;
};

static void build_blender_page(lv_obj_t *page, USBHIDKeyboard *keyboard)
{
    create_blender_page(page, keyboard);
}

static void build_system_page(lv_obj_t *page, USBHIDKeyboard *keyboard)
{
    create_system_page(page, keyboard);
}

static void build_inkscape_page(lv_obj_t *page, USBHIDKeyboard *keyboard)
{
    create_inkscape_page(page, keyboard);
}

static void build_weather_page(lv_obj_t *page, USBHIDKeyboard *)
{
    create_weather_page(page);
}

// Add new page builders here. Navigation directions are calculated automatically.
static const PageDefinition pages[] = {
    {build_weather_page},
    {build_system_page},
    {build_blender_page},
    {build_inkscape_page},
};

static lv_obj_t *weather_page;

static void tileview_page_changed_cb(lv_event_t *event)
{
    lv_obj_t *tileview = lv_event_get_target(event);
    screen_power_set_timeout_enabled(lv_tileview_get_tile_act(tileview) != weather_page);
}

static lv_dir_t get_page_directions(size_t page_index, size_t page_count)
{
    uint8_t directions = LV_DIR_NONE;
    if (page_index > 0) {
        directions |= LV_DIR_LEFT;
    }
    if (page_index + 1 < page_count) {
        directions |= LV_DIR_RIGHT;
    }
    return static_cast<lv_dir_t>(directions);
}

void create_app_ui(USBHIDKeyboard *keyboard)
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(app_config::COLOR_BACKGROUND), 0);
    lv_obj_add_event_cb(screen, screen_power_event_cb, LV_EVENT_PRESSED, nullptr);

    lv_obj_t *tileview = lv_tileview_create(screen);
    lv_obj_set_style_bg_color(tileview, lv_color_hex(app_config::COLOR_BACKGROUND), 0);
    lv_obj_set_style_pad_all(tileview, 0, 0);
    lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(tileview, screen_power_event_cb, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(tileview, tileview_page_changed_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    constexpr size_t page_count = sizeof(pages) / sizeof(pages[0]);
    for (size_t page_index = 0; page_index < page_count; page_index++) {
        lv_obj_t *page = lv_tileview_add_tile(
            tileview, page_index, 0, get_page_directions(page_index, page_count)
        );
        if (page_index == 0) {
            weather_page = page;
        }
        configure_page(page);
        pages[page_index].build(page, keyboard);
    }

    screen_power_start_timeout();
    screen_power_set_timeout_enabled(false);
}
