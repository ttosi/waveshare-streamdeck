# Waveshare Streamdeck

PlatformIO project for the Waveshare ESP32-S3-Touch-LCD-4.3 version A. The app uses LVGL tileview pages and sends USB HID keyboard events.

## Layout

- `src/main.cpp`: hardware, USB, display, touch, and LVGL startup.
- `src/app` / `include/app`: top-level UI and page registry.
- `src/pages` / `include/pages`: one module per screen page.
- `src/ui` / `include/ui`: reusable LVGL widgets and page styling.
- `src/services` / `include/services`: HID text output and screen power/touch wake logic.
- `src/platform` / `include/platform`: LVGL display/touch port.
- `src/assets` / `include/assets`: LVGL image arrays and declarations.
- `include/config`: app-level configuration and local secrets.
- `include/lv_conf.h`: LVGL configuration.
- `include/esp_panel_board_custom_conf.h`: Waveshare panel/touch/backlight/USB mux configuration.
- `hardware`: enclosure/reference files.

## Adding A Page

1. Add `include/pages/my_page.h` and `src/pages/my_page.cpp`.
2. Expose a builder such as:

   ```cpp
   void create_my_page(lv_obj_t *page, USBHIDKeyboard *keyboard);
   ```

3. Include the page header in `src/app/app_ui.cpp`.
4. Add a small builder adapter and register it in the `pages` array.

Tile positions and left/right navigation directions are calculated automatically.

## Adding Icons

Place generated LVGL image `.c` files in `src/assets` and add matching `LV_IMG_DECLARE(...)` declarations to `include/assets/assets.h`.

For pages with many icons, declare a local `IconGridItem icons[]` in that page module. `IconGridConfig` controls columns, cell size, and gaps; up to 15 icon buttons fits naturally as a 5x3 or 3x5 grid.

## Build

```bash
~/.platformio/penv/bin/pio run
```

## Upload

```bash
~/.platformio/penv/bin/pio run -t upload
```

The upload config uses native USB with a 1200-baud touch reset and waits for `/dev/ttyACM0` to return.
