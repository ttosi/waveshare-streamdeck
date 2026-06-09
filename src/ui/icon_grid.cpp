#include "ui/icon_grid.h"

#include "services/image_assets.h"

void create_icon_grid(
    lv_obj_t *page, const IconGridItem *items, size_t item_count, const IconGridConfig &config
)
{
    if (item_count == 0 || config.columns == 0) {
        return;
    }

    const size_t rows = (item_count + config.columns - 1) / config.columns;
    const int16_t grid_width =
        config.columns * config.cell_width + (config.columns - 1) * config.column_gap;
    const int16_t grid_height = rows * config.cell_height + (rows - 1) * config.row_gap;

    for (size_t index = 0; index < item_count; index++) {
        if (items[index].image == nullptr || items[index].image[0] == '\0') {
            continue;
        }

        const int16_t column = index % config.columns;
        const int16_t row = index / config.columns;
        const int16_t x =
            column * (config.cell_width + config.column_gap) + config.cell_width / 2 - grid_width / 2;
        const int16_t y =
            row * (config.cell_height + config.row_gap) + config.cell_height / 2 - grid_height / 2;

        lv_obj_t *icon = lv_img_create(page);
        const lv_img_dsc_t *image = load_image_asset(items[index].image);
        lv_img_set_src(icon, image != nullptr ? static_cast<const void *>(image) : items[index].image);
        lv_obj_align(icon, LV_ALIGN_CENTER, x, y);

        if (items[index].event_cb != nullptr) {
            lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(icon, items[index].event_cb, LV_EVENT_CLICKED, items[index].user_data);
        }
    }
}
