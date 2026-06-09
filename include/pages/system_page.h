#pragma once

#include <lvgl.h>
#include <USBHIDKeyboard.h>

void create_system_page(lv_obj_t *page, USBHIDKeyboard *keyboard);
