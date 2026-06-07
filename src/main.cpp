#include <Arduino.h>

#ifndef ARDUINO_USB_MODE
#error "This sketch requires an ESP32-S3 with native USB support."
#elif ARDUINO_USB_MODE == 1
#error "Set ARDUINO_USB_MODE=0 for USB-OTG (TinyUSB)."
#endif

#if ARDUINO_USB_CDC_ON_BOOT
#error "Set ARDUINO_USB_CDC_ON_BOOT=0. This sketch adds CDC manually."
#endif

#ifndef BOARD_HAS_PSRAM
#error "Enable OPI PSRAM for triple-buffered animation."
#endif

#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "USB.h"
#include "USBHIDKeyboard.h"

#include "app_ui.h"
#include "lvgl_v8_port.h"
#include "screen_power.h"

USBCDC USBSerial;
USBHIDKeyboard Keyboard;

void setup()
{
    // Panel initialization configures the board's physical USB/CAN switch.
    ESP_Panel *panel = new ESP_Panel();
    panel->init();
    static_cast<esp_panel::drivers::BusRGB *>(panel->getLcd()->getBus())
        ->configRGB_FrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
    panel->begin();
    screen_power_init(panel->getBacklight());

    // Start TinyUSB only after GPIO19/GPIO20 are routed to the USB connector.
    USBSerial.begin();
    Keyboard.setShiftKeyReports(true);
    Keyboard.begin();
    USB.begin();

    USBSerial.println("Starting display...");

    lvgl_port_init(panel->getLcd(), panel->getTouch());

    if (lvgl_port_lock(-1)) {
        create_app_ui(&Keyboard);
        lvgl_port_unlock();
    }

    USBSerial.println("Stream deck ready.");
}

void loop()
{
    delay(1000);
}
