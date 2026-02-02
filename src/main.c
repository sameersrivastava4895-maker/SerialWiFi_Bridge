#include "common_def.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "wifi_app.h"
#include "uart_app.h"

void app_main(void)
{
    ESP_LOGI(TAG_BRIDGE, "Starting ESP32 USB-UART <-> WiFi Bridge");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize UART Driver and Task
    uart_app_init();

    // Initialize WiFi (which will start Socket Server upon Connection)
    wifi_app_init();
}