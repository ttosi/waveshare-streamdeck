#include "pages/system_page.h"

#include "services/hid_action.h"
#include "services/screen_power.h"
#include "ui/icon_grid.h"

namespace {

struct SystemButton {
    const char *icon;
    const char *action;
    USBHIDKeyboard *keyboard;
};

SystemButton buttons[] = {
    {"A:/icons/system/login.bin",     "key:KEY_SPACE|delay:500|key:KEY_DELETE|secret:LOGIN_PASSWORD|enter", nullptr},
    {"A:/icons/system/restart.bin",   "key:KEY_LEFT_GUI|delay:250|type:restart|enter",                       nullptr},
    {"",                              "",                                                                    nullptr},
    {"",                              "",                                                                    nullptr},
    {"",                              "",                                                                    nullptr},
    {"A:/icons/system/logout.bin",    "key:KEY_LEFT_GUI|delay:250|type:logout|enter",                        nullptr},
    {"A:/icons/system/power_off.bin", "key:KEY_LEFT_GUI|delay:250|type:power off|enter",                     nullptr},
    {"",                              "",                                                                    nullptr},
    {"",                              "",                                                                    nullptr},
    {"",                              "",                                                                    nullptr},
    {"A:/icons/system/lock.bin",      "key:KEY_LEFT_GUI|delay:250|type:lock screen|enter",                   nullptr},
    {"",                              "",                                                                    nullptr},
    {"",                              "",                                                                    nullptr},
    {"",                              "",                                                                    nullptr},
    {"",                              "",                                                                    nullptr},
};

void system_button_event_cb(lv_event_t *event)
{
    screen_power_note_touch();
    if (screen_power_should_consume_click()) {
        return;
    }

    const SystemButton *button =
        static_cast<const SystemButton *>(lv_event_get_user_data(event));
    execute_hid_action(button->keyboard, button->action);
}

} // namespace

void create_system_page(lv_obj_t *page, USBHIDKeyboard *keyboard)
{
    IconGridItem icons[sizeof(buttons) / sizeof(buttons[0])];
    for (size_t index = 0; index < sizeof(buttons) / sizeof(buttons[0]); index++) {
        buttons[index].keyboard = keyboard;
        const bool has_action = buttons[index].action != nullptr && buttons[index].action[0] != '\0';
        icons[index] = {
            buttons[index].icon,
            has_action ? system_button_event_cb : nullptr,
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
