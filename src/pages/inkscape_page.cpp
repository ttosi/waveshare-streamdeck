#include "pages/inkscape_page.h"

#include "services/hid_action.h"
#include "services/screen_power.h"
#include "ui/icon_grid.h"

namespace {

struct InkscapeButton {
    const char *icon;
    const char *action;
    USBHIDKeyboard *keyboard;
};

InkscapeButton buttons[] = {
    {"A:/icons/inkscape/align_center_vert.bin",        "combo:KEY_LEFT_CTRL+KEY_LEFT_ALT+7",           nullptr},
    {"A:/icons/inkscape/dialog_export.bin",            "combo:KEY_LEFT_CTRL+KEY_LEFT_SHIFT+E",         nullptr},
    {"A:/icons/inkscape/dialog_history.bin",           "combo:KEY_LEFT_CTRL+KEY_LEFT_SHIFT+H",         nullptr},
    {"A:/icons/inkscape/rectangle.bin",                "key:r",                                        nullptr},
    {"A:/icons/inkscape/dialog_layers.bin",            "combo:KEY_LEFT_CTRL+KEY_LEFT_SHIFT+L",         nullptr},
    {"A:/icons/inkscape/align_center_hort.bin",        "combo:KEY_LEFT_CTRL+KEY_LEFT_ALT+1",           nullptr},
    {"A:/icons/inkscape/dialog_fill_and_stroke.bin",   "combo:KEY_LEFT_CTRL+KEY_LEFT_SHIFT+F",         nullptr},
    {"A:/icons/inkscape/dialog_object_properties.bin", "combo:KEY_LEFT_CTRL+KEY_LEFT_SHIFT+O",         nullptr},
    {"A:/icons/inkscape/ellipse.bin",                  "key:e",                                        nullptr},
    {"A:/icons/inkscape/raise_layer.bin",              "combo:KEY_LEFT_CTRL+KEY_LEFT_SHIFT+KEY_PAGE_UP", nullptr},
    {"A:/icons/inkscape/dialog_align.bin",             "combo:KEY_LEFT_CTRL+KEY_LEFT_SHIFT+A",         nullptr},
    {"A:/icons/inkscape/pages_setup.bin",              "combo:KEY_LEFT_CTRL+KEY_LEFT_SHIFT+P",         nullptr},
    {"A:/icons/inkscape/dialog_import.bin",            "combo:KEY_LEFT_CTRL+I",                        nullptr},
    {"A:/icons/inkscape/3d_object.bin",                "combo:KEY_LEFT_SHIFT+KEY_F4",                  nullptr},
    {"A:/icons/inkscape/lower_layer.bin",              "combo:KEY_LEFT_CTRL+KEY_LEFT_SHIFT+KEY_PAGE_DOWN", nullptr},
};

void inkscape_button_event_cb(lv_event_t *event)
{
    screen_power_note_touch();
    if (screen_power_should_consume_click()) {
        return;
    }

    const InkscapeButton *button =
        static_cast<const InkscapeButton *>(lv_event_get_user_data(event));
    execute_hid_action(button->keyboard, button->action);
}

} // namespace

void create_inkscape_page(lv_obj_t *page, USBHIDKeyboard *keyboard)
{
    IconGridItem icons[sizeof(buttons) / sizeof(buttons[0])];
    for (size_t index = 0; index < sizeof(buttons) / sizeof(buttons[0]); index++) {
        buttons[index].keyboard = keyboard;
        const bool has_action = buttons[index].action != nullptr && buttons[index].action[0] != '\0';
        icons[index] = {
            buttons[index].icon,
            has_action ? inkscape_button_event_cb : nullptr,
            has_action ? &buttons[index] : nullptr,
        };
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
