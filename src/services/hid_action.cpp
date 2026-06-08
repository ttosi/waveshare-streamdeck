#include "services/hid_action.h"

#include <Arduino.h>
#include <ctype.h>

#include "services/hid_text.h"

namespace {

constexpr uint32_t KEY_HOLD_MS = 30;
constexpr uint32_t STEP_DELAY_MS = 50;
constexpr uint32_t MAX_ACTION_DELAY_MS = 10000;

struct NamedKey {
    const char *name;
    uint8_t code;
};

const NamedKey NAMED_KEYS[] = {
    {"KEY_LEFT_CTRL", KEY_LEFT_CTRL},
    {"KEY_LEFT_SHIFT", KEY_LEFT_SHIFT},
    {"KEY_LEFT_ALT", KEY_LEFT_ALT},
    {"KEY_LEFT_GUI", KEY_LEFT_GUI},
    {"KEY_RIGHT_CTRL", KEY_RIGHT_CTRL},
    {"KEY_RIGHT_SHIFT", KEY_RIGHT_SHIFT},
    {"KEY_RIGHT_ALT", KEY_RIGHT_ALT},
    {"KEY_RIGHT_GUI", KEY_RIGHT_GUI},
    {"KEY_UP_ARROW", KEY_UP_ARROW},
    {"KEY_DOWN_ARROW", KEY_DOWN_ARROW},
    {"KEY_LEFT_ARROW", KEY_LEFT_ARROW},
    {"KEY_RIGHT_ARROW", KEY_RIGHT_ARROW},
    {"KEY_MENU", KEY_MENU},
    {"KEY_SPACE", KEY_SPACE},
    {"KEY_BACKSPACE", KEY_BACKSPACE},
    {"KEY_TAB", KEY_TAB},
    {"KEY_RETURN", KEY_RETURN},
    {"KEY_ENTER", KEY_RETURN},
    {"KEY_ESC", KEY_ESC},
    {"KEY_ESCAPE", KEY_ESC},
    {"KEY_INSERT", KEY_INSERT},
    {"KEY_DELETE", KEY_DELETE},
    {"KEY_PAGE_UP", KEY_PAGE_UP},
    {"KEY_PAGE_DOWN", KEY_PAGE_DOWN},
    {"KEY_HOME", KEY_HOME},
    {"KEY_END", KEY_END},
    {"KEY_NUM_LOCK", KEY_NUM_LOCK},
    {"KEY_CAPS_LOCK", KEY_CAPS_LOCK},
    {"KEY_PRINT_SCREEN", KEY_PRINT_SCREEN},
    {"KEY_SCROLL_LOCK", KEY_SCROLL_LOCK},
    {"KEY_PAUSE", KEY_PAUSE},
    {"KEY_KP_SLASH", KEY_KP_SLASH},
    {"KEY_KP_ASTERISK", KEY_KP_ASTERISK},
    {"KEY_KP_MINUS", KEY_KP_MINUS},
    {"KEY_KP_PLUS", KEY_KP_PLUS},
    {"KEY_KP_ENTER", KEY_KP_ENTER},
    {"KEY_KP_0", KEY_KP_0},
    {"KEY_KP_1", KEY_KP_1},
    {"KEY_KP_2", KEY_KP_2},
    {"KEY_KP_3", KEY_KP_3},
    {"KEY_KP_4", KEY_KP_4},
    {"KEY_KP_5", KEY_KP_5},
    {"KEY_KP_6", KEY_KP_6},
    {"KEY_KP_7", KEY_KP_7},
    {"KEY_KP_8", KEY_KP_8},
    {"KEY_KP_9", KEY_KP_9},
    {"KEY_KP_DOT", KEY_KP_DOT},
};

bool parse_function_key(const String &name, uint8_t *keycode)
{
    if (!name.startsWith("KEY_F")) {
        return false;
    }

    const int number = name.substring(5).toInt();
    if (number >= 1 && number <= 12) {
        *keycode = KEY_F1 + number - 1;
        return true;
    }
    if (number >= 13 && number <= 24) {
        *keycode = KEY_F13 + number - 13;
        return true;
    }
    return false;
}

bool parse_key(String name, uint8_t *keycode)
{
    name.trim();
    if (name.length() == 1) {
        *keycode = static_cast<uint8_t>(name[0]);
        return true;
    }

    name.toUpperCase();
    for (const NamedKey &key : NAMED_KEYS) {
        if (name == key.name) {
            *keycode = key.code;
            return true;
        }
    }

    if (name.length() == 5 && name.startsWith("KEY_")) {
        *keycode = static_cast<uint8_t>(tolower(name[4]));
        return true;
    }

    return parse_function_key(name, keycode);
}

bool execute_key(USBHIDKeyboard *keyboard, const String &key_name)
{
    uint8_t keycode;
    if (!parse_key(key_name, &keycode)) {
        return false;
    }

    keyboard->write(keycode);
    return true;
}

bool execute_combo(USBHIDKeyboard *keyboard, String combo)
{
    keyboard->releaseAll();

    int start = 0;
    bool pressed_key = false;
    while (start <= combo.length()) {
        const int separator = combo.indexOf('+', start);
        const int end = separator < 0 ? combo.length() : separator;
        uint8_t keycode;
        if (!parse_key(combo.substring(start, end), &keycode)) {
            keyboard->releaseAll();
            return false;
        }

        keyboard->press(keycode);
        pressed_key = true;
        if (separator < 0) {
            break;
        }
        start = separator + 1;
    }

    if (pressed_key) {
        delay(KEY_HOLD_MS);
    }
    keyboard->releaseAll();
    return pressed_key;
}

bool parse_delay_ms(String value, uint32_t *delay_ms)
{
    value.trim();
    if (value.length() == 0) {
        return false;
    }

    for (size_t index = 0; index < value.length(); index++) {
        if (!isdigit(static_cast<unsigned char>(value[index]))) {
            return false;
        }
    }

    const unsigned long parsed_delay = strtoul(value.c_str(), nullptr, 10);
    if (parsed_delay > MAX_ACTION_DELAY_MS) {
        return false;
    }

    *delay_ms = static_cast<uint32_t>(parsed_delay);
    return true;
}

bool execute_step(USBHIDKeyboard *keyboard, String step)
{
    step.trim();
    if (step.length() == 0) {
        return true;
    }

    if (step.equalsIgnoreCase("enter")) {
        return execute_key(keyboard, "KEY_RETURN");
    }

    const int separator = step.indexOf(':');
    if (separator < 0) {
        return false;
    }

    String command = step.substring(0, separator);
    String value = step.substring(separator + 1);
    command.trim();

    if (command.equalsIgnoreCase("type") || command.equalsIgnoreCase("text")) {
        send_hid_text(keyboard, value.c_str());
        return true;
    }
    if (command.equalsIgnoreCase("key") || command.equalsIgnoreCase("press")) {
        return execute_key(keyboard, value);
    }
    if (command.equalsIgnoreCase("combo")) {
        return execute_combo(keyboard, value);
    }
    if (command.equalsIgnoreCase("delay")) {
        uint32_t delay_ms;
        if (!parse_delay_ms(value, &delay_ms)) {
            return false;
        }
        delay(delay_ms);
        return true;
    }

    return false;
}

} // namespace

bool execute_hid_action(USBHIDKeyboard *keyboard, const char *action)
{
    if (keyboard == nullptr || action == nullptr) {
        return false;
    }

    const String sequence(action);
    int start = 0;
    while (start <= sequence.length()) {
        const int separator = sequence.indexOf('|', start);
        const int end = separator < 0 ? sequence.length() : separator;
        if (!execute_step(keyboard, sequence.substring(start, end))) {
            keyboard->releaseAll();
            return false;
        }

        if (separator < 0) {
            break;
        }
        delay(STEP_DELAY_MS);
        start = separator + 1;
    }

    keyboard->releaseAll();
    return true;
}
