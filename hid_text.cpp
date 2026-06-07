#include "hid_text.h"

#include <Arduino.h>

static constexpr uint32_t KEY_DELAY_MS = 8;

void send_hid_text(USBHIDKeyboard *keyboard, const char *text)
{
    keyboard->releaseAll();
    while (*text != '\0') {
        keyboard->write(static_cast<uint8_t>(*text));
        delay(KEY_DELAY_MS);
        text++;
    }
    keyboard->releaseAll();
}

void send_hid_text_and_enter(USBHIDKeyboard *keyboard, const char *text)
{
    send_hid_text(keyboard, text);
    keyboard->write(KEY_RETURN);
}
