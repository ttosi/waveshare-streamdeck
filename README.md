# Waveshare Streamdeck

PlatformIO project for the Waveshare ESP32-S3-Touch-LCD-4.3 version A. The app uses LVGL tileview pages and sends USB HID keyboard events.

## Layout

- `src/main.cpp`: hardware, USB, display, touch, and LVGL startup.
- `src/app` / `include/app`: top-level UI and page registry.
- `src/pages` / `include/pages`: one module per screen page.
- `src/ui` / `include/ui`: reusable LVGL widgets and page styling.
- `src/services` / `include/services`: HID text output and screen power/touch wake logic.
- `src/platform` / `include/platform`: LVGL display/touch port.
- `data/icons`: LVGL binary icon files for the device filesystem.
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

Place LVGL `.bin` files in `data/icons`. Use `RGB565A8` for icons with transparency. The recommended size is `96 x 96 px`, with the visible artwork centered around `80 x 80 px`.

For pages with many icons, declare a local `IconGridItem icons[]` in that page module. `IconGridConfig` controls columns, cell size, and gaps; up to 15 icon buttons fits naturally as a 5x3 or 3x5 grid.

Keep all 15 CSV rows to preserve the 5x3 layout. For an empty, non-clickable slot, leave both `icon` and `action` empty:

```csv
page,position,icon,action
inkscape,0,icons/inkscape/rectangle.bin,key:r
inkscape,1,,
inkscape,2,icons/inkscape/ellipse.bin,key:e
```

Use empty fields rather than the literal text `null`.

Convert a directory of PNG icons to LVGL v8 `RGB565A8` binary files with:

```bash
python3 tools/convert_lvgl_icons.py assests/icons/blender data/icons/blender
```

The converter requires ImageMagick's `convert` command.

Upload files from `data` to LittleFS separately from the firmware:

```bash
~/.platformio/penv/bin/pio run -t uploadfs
```

## Button Actions

Button actions are pipe-separated steps:

```text
combo:KEY_LEFT_SHIFT+a|type:cube|enter
```

Supported steps:

- `type:text`: type text exactly, including characters such as `$` and `#`.
- `key:KEY_KP_1`: press and release one named key.
- `press:KEY_KP_1`: alias for `key:`.
- `combo:KEY_LEFT_SHIFT+a`: press keys together, then release all.
- `delay:150`: wait milliseconds before the next step.
- `secret:LOGIN_PASSWORD`: type the password stored in ignored `include/config/secrets.h`.
- `enter`: alias for `key:KEY_RETURN`.

Common key names include `KEY_RETURN`, `KEY_TAB`, `KEY_ESC`, `KEY_SPACE`, `KEY_LEFT_SHIFT`, `KEY_LEFT_CTRL`, `KEY_LEFT_ALT`, `KEY_F1` through `KEY_F24`, and `KEY_KP_0` through `KEY_KP_9`.

Example lock-screen login action:

```text
key:KEY_SPACE|delay:500|key:KEY_DELETE|secret:LOGIN_PASSWORD|enter
```

## Build

```bash
~/.platformio/penv/bin/pio run
```

## Upload

```bash
~/.platformio/penv/bin/pio run -t upload
```

The upload config uses native USB with a 1200-baud touch reset and waits for `/dev/ttyACM0` to return.
