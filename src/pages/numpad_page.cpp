#include "pages/numpad_page.h"

#include "config/app_config.h"
#include "services/screen_power.h"

struct NumpadKey {
    const char *label;
    uint8_t keycode;
    int16_t column;
    int16_t row;
    int16_t column_span;
    int16_t row_span;
    bool accent;
};

static USBHIDKeyboard *numpad_keyboard;

static const NumpadKey numpad_keys[] = {
    {"NUM", KEY_NUM_LOCK,    0, 0, 1, 1, true},
    {"/",   KEY_KP_SLASH,    1, 0, 1, 1, false},
    {"*",   KEY_KP_ASTERISK, 2, 0, 1, 1, false},
    {"-",   KEY_KP_MINUS,    3, 0, 1, 1, false},
    {"7",   KEY_KP_7,        0, 1, 1, 1, false},
    {"8",   KEY_KP_8,        1, 1, 1, 1, false},
    {"9",   KEY_KP_9,        2, 1, 1, 1, false},
    {"+",   KEY_KP_PLUS,     3, 1, 1, 2, true},
    {"4",   KEY_KP_4,        0, 2, 1, 1, false},
    {"5",   KEY_KP_5,        1, 2, 1, 1, false},
    {"6",   KEY_KP_6,        2, 2, 1, 1, false},
    {"1",   KEY_KP_1,        0, 3, 1, 1, false},
    {"2",   KEY_KP_2,        1, 3, 1, 1, false},
    {"3",   KEY_KP_3,        2, 3, 1, 1, false},
    {"ENT", KEY_KP_ENTER,    3, 3, 1, 2, true},
    {"0",   KEY_KP_0,        0, 4, 2, 1, false},
    {".",   KEY_KP_DOT,      2, 4, 1, 1, false},
};

static void numpad_key_event_cb(lv_event_t *event)
{
    const lv_event_code_t event_code = lv_event_get_code(event);
    if (event_code == LV_EVENT_PRESSED) {
        screen_power_note_touch();
        return;
    }
    if (event_code != LV_EVENT_CLICKED) {
        return;
    }

    screen_power_note_touch();
    if (screen_power_should_consume_click()) {
        return;
    }

    const NumpadKey *key = static_cast<const NumpadKey *>(lv_event_get_user_data(event));
    numpad_keyboard->write(key->keycode);
}

static void create_numpad_key(lv_obj_t *page, const NumpadKey *key)
{
    constexpr int16_t grid_x = 92;
    constexpr int16_t grid_y = 22;
    constexpr int16_t key_width = 145;
    constexpr int16_t key_height = 79;
    constexpr int16_t gap = 12;

    const int16_t x = grid_x + key->column * (key_width + gap);
    const int16_t y = grid_y + key->row * (key_height + gap);
    const int16_t width = key->column_span * key_width + (key->column_span - 1) * gap;
    const int16_t height = key->row_span * key_height + (key->row_span - 1) * gap;

    lv_obj_t *button = lv_btn_create(page);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_size(button, width, height);
    lv_obj_set_style_radius(button, 12, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(key->accent ? app_config::COLOR_ACCENT : app_config::COLOR_KEY), 0);
    lv_obj_set_style_bg_color(
        button, lv_color_hex(key->accent ? app_config::COLOR_ACCENT_PRESSED : app_config::COLOR_KEY_PRESSED),
        LV_STATE_PRESSED
    );
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_border_color(
        button, lv_color_hex(key->accent ? app_config::COLOR_ACCENT_BORDER : app_config::COLOR_KEY_BORDER), 0
    );
    lv_obj_add_event_cb(button, numpad_key_event_cb, LV_EVENT_ALL, const_cast<NumpadKey *>(key));

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, key->label);
    lv_obj_set_style_text_color(label, lv_color_hex(app_config::COLOR_TEXT), 0);
    lv_obj_set_style_text_font(label, key->label[1] == '\0' ? &lv_font_montserrat_30 : &lv_font_montserrat_16, 0);
    lv_obj_center(label);
}

void create_numpad_page(lv_obj_t *page, USBHIDKeyboard *keyboard)
{
    numpad_keyboard = keyboard;
    for (const NumpadKey &key : numpad_keys) {
        create_numpad_key(page, &key);
    }
}
