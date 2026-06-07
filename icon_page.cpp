#include "icon_page.h"

#include "assets.h"
#include "hid_text.h"
#include "icon_grid.h"
#include "screen_power.h"

static void send_test_event_cb(lv_event_t *event)
{
    screen_power_note_touch();
    if (screen_power_should_consume_click()) {
        return;
    }

    USBHIDKeyboard *keyboard = static_cast<USBHIDKeyboard *>(lv_event_get_user_data(event));
    send_hid_text_and_enter(keyboard, "");
}

void create_icon_page(lv_obj_t *page, USBHIDKeyboard *keyboard)
{
    const IconGridItem icons[] = {
        {&icon_axis_side, send_test_event_cb, keyboard},
    };
    static const IconGridConfig grid = {1, 96, 96, 0, 0};

    create_icon_grid(page, icons, sizeof(icons) / sizeof(icons[0]), grid);
}
