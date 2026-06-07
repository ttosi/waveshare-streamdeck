# Stream Deck Sketch Structure

- `src/main.cpp`: board, USB, and LVGL startup only.
- `app_ui.cpp`: tileview page registry and navigation.
- `app_config.h`: shared colors and timeout settings.
- `screen_power.cpp`: inactivity timeout and touch-to-wake behavior.
- `ui_common.cpp`: shared page styling.
- `icon_grid.cpp`: reusable data-driven icon grid with optional events.
- `icon_page.cpp`: example icon-grid page.
- `numpad_page.cpp`: numpad layout and HID actions.
- `assets.h`: declarations for converted LVGL image assets.
- `icon_*.c`: generated LVGL image data.

## Adding A Page

1. Create `my_page.h` and `my_page.cpp` with a function such as:

   ```cpp
   void create_my_page(lv_obj_t *page);
   ```

2. Include the header in `app_ui.cpp`.
3. Add a small builder adapter and register it in the `pages` array.

Tile positions and left/right navigation directions are calculated automatically.

## Adding Image Assets

Place generated LVGL image `.c` files in this sketch directory and add their
`LV_IMG_DECLARE(...)` declarations to `assets.h`.

Declare each page's icons in a local `IconGridItem` array. Adjust its
`IconGridConfig` to control columns, cell sizes, and gaps.
