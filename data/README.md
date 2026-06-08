Place LVGL binary icon files here.

Recommended format for streamdeck buttons:

- `RGB565A8`
- `96 x 96 px`
- Transparent canvas with visible artwork centered around `80 x 80 px`

Example path used by LVGL after mounting the filesystem:

```cpp
lv_img_set_src(icon, "A:/icons/axis_side.bin");
```
