/*
   M5NanoC6 Matter Switch - Driver Implementation

   Hardware:
   - Button: GPIO 9 (active low)
   - WS2812 LED Data: GPIO 20
   - WS2812 LED Power Enable: GPIO 19
*/

#include <stdlib.h>
#include <string.h>
#include <atomic>

#include <esp_log.h>
#include <esp_matter.h>
#include <driver/gpio.h>
#include <driver/rmt.h>
#include <led_strip.h>
#include <iot_button.h>
#include <button_gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/timers.h>

#include <app_priv.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";

static led_strip_t *s_led_strip = NULL;
static SemaphoreHandle_t s_led_mutex = NULL;
static TimerHandle_t s_identify_timer = NULL;
static std::atomic<bool> s_identify_blink_state{false};

// Helper macro for LED mutex lock/unlock with timeout
#define LED_MUTEX_TIMEOUT_MS 50
#define LED_LOCK() (s_led_mutex && xSemaphoreTake(s_led_mutex, pdMS_TO_TICKS(LED_MUTEX_TIMEOUT_MS)) == pdTRUE)
#define LED_UNLOCK() do { if (s_led_mutex) xSemaphoreGive(s_led_mutex); } while(0)

// Forward declaration for timer callback
static void identify_timer_cb(TimerHandle_t timer);

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
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed: %d", err);
        return NULL;
    }
    err = gpio_set_level(static_cast<gpio_num_t>(M5NANOC6_LED_POWER_GPIO), 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO set level failed: %d", err);
        return NULL;
    }
    ESP_LOGI(TAG, "Enabled WS2812 power on GPIO %d", M5NANOC6_LED_POWER_GPIO);

    // Configure RMT for WS2812
    rmt_config_t rmt_cfg = RMT_DEFAULT_CONFIG_TX(static_cast<gpio_num_t>(M5NANOC6_LED_DATA_GPIO),
                                                  static_cast<rmt_channel_t>(M5NANOC6_RMT_CHANNEL));
    rmt_cfg.clk_div = 2;

    err = rmt_config(&rmt_cfg);
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
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(1, reinterpret_cast<led_strip_dev_t>(rmt_cfg.channel));
    s_led_strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!s_led_strip) {
        ESP_LOGE(TAG, "Failed to create WS2812 LED strip");
        rmt_driver_uninstall(rmt_cfg.channel);  // Cleanup on failure
        return NULL;
    }

    // Create mutex for thread-safe LED access
    s_led_mutex = xSemaphoreCreateMutex();
    if (!s_led_mutex) {
        ESP_LOGE(TAG, "Failed to create LED mutex");
        s_led_strip->clear(s_led_strip, LED_REFRESH_TIMEOUT_MS);
        free(s_led_strip);
        s_led_strip = NULL;
        rmt_driver_uninstall(static_cast<rmt_channel_t>(M5NANOC6_RMT_CHANNEL));
        return NULL;
    }

    // Set initial LED state (off = dim blue)
    s_led_strip->set_pixel(s_led_strip, 0, LED_COLOR_OFF_G, LED_COLOR_OFF_R, LED_COLOR_OFF_B);
    s_led_strip->refresh(s_led_strip, LED_REFRESH_TIMEOUT_MS);

    // Pre-create identify timer to avoid allocation during operation
    s_identify_timer = xTimerCreate("identify", pdMS_TO_TICKS(LED_IDENTIFY_BLINK_MS), pdTRUE, NULL, identify_timer_cb);
    if (!s_identify_timer) {
        ESP_LOGE(TAG, "Failed to create identify timer");
        vSemaphoreDelete(s_led_mutex);
        s_led_mutex = NULL;
        s_led_strip->clear(s_led_strip, LED_REFRESH_TIMEOUT_MS);
        free(s_led_strip);
        s_led_strip = NULL;
        rmt_driver_uninstall(static_cast<rmt_channel_t>(M5NANOC6_RMT_CHANNEL));
        return NULL;
    }

    ESP_LOGI(TAG, "LED driver initialized on GPIO %d", M5NANOC6_LED_DATA_GPIO);
    return static_cast<app_driver_handle_t>(s_led_strip);
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
    return static_cast<app_driver_handle_t>(btn_handle);
}

esp_err_t app_driver_led_set_power(app_driver_handle_t handle, bool power)
{
    (void)handle;  // Unused - always use global LED strip

    if (!s_led_strip) {
        ESP_LOGE(TAG, "LED strip not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Retry logic to handle concurrent LED operations (e.g., identify blink)
    const int MAX_RETRIES = 3;
    for (int i = 0; i < MAX_RETRIES; i++) {
        if (LED_LOCK()) {
            if (power) {
                // ON state = bright blue
                s_led_strip->set_pixel(s_led_strip, 0, LED_COLOR_ON_G, LED_COLOR_ON_R, LED_COLOR_ON_B);
            } else {
                // OFF state = dim blue
                s_led_strip->set_pixel(s_led_strip, 0, LED_COLOR_OFF_G, LED_COLOR_OFF_R, LED_COLOR_OFF_B);
            }

            s_led_strip->refresh(s_led_strip, LED_REFRESH_TIMEOUT_MS);
            LED_UNLOCK();

            ESP_LOGD(TAG, "LED set to %s", power ? "ON" : "OFF");
            return ESP_OK;
        }
        // Brief delay before retry
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGE(TAG, "LED mutex timeout after %d retries", MAX_RETRIES);
    return ESP_ERR_TIMEOUT;
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, const esp_matter_attr_val_t *val)
{
    if (!val) {
        ESP_LOGE(TAG, "Attribute value is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = ESP_OK;

    if (cluster_id == OnOff::Id) {
        if (attribute_id == OnOff::Attributes::OnOff::Id) {
            ESP_LOGD(TAG, "OnOff: endpoint %d, value %d", endpoint_id, val->val.b);
            err = app_driver_led_set_power(driver_handle, val->val.b);
        }
    }

    return err;
}

// Timer callback for identify blink
static void identify_timer_cb(TimerHandle_t timer)
{
    if (!s_led_strip || !LED_LOCK()) {
        return;
    }

    s_identify_blink_state = !s_identify_blink_state;
    if (s_identify_blink_state) {
        // Blink ON - white flash
        s_led_strip->set_pixel(s_led_strip, 0, LED_COLOR_IDENTIFY_G, LED_COLOR_IDENTIFY_R, LED_COLOR_IDENTIFY_B);
    } else {
        // Blink OFF
        s_led_strip->set_pixel(s_led_strip, 0, 0, 0, 0);
    }
    s_led_strip->refresh(s_led_strip, LED_REFRESH_TIMEOUT_MS);
    LED_UNLOCK();
}

esp_err_t app_driver_led_identify_start(void)
{
    ESP_LOGI(TAG, "Starting identify blink");

    if (!s_identify_timer) {
        ESP_LOGW(TAG, "Identify timer not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (xTimerStart(s_identify_timer, pdMS_TO_TICKS(100)) != pdPASS) {
        ESP_LOGE(TAG, "Failed to start identify timer");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t app_driver_led_identify_stop(bool current_power)
{
    ESP_LOGI(TAG, "Stopping identify blink");

    if (s_identify_timer) {
        // Use timeout to ensure command is processed
        xTimerStop(s_identify_timer, pdMS_TO_TICKS(100));
    }

    // Restore normal LED state
    return app_driver_led_set_power(NULL, current_power);
}

led_strip_t *app_driver_get_led_strip(void)
{
    return s_led_strip;
}

bool app_driver_led_lock(void)
{
    return LED_LOCK();
}

void app_driver_led_unlock(void)
{
    LED_UNLOCK();
}
