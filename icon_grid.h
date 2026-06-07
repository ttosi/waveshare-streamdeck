#pragma once

#include <lvgl.h>

struct IconGridItem {
    const lv_img_dsc_t *image;
    lv_event_cb_t event_cb = nullptr;
    void *user_data = nullptr;
};

struct IconGridConfig {
    uint8_t columns;
    int16_t cell_width;
    int16_t cell_height;
    int16_t column_gap = 0;
    int16_t row_gap = 0;
};

void create_icon_grid(
    lv_obj_t *page, const IconGridItem *items, size_t item_count, const IconGridConfig &config
);
