/*
   M5NanoC6 Matter Switch - Main Application

   Creates a Matter on_off_plug_in_unit device with:
   - WS2812 LED indicator (bright blue=on, dim blue=off)
   - Button for local toggle control
   - Thread networking
*/

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>  // Required for CONFIG_ENABLE_OTA_REQUESTOR

#include <iot_button.h>
#include <common_macros.h>
#include <app_priv.h>
#include "app_reset.h"

#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <esp_wifi.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ESP32/OpenthreadLauncher.h>

// OpenThread platform configuration
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                           \
    {                                                                                   \
        .radio_mode = RADIO_MODE_NATIVE,                                                \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                            \
    {                                                                                   \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,                              \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                            \
    {                                                                                   \
        .storage_partition_name = "nvs", .netif_queue_size = 10, .task_queue_size = 10, \
    }
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

#include "include/CHIPProjectConfig.h"
#include <esp_app_desc.h>

static const char *TAG = "app_main";

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;

// Driver handles
static app_driver_handle_t s_led_handle = NULL;
static app_driver_handle_t s_button_handle = NULL;
static uint16_t s_switch_endpoint_id = 0;

// Cached attribute pointer for fast button toggle
static attribute_t *s_onoff_attribute = NULL;

// Cluster/attribute IDs (compile-time constants)
static constexpr uint32_t ONOFF_CLUSTER_ID = OnOff::Id;
static constexpr uint32_t ONOFF_ATTRIBUTE_ID = OnOff::Attributes::OnOff::Id;

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG, "Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG, "Commissioning session stopped");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG, "Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG, "Commissioning window closed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
        ESP_LOGI(TAG, "Fabric removed successfully");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
            if (!commissionMgr.IsCommissioningWindowOpen()) {
                CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                                            chip::CommissioningWindowAdvertisement::kDnssdOnly);
                if (err != CHIP_NO_ERROR) {
                    ESP_LOGE(TAG, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                }
            }
        }
        break;
    }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        ESP_LOGI(TAG, "Fabric will be removed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        ESP_LOGI(TAG, "Fabric is updated");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        ESP_LOGI(TAG, "Fabric is committed");
        break;

    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
        ESP_LOGI(TAG, "BLE deinitialized and memory reclaimed");
        break;

    default:
        break;
    }
}

static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);

    if (type == identification::callback_type_t::START || type == identification::callback_type_t::EFFECT) {
        app_driver_led_identify_start();
    } else if (type == identification::callback_type_t::STOP) {
        // Get current OnOff state to restore LED
        bool current_power = false;
        if (s_onoff_attribute) {
            esp_matter_attr_val_t val = esp_matter_invalid(NULL);
            attribute::get_val(s_onoff_attribute, &val);
            current_power = val.val.b;
        }
        app_driver_led_identify_stop(current_power);
    }

    return ESP_OK;
}

static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    esp_err_t err = ESP_OK;

    if (type == PRE_UPDATE) {
        auto driver_handle = static_cast<app_driver_handle_t>(priv_data);
        err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
    }

    return err;
}

// Button callback to toggle switch state
static void button_toggle_cb(void *arg, void *data)
{
    if (!s_onoff_attribute) {
        ESP_LOGW(TAG, "OnOff attribute not cached");
        return;
    }

    // Get current OnOff state using cached attribute pointer (fast path)
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(s_onoff_attribute, &val);
    bool current_state = val.val.b;

    // Toggle the state
    val.val.b = !current_state;
    ESP_LOGD(TAG, "Button: toggle %d -> %d", current_state, val.val.b);

    // Update the attribute (this will trigger the callback and update the LED)
    attribute::update(s_switch_endpoint_id, ONOFF_CLUSTER_ID, ONOFF_ATTRIBUTE_ID, &val);
}

// Get current on/off power state (used by app_reset to restore LED after cancelled reset)
extern "C" bool app_get_current_power_state(void)
{
    if (!s_onoff_attribute) {
        return false;
    }
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(s_onoff_attribute, &val);
    return val.val.b;
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    // Initialize NVS with error recovery
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition corrupted, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Initialize LED driver first (for visual feedback)
    s_led_handle = app_driver_led_init();
    if (!s_led_handle) {
        ESP_LOGE(TAG, "Failed to initialize LED driver");
    }

    // Create Matter node (product name set via CHIPProjectConfig.h)
    node::config_t node_config = {};  // Zero-initialize all members
    // Set default NodeLabel (user-configurable label after commissioning)
    strncpy(node_config.root_node.basic_information.node_label, "M5NanoC6 Switch",
            sizeof(node_config.root_node.basic_information.node_label) - 1);
    node_config.root_node.basic_information.node_label[sizeof(node_config.root_node.basic_information.node_label) - 1] = '\0';
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG, "Failed to create Matter node"));

    // Create on_off_plug_in_unit endpoint
    on_off_plug_in_unit::config_t plug_config = {};  // Zero-initialize all members
    plug_config.on_off.on_off = false;  // Start in OFF state
    endpoint_t *endpoint = on_off_plug_in_unit::create(node, &plug_config, ENDPOINT_FLAG_NONE, s_led_handle);
    ABORT_APP_ON_FAILURE(endpoint != nullptr, ESP_LOGE(TAG, "Failed to create plug endpoint"));

    s_switch_endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Created on_off_plug_in_unit endpoint with ID %d", s_switch_endpoint_id);

    // Cache OnOff attribute pointer for fast button toggle
    s_onoff_attribute = attribute::get(s_switch_endpoint_id, ONOFF_CLUSTER_ID, ONOFF_ATTRIBUTE_ID);
    if (!s_onoff_attribute) {
        ESP_LOGW(TAG, "Failed to cache OnOff attribute");
    }

    // Initialize button and register callbacks
    s_button_handle = app_driver_button_init();
    if (s_button_handle) {
        iot_button_register_cb(static_cast<button_handle_t>(s_button_handle), BUTTON_SINGLE_CLICK, button_toggle_cb, NULL);
        app_reset_button_register(s_button_handle);
        ESP_LOGI(TAG, "Button initialized with toggle and factory reset callbacks");
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    // Set OpenThread platform config
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
#endif

    // Start Matter
    err = esp_matter::start(app_event_cb);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));

#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD
    // Log WiFi provisioning status
    if (!chip::DeviceLayer::ConnectivityMgr().IsWiFiStationProvisioned()) {
        ESP_LOGI(TAG, "WiFi not provisioned - AP mode active for commissioning");
    }
#endif

    const esp_app_desc_t *app_desc = esp_app_get_description();
    ESP_LOGI(TAG, "M5NanoC6 Matter Switch v%s started", app_desc->version);

    // Log commissioning info from CHIPProjectConfig.h
    // To change these values, edit main/include/CHIPProjectConfig.h
    // and regenerate using: python3 scripts/generate_pairing_config.py
    ESP_LOGI(TAG, "=== Commissioning Info ===");
    ESP_LOGI(TAG, "Discriminator: %d (0x%03X)",
             CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR,
             CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR);
    ESP_LOGI(TAG, "Passcode: %d", CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE);
    ESP_LOGI(TAG, "Run 'scripts/generate_pairing_config.py' for QR code");
    ESP_LOGI(TAG, "==========================");

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
#if CONFIG_OPENTHREAD_CLI
    esp_matter::console::otcli_register_commands();
#endif
    esp_matter::console::init();
#endif
}
