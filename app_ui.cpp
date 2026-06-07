#include "app_ui.h"

#include <lvgl.h>

#include "app_config.h"
#include "icon_page.h"
#include "numpad_page.h"
#include "screen_power.h"
#include "ui_common.h"

using PageBuilder = void (*)(lv_obj_t *page, USBHIDKeyboard *keyboard);

struct PageDefinition {
    PageBuilder build;
};

static void build_icon_page(lv_obj_t *page, USBHIDKeyboard *keyboard)
{
    create_icon_page(page, keyboard);
}

static void build_numpad_page(lv_obj_t *page, USBHIDKeyboard *keyboard)
{
    create_numpad_page(page, keyboard);
}

// Add new page builders here. Navigation directions are calculated automatically.
static const PageDefinition pages[] = {
    {build_icon_page},
    {build_numpad_page},
};

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

    constexpr size_t page_count = sizeof(pages) / sizeof(pages[0]);
    for (size_t page_index = 0; page_index < page_count; page_index++) {
        lv_obj_t *page = lv_tileview_add_tile(
            tileview, page_index, 0, get_page_directions(page_index, page_count)
        );
        configure_page(page);
        pages[page_index].build(page, keyboard);
    }

    screen_power_start_timeout();
}
