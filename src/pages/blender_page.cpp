#include "pages/blender_page.h"

#include "services/hid_action.h"
#include "services/screen_power.h"
#include "ui/icon_grid.h"

namespace {

struct BlenderButton {
    const char *icon;
    const char *action;
    USBHIDKeyboard *keyboard;
};

BlenderButton buttons[] = {
    {"A:/icons/blender/viewport_x.bin",          "key:KEY_KP_3",                            nullptr},
    {"A:/icons/blender/grab_x.bin",              "key:g|key:x",                             nullptr},
    {"A:/icons/blender/resize_x.bin",            "key:s|key:x",                             nullptr},
    {"A:/icons/blender/add_cube.bin",            "combo:KEY_LEFT_SHIFT+a|type:cube|enter",  nullptr},
    {"A:/icons/blender/cursor_to_origin.bin",    "combo:KEY_LEFT_SHIFT+s|type:1",            nullptr},
    {"A:/icons/blender/viewport_y.bin",          "key:KEY_KP_1",                            nullptr},
    {"A:/icons/blender/grab_y.bin",              "key:g|key:y",                             nullptr},
    {"A:/icons/blender/resize_y.bin",            "key:s|key:y",                             nullptr},
    {"A:/icons/blender/add_cylinder.bin",        "combo:KEY_LEFT_SHIFT+a|type:cylinder|enter", nullptr},
    {"A:/icons/blender/cursor_to_selection.bin", "combo:KEY_LEFT_SHIFT+s|type:2",            nullptr},
    {"A:/icons/blender/viewport_z.bin",          "key:KEY_KP_7",                            nullptr},
    {"A:/icons/blender/grab_z.bin",              "key:g|key:z",                             nullptr},
    {"A:/icons/blender/resize_z.bin",            "key:s|key:z",                             nullptr},
    {"A:/icons/blender/add_plane.bin",           "combo:KEY_LEFT_SHIFT+a|type:plane|enter", nullptr},
    {"A:/icons/blender/selection_to_cursor.bin", "combo:KEY_LEFT_SHIFT+s|type:8",            nullptr},
};

void blender_button_event_cb(lv_event_t *event)
{
    screen_power_note_touch();
    if (screen_power_should_consume_click()) {
        return;
    }

    const BlenderButton *button =
        static_cast<const BlenderButton *>(lv_event_get_user_data(event));
    execute_hid_action(button->keyboard, button->action);
}

} // namespace

void create_blender_page(lv_obj_t *page, USBHIDKeyboard *keyboard)
{
    IconGridItem icons[sizeof(buttons) / sizeof(buttons[0])];
    for (size_t index = 0; index < sizeof(buttons) / sizeof(buttons[0]); index++) {
        buttons[index].keyboard = keyboard;
        icons[index] = {buttons[index].icon, blender_button_event_cb, &buttons[index]};
    }

    static const IconGridConfig grid = {
        5,
        136,
        132,
        12,
        12,
    };
    create_icon_grid(page, icons, sizeof(icons) / sizeof(icons[0]), grid);
}
