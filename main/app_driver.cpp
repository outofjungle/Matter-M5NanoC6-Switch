/*
   M5NanoC6 Matter Switch - Driver Implementation

   Hardware:
   - Button: GPIO 9 (active low)
   - WS2812 LED Data: GPIO 20
   - WS2812 LED Power Enable: GPIO 19
*/

#include <stdlib.h>
#include <string.h>

#include <esp_log.h>
#include <esp_matter.h>
#include <driver/gpio.h>
#include <driver/rmt.h>
#include <led_strip.h>
#include <iot_button.h>
#include <button_gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include <app_priv.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";
led_strip_t *s_led_strip = NULL;  // Exported for app_reset LED control
static TimerHandle_t s_identify_timer = NULL;
static bool s_identify_blink_state = false;

app_driver_handle_t app_driver_led_init(void)
{
    // Enable power to WS2812 LED by setting GPIO 19 HIGH
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << M5NANOC6_LED_POWER_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)M5NANOC6_LED_POWER_GPIO, 1);
    ESP_LOGI(TAG, "Enabled WS2812 power on GPIO %d", M5NANOC6_LED_POWER_GPIO);

    // Configure RMT for WS2812
    rmt_config_t rmt_cfg = RMT_DEFAULT_CONFIG_TX((gpio_num_t)M5NANOC6_LED_DATA_GPIO, (rmt_channel_t)M5NANOC6_RMT_CHANNEL);
    rmt_cfg.clk_div = 2;

    esp_err_t err = rmt_config(&rmt_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "RMT config failed: %d", err);
        return NULL;
    }

    err = rmt_driver_install(rmt_cfg.channel, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "RMT driver install failed: %d", err);
        return NULL;
    }

    // Create LED strip using RMT
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(1, (led_strip_dev_t)rmt_cfg.channel);
    s_led_strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!s_led_strip) {
        ESP_LOGE(TAG, "Failed to create WS2812 LED strip");
        return NULL;
    }

    // Set initial LED state (off = dim blue)
    s_led_strip->set_pixel(s_led_strip, 0, 0, 0, 20);  // Dim blue
    s_led_strip->refresh(s_led_strip, 100);

    ESP_LOGI(TAG, "LED driver initialized on GPIO %d", M5NANOC6_LED_DATA_GPIO);
    return (app_driver_handle_t)s_led_strip;
}

app_driver_handle_t app_driver_button_init(void)
{
    // Initialize button using iot_button
    button_config_t btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 0,
        .short_press_time = 0,
        .gpio_button_config = {
            .gpio_num = M5NANOC6_BUTTON_GPIO,
            .active_level = 0,  // Active low
        },
    };

    button_handle_t btn_handle = iot_button_create(&btn_cfg);
    if (!btn_handle) {
        ESP_LOGE(TAG, "Failed to create button device");
        return NULL;
    }

    ESP_LOGI(TAG, "Button initialized on GPIO %d", M5NANOC6_BUTTON_GPIO);
    return (app_driver_handle_t)btn_handle;
}

esp_err_t app_driver_led_set_power(app_driver_handle_t handle, bool power)
{
    led_strip_t *strip = s_led_strip;
    if (handle) {
        strip = (led_strip_t *)handle;
    }
    if (!strip) {
        ESP_LOGE(TAG, "LED strip is NULL");
        return ESP_ERR_INVALID_STATE;
    }

    if (power) {
        // ON state = bright blue
        strip->set_pixel(strip, 0, 0, 0, 128);  // Bright blue (GRB order for WS2812)
    } else {
        // OFF state = dim blue
        strip->set_pixel(strip, 0, 0, 0, 20);   // Dim blue
    }

    strip->refresh(strip, 100);
    ESP_LOGI(TAG, "LED indicator set to %s", power ? "ON (bright blue)" : "OFF (dim blue)");
    return ESP_OK;
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;

    if (cluster_id == OnOff::Id) {
        if (attribute_id == OnOff::Attributes::OnOff::Id) {
            ESP_LOGI(TAG, "OnOff attribute updated: endpoint %d, value %d", endpoint_id, val->val.b);
            err = app_driver_led_set_power(driver_handle, val->val.b);
        }
    }

    return err;
}

// Timer callback for identify blink
static void identify_timer_cb(TimerHandle_t timer)
{
    if (!s_led_strip) {
        return;
    }

    s_identify_blink_state = !s_identify_blink_state;
    if (s_identify_blink_state) {
        // Blink ON - white flash
        s_led_strip->set_pixel(s_led_strip, 0, 128, 128, 128);
    } else {
        // Blink OFF
        s_led_strip->set_pixel(s_led_strip, 0, 0, 0, 0);
    }
    s_led_strip->refresh(s_led_strip, 100);
}

esp_err_t app_driver_led_identify_start(void)
{
    ESP_LOGI(TAG, "Starting identify blink");

    if (!s_identify_timer) {
        s_identify_timer = xTimerCreate("identify", pdMS_TO_TICKS(500), pdTRUE, NULL, identify_timer_cb);
    }

    if (s_identify_timer) {
        xTimerStart(s_identify_timer, 0);
    }

    return ESP_OK;
}

esp_err_t app_driver_led_identify_stop(bool current_power)
{
    ESP_LOGI(TAG, "Stopping identify blink");

    if (s_identify_timer) {
        xTimerStop(s_identify_timer, 0);
    }

    // Restore normal LED state
    return app_driver_led_set_power(NULL, current_power);
}
