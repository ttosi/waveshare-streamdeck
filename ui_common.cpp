#include "ui_common.h"

#include "app_config.h"
#include "screen_power.h"

void configure_page(lv_obj_t *page)
{
    lv_obj_set_style_bg_color(page, lv_color_hex(app_config::COLOR_BACKGROUND), 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(page, screen_power_event_cb, LV_EVENT_PRESSED, nullptr);
}
