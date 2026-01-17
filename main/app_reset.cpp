/*
   M5NanoC6 Matter Switch - Factory Reset Handler

   Implements 20 second button hold for factory reset with LED countdown.
   Runtime only - hold button for 20 seconds while device is running.
*/

#include <atomic>
#include <esp_log.h>
#include <esp_matter.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <iot_button.h>
#include <led_strip.h>

#include "app_priv.h"

static const char *TAG = "app_reset";

// Factory reset timing
#define FACTORY_RESET_HOLD_TIME_MS  20000  // 20 seconds

// Reset state machine
enum class ResetState : uint8_t {
    IDLE,
    COUNTDOWN,      // Counting down with slow steady red blink
    FLASHING,       // Solid red - factory reset in progress (user can release)
};

static TimerHandle_t s_reset_timer = NULL;
static std::atomic<uint32_t> s_reset_start_time{0};
static std::atomic<ResetState> s_reset_state{ResetState::IDLE};
static std::atomic<uint8_t> s_flash_count{0};  // Flash sequence counter

// Set LED color for reset countdown (slow steady red blink)
static void set_reset_led(uint32_t elapsed_ms)
{
    if (!app_driver_led_lock()) return;

    led_strip_t *strip = app_driver_get_led_strip();
    if (!strip) {
        app_driver_led_unlock();
        return;
    }

    // Slow steady blink (no speed change)
    constexpr uint32_t blink_period = LED_RESET_BLINK_START_MS;
    bool led_on = ((elapsed_ms / (blink_period / 2)) % 2) == 0;

    if (led_on) {
        // Solid red at full intensity
        strip->set_pixel(strip, 0, LED_COLOR_RESET_R_MAX, LED_COLOR_RESET_G, LED_COLOR_RESET_B);  // RGB order: (R, G, B)
    } else {
        strip->set_pixel(strip, 0, 0, 0, 0);  // Off
    }
    strip->refresh(strip, LED_REFRESH_TIMEOUT_MS);
    app_driver_led_unlock();
}

// Delay before factory reset (solid red indicator)
#define RESET_SOLID_DELAY_MS  1000  // 1 second solid red before reset

// Set solid red LED (user can release button now)
static void set_solid_red_led(void)
{
    if (!app_driver_led_lock()) return;

    led_strip_t *strip = app_driver_get_led_strip();
    if (!strip) {
        app_driver_led_unlock();
        return;
    }

    // Solid red - factory reset in progress
    strip->set_pixel(strip, 0, LED_COLOR_RESET_R_MAX, LED_COLOR_RESET_G, LED_COLOR_RESET_B);  // RGB order: (R, G, B)
    strip->refresh(strip, LED_REFRESH_TIMEOUT_MS);
    app_driver_led_unlock();
}

// Timer callback for reset countdown and solid red phase
static void reset_timer_cb(TimerHandle_t timer)
{
    // Validate LED driver is available before proceeding
    if (!app_driver_get_led_strip()) {
        ESP_LOGW(TAG, "LED driver not initialized, cannot display reset countdown");
        return;
    }

    ResetState state = s_reset_state.load();

    if (state == ResetState::COUNTDOWN) {
        // Countdown phase - slow steady blink
        uint32_t start_time = s_reset_start_time.load();
        uint32_t elapsed_ms = (xTaskGetTickCount() * portTICK_PERIOD_MS) - start_time;

        set_reset_led(elapsed_ms);

        // Check if countdown complete - transition to solid red phase
        if (elapsed_ms >= FACTORY_RESET_HOLD_TIME_MS) {
            ESP_LOGW(TAG, "Factory reset triggered! User can release button.");
            s_reset_start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
            s_reset_state = ResetState::FLASHING;
            set_solid_red_led();  // Solid red - user can release button now
        }
    } else if (state == ResetState::FLASHING) {
        // Solid red phase - wait for delay then perform reset
        uint32_t start_time = s_reset_start_time.load();
        uint32_t elapsed_ms = (xTaskGetTickCount() * portTICK_PERIOD_MS) - start_time;

        if (elapsed_ms >= RESET_SOLID_DELAY_MS) {
            // Delay complete - perform factory reset
            xTimerStop(s_reset_timer, pdMS_TO_TICKS(100));
            s_reset_state = ResetState::IDLE;
            esp_matter::factory_reset();
        }
    }
}

static void button_long_press_start_cb(void *arg, void *data)
{
    // Only start if idle (not already in countdown or flashing)
    ResetState expected = ResetState::IDLE;
    if (!s_reset_state.compare_exchange_strong(expected, ResetState::COUNTDOWN)) {
        return;
    }

    ESP_LOGW(TAG, "Hold button for %u seconds to factory reset",
             static_cast<unsigned>(FACTORY_RESET_HOLD_TIME_MS / 1000));

    s_reset_start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Start timer for LED updates
    if (s_reset_timer && xTimerStart(s_reset_timer, pdMS_TO_TICKS(100)) != pdPASS) {
        ESP_LOGE(TAG, "Failed to start reset timer");
        s_reset_state = ResetState::IDLE;
    }
}

static void button_released_cb(void *arg, void *data)
{
    // Only cancel if in countdown phase (not if already flashing)
    ResetState expected = ResetState::COUNTDOWN;
    if (!s_reset_state.compare_exchange_strong(expected, ResetState::IDLE)) {
        return;
    }

    // Stop timer
    if (s_reset_timer) {
        xTimerStop(s_reset_timer, pdMS_TO_TICKS(100));
    }

    uint32_t elapsed_ms = (xTaskGetTickCount() * portTICK_PERIOD_MS) - s_reset_start_time.load();
    ESP_LOGI(TAG, "Factory reset cancelled after %u ms", static_cast<unsigned>(elapsed_ms));

    // Restore LED state (will be updated by attribute callback)
    if (app_driver_led_lock()) {
        led_strip_t *strip = app_driver_get_led_strip();
        if (strip) {
            strip->set_pixel(strip, 0, LED_COLOR_OFF_G, LED_COLOR_OFF_R, LED_COLOR_OFF_B);
            strip->refresh(strip, LED_REFRESH_TIMEOUT_MS);
        }
        app_driver_led_unlock();
    }
}

extern "C" esp_err_t app_reset_button_register(void *handle)
{
    if (!handle) {
        ESP_LOGE(TAG, "Handle cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    auto button_handle = static_cast<button_handle_t>(handle);
    esp_err_t err;

    // Pre-create reset timer to avoid allocation during operation
    s_reset_timer = xTimerCreate("reset", pdMS_TO_TICKS(LED_RESET_UPDATE_MS), pdTRUE, NULL, reset_timer_cb);
    if (!s_reset_timer) {
        ESP_LOGE(TAG, "Failed to create reset timer");
        return ESP_ERR_NO_MEM;
    }

    // Long press start triggers countdown
    err = iot_button_register_cb(button_handle, BUTTON_LONG_PRESS_START,
                                  button_long_press_start_cb, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register long press callback: %d", err);
        xTimerDelete(s_reset_timer, 0);
        s_reset_timer = NULL;
        return err;
    }

    // Release cancels if not complete
    err = iot_button_register_cb(button_handle, BUTTON_PRESS_UP,
                                  button_released_cb, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register press up callback: %d", err);
        xTimerDelete(s_reset_timer, 0);
        s_reset_timer = NULL;
        return err;
    }

    ESP_LOGI(TAG, "Factory reset handler registered (hold %us)",
             static_cast<unsigned>(FACTORY_RESET_HOLD_TIME_MS / 1000));
    return ESP_OK;
}
