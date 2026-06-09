#include "services/image_assets.h"

#include <Arduino.h>
#include <LittleFS.h>

namespace {

constexpr size_t MAX_IMAGE_ASSETS = 64;

struct ImageAsset {
    const char *path;
    lv_img_dsc_t descriptor;
};

ImageAsset assets[MAX_IMAGE_ASSETS];
size_t asset_count;

const char *littlefs_path(const char *lvgl_path)
{
    return lvgl_path[0] != '\0' && lvgl_path[1] == ':' ? lvgl_path + 2 : lvgl_path;
}

const lv_img_dsc_t *find_loaded_asset(const char *path)
{
    for (size_t index = 0; index < asset_count; index++) {
        if (strcmp(assets[index].path, path) == 0) {
            return &assets[index].descriptor;
        }
    }
    return nullptr;
}

} // namespace

const lv_img_dsc_t *load_image_asset(const char *lvgl_path)
{
    if (lvgl_path == nullptr) {
        return nullptr;
    }

    const lv_img_dsc_t *loaded = find_loaded_asset(lvgl_path);
    if (loaded != nullptr) {
        return loaded;
    }
    if (asset_count >= MAX_IMAGE_ASSETS) {
        return nullptr;
    }

    File file = LittleFS.open(littlefs_path(lvgl_path), FILE_READ);
    if (!file || file.size() <= sizeof(lv_img_header_t)) {
        return nullptr;
    }

    ImageAsset &asset = assets[asset_count];
    if (file.read(reinterpret_cast<uint8_t *>(&asset.descriptor.header), sizeof(lv_img_header_t))
            != sizeof(lv_img_header_t)) {
        return nullptr;
    }

    asset.descriptor.data_size = file.size() - sizeof(lv_img_header_t);
    uint8_t *data = static_cast<uint8_t *>(ps_malloc(asset.descriptor.data_size));
    if (data == nullptr || file.read(data, asset.descriptor.data_size) != asset.descriptor.data_size) {
        free(data);
        return nullptr;
    }

    asset.path = lvgl_path;
    asset.descriptor.data = data;
    asset_count++;
    return &asset.descriptor;
}
