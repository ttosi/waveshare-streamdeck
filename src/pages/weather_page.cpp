#include "pages/weather_page.h"

#include <math.h>
#include <time.h>

#include "config/app_config.h"
#include "config/weather_config.h"
#include "services/image_assets.h"
#include "services/weather_service.h"

namespace {

struct ForecastWidgets {
    lv_obj_t *day;
    lv_obj_t *icon;
    lv_obj_t *precipitation;
    lv_obj_t *temperatures;
};

lv_obj_t *clock_label;
lv_obj_t *date_label;
lv_obj_t *date_bold_label;
lv_obj_t *status_label;
lv_obj_t *current_icon;
lv_obj_t *temperature_label;
lv_obj_t *condition_label;
lv_obj_t *details_label;
lv_obj_t *temperature_chart;
lv_obj_t *chart_range_label;
lv_chart_series_t *temperature_series;
ForecastWidgets forecast[weather_config::FORECAST_DAY_COUNT];

void set_text_color(lv_obj_t *object, uint32_t color)
{
    lv_obj_set_style_text_color(object, lv_color_hex(color), 0);
}

lv_obj_t *create_label(lv_obj_t *parent, const lv_font_t *font, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, font, 0);
    set_text_color(label, color);
    return label;
}

lv_obj_t *create_card(lv_obj_t *parent)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_style_bg_color(card, lv_color_hex(app_config::COLOR_KEY), 0);
    lv_obj_set_style_border_color(card, lv_color_hex(app_config::COLOR_KEY_BORDER), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_pad_all(card, 8, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

void update_icon(lv_obj_t *image, const char *path)
{
    const lv_img_dsc_t *asset = load_image_asset(path);
    if (asset != nullptr) {
        lv_img_set_src(image, asset);
    }
}

void chart_draw_event_cb(lv_event_t *event)
{
    lv_obj_draw_part_dsc_t *draw = lv_event_get_draw_part_dsc(event);
    if (!lv_obj_draw_part_check_type(draw, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL)) {
        return;
    }
    if (draw->id == LV_CHART_AXIS_PRIMARY_Y && draw->text != nullptr) {
        lv_snprintf(draw->text, draw->text_length, "%d°", static_cast<int>(draw->value));
    }
}

void format_forecast_day(const char *date, size_t index, char *output, size_t output_size)
{
    if (index == 0) {
        strlcpy(output, "TODAY", output_size);
        return;
    }

    struct tm value = {};
    if (sscanf(date, "%d-%d-%d", &value.tm_year, &value.tm_mon, &value.tm_mday) != 3) {
        strlcpy(output, "---", output_size);
        return;
    }
    value.tm_year -= 1900;
    value.tm_mon -= 1;
    mktime(&value);
    strftime(output, output_size, "%a", &value);
    for (size_t character = 0; output[character] != '\0'; character++) {
        output[character] = toupper(static_cast<unsigned char>(output[character]));
    }
}

void refresh_page(lv_timer_t *)
{
    const time_t now = time(nullptr);
    struct tm local = {};
    if (now > 100000 && localtime_r(&now, &local) != nullptr) {
        char clock[16];
        char date[32];
        int hour = local.tm_hour % 12;
        if (hour == 0) {
            hour = 12;
        }
        lv_snprintf(clock, sizeof(clock), "%d:%02d %s", hour, local.tm_min, local.tm_hour < 12 ? "AM" : "PM");
        char weekday[12];
        char month[12];
        strftime(weekday, sizeof(weekday), "%A", &local);
        strftime(month, sizeof(month), "%B", &local);
        lv_snprintf(date, sizeof(date), "%s, %s %d", weekday, month, local.tm_mday);
        lv_label_set_text(clock_label, clock);
        lv_label_set_text(date_label, date);
        lv_label_set_text(date_bold_label, date);
    }

    WeatherSnapshot snapshot;
    if (!weather_service_get_snapshot(&snapshot)) {
        return;
    }
    if (!snapshot.valid) {
        lv_label_set_text(
            status_label,
            snapshot.status[0] != '\0'
                ? snapshot.status
                : (snapshot.connected ? "Updating weather..." : "Connecting to Wi-Fi...")
        );
        return;
    }

    char buffer[64];
    const int temperature = lroundf(snapshot.temperature);
    const int high = lroundf(snapshot.days[0].high);
    const int low = lroundf(snapshot.days[0].low);
    const int humidity = lroundf(snapshot.humidity);
    const int dew_point = lroundf(snapshot.dew_point);
    lv_label_set_text(
        status_label,
        snapshot.connected
            ? snapshot.status
            : "Offline - showing last update"
    );
    update_icon(current_icon, weather_icon_path(snapshot.weather_code, snapshot.is_day));
    lv_snprintf(buffer, sizeof(buffer), "%d°", temperature);
    lv_label_set_text(temperature_label, buffer);
    lv_label_set_text(condition_label, weather_condition_name(snapshot.weather_code));
    lv_snprintf(
        buffer, sizeof(buffer), "H %d°  L %d°     Humidity %d%%  Dew %d°",
        high, low, humidity, dew_point
    );
    lv_label_set_text(details_label, buffer);

    int chart_min = 200;
    int chart_max = -200;
    for (size_t index = 0; index < weather_config::HOURLY_POINT_COUNT; index++) {
        const int value = lroundf(snapshot.hourly_temperatures[index]);
        chart_min = min(chart_min, value);
        chart_max = max(chart_max, value);
        lv_chart_set_next_value(temperature_chart, temperature_series, value);
    }
    const int chart_padding = max(3, (chart_max - chart_min) / 4);
    lv_chart_set_range(temperature_chart, LV_CHART_AXIS_PRIMARY_Y, chart_min - chart_padding, chart_max + chart_padding);
    lv_chart_set_axis_tick(
        temperature_chart, LV_CHART_AXIS_PRIMARY_Y,
        0, 0, 3, 1, true, 34
    );
    lv_snprintf(buffer, sizeof(buffer), "Next 12h   %d° - %d°", chart_min, chart_max);
    lv_label_set_text(chart_range_label, buffer);
    lv_chart_refresh(temperature_chart);

    for (size_t index = 0; index < weather_config::FORECAST_DAY_COUNT; index++) {
        char day[8];
        format_forecast_day(snapshot.days[index].date, index, day, sizeof(day));
        lv_label_set_text(forecast[index].day, day);
        update_icon(forecast[index].icon, weather_icon_path(snapshot.days[index].weather_code, true));
        lv_snprintf(buffer, sizeof(buffer), "PoP %d%%", snapshot.days[index].precipitation_probability);
        lv_label_set_text(forecast[index].precipitation, buffer);
        lv_snprintf(
            buffer, sizeof(buffer), "%d° / %d°",
            lroundf(snapshot.days[index].high), lroundf(snapshot.days[index].low)
        );
        lv_label_set_text(forecast[index].temperatures, buffer);
    }
}

} // namespace

void create_weather_page(lv_obj_t *page)
{
    clock_label = create_label(page, &lv_font_montserrat_48, app_config::COLOR_TEXT);
    lv_obj_align(clock_label, LV_ALIGN_TOP_LEFT, 33, 18);
    lv_label_set_text(clock_label, "--:-- --");

    date_label = create_label(page, &lv_font_montserrat_16, 0xAEB6C4);
    lv_obj_align(date_label, LV_ALIGN_TOP_RIGHT, -28, 32);
    lv_label_set_text(date_label, "Waiting for time...");

    date_bold_label = create_label(page, &lv_font_montserrat_16, 0xAEB6C4);
    lv_obj_align(date_bold_label, LV_ALIGN_TOP_RIGHT, -27, 32);
    lv_label_set_text(date_bold_label, "Waiting for time...");

    status_label = create_label(page, &lv_font_montserrat_12, 0x8C96A8);
    lv_obj_align(status_label, LV_ALIGN_TOP_RIGHT, -28, 58);
    lv_label_set_text(status_label, "Connecting to Wi-Fi...");

    lv_obj_t *current_card = create_card(page);
    lv_obj_set_size(current_card, 364, 172);
    lv_obj_align(current_card, LV_ALIGN_TOP_LEFT, 28, 92);

    current_icon = lv_img_create(current_card);
    lv_obj_align(current_icon, LV_ALIGN_LEFT_MID, 4, -18);

    temperature_label = create_label(current_card, &lv_font_montserrat_48, app_config::COLOR_TEXT);
    lv_obj_align(temperature_label, LV_ALIGN_LEFT_MID, 92, -31);
    lv_label_set_text(temperature_label, "--°");

    condition_label = create_label(current_card, &lv_font_montserrat_16, app_config::COLOR_TEXT);
    lv_obj_align(condition_label, LV_ALIGN_LEFT_MID, 96, 24);
    lv_label_set_text(condition_label, "Waiting for weather...");

    details_label = create_label(current_card, &lv_font_montserrat_16, 0xAEB6C4);
    lv_obj_align(details_label, LV_ALIGN_BOTTOM_LEFT, 8, -7);
    lv_label_set_text(details_label, "H --°  L --°   Hum --%  Dew --°");

    lv_obj_t *chart_card = create_card(page);
    lv_obj_set_size(chart_card, 364, 172);
    lv_obj_align(chart_card, LV_ALIGN_TOP_RIGHT, -28, 92);

    chart_range_label = create_label(chart_card, &lv_font_montserrat_14, 0xAEB6C4);
    lv_obj_align(chart_range_label, LV_ALIGN_TOP_LEFT, 5, 2);
    lv_label_set_text(chart_range_label, "Next 12h");

    temperature_chart = lv_chart_create(chart_card);
    lv_obj_set_size(temperature_chart, 332, 116);
    lv_obj_align(temperature_chart, LV_ALIGN_BOTTOM_MID, 13, -2);
    lv_obj_set_style_bg_opa(temperature_chart, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(temperature_chart, 0, 0);
    lv_obj_set_style_line_color(temperature_chart, lv_color_hex(app_config::COLOR_KEY_BORDER), LV_PART_MAIN);
    lv_obj_set_style_line_opa(temperature_chart, LV_OPA_40, LV_PART_MAIN);
    lv_chart_set_type(temperature_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(temperature_chart, weather_config::HOURLY_POINT_COUNT);
    lv_chart_set_div_line_count(temperature_chart, 3, 3);
    lv_obj_add_event_cb(temperature_chart, chart_draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, nullptr);
    temperature_series = lv_chart_add_series(
        temperature_chart, lv_color_hex(app_config::COLOR_ACCENT), LV_CHART_AXIS_PRIMARY_Y
    );

    constexpr int card_width = 174;
    constexpr int card_gap = 16;
    constexpr int row_width = card_width * weather_config::FORECAST_DAY_COUNT
        + card_gap * (weather_config::FORECAST_DAY_COUNT - 1);
    for (size_t index = 0; index < weather_config::FORECAST_DAY_COUNT; index++) {
        lv_obj_t *card = create_card(page);
        lv_obj_set_size(card, card_width, 174);
        lv_obj_align(card, LV_ALIGN_BOTTOM_LEFT, (800 - row_width) / 2 + index * (card_width + card_gap), -24);

        forecast[index].day = create_label(card, &lv_font_montserrat_14, 0xAEB6C4);
        lv_obj_align(forecast[index].day, LV_ALIGN_TOP_MID, 0, 2);
        lv_label_set_text(forecast[index].day, index == 0 ? "TODAY" : "---");

        forecast[index].icon = lv_img_create(card);
        lv_obj_align(forecast[index].icon, LV_ALIGN_CENTER, 0, -4);

        forecast[index].precipitation = create_label(card, &lv_font_montserrat_12, 0x8AB4F8);
        lv_obj_align(forecast[index].precipitation, LV_ALIGN_BOTTOM_MID, 0, -27);
        lv_label_set_text(forecast[index].precipitation, "PoP --%");

        forecast[index].temperatures = create_label(card, &lv_font_montserrat_16, app_config::COLOR_TEXT);
        lv_obj_align(forecast[index].temperatures, LV_ALIGN_BOTTOM_MID, 0, -3);
        lv_label_set_text(forecast[index].temperatures, "--° / --°");
    }

    lv_timer_t *timer = lv_timer_create(refresh_page, 1000, nullptr);
    lv_timer_ready(timer);
}
