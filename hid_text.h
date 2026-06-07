#pragma once

#include <USBHIDKeyboard.h>

void send_hid_text(USBHIDKeyboard *keyboard, const char *text);
void send_hid_text_and_enter(USBHIDKeyboard *keyboard, const char *text);
