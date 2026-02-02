#ifndef COMMON_DEF_H
#define COMMON_DEF_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"

// =======================================
// WiFi Configuration
// =======================================
#define WIFI_SSID           "MyEsp32Bridge"    // CHANGE THIS
#define WIFI_PASS           "12345678"         // CHANGE THIS
#define WIFI_MAXIMUM_RETRY  5

// =======================================
// TCP Server Configuration
// =======================================
#define TCP_PORT            3333
#define TCP_LISTEN_QUEUE    1

// =======================================
// UART Configuration
// =======================================
#define UART_PORT_NUM       UART_NUM_0         // Using UART0 (USB Serial) for ease of testing
#define UART_BAUD_RATE      115200
#define UART_TX_PIN         1                  // Default TX pin for ESP32-S3 UART0
#define UART_RX_PIN         3                  // Default RX pin for ESP32-S3 UART0
#define UART_BUF_SIZE       1024               // Driver buffer size

// =======================================
// Task Configuration
// =======================================
// All tasks pinned to Core 0 for "Single Core" requirement
#define BRIDGE_CORE_ID      0 

#define UART_TASK_STACK     4096
#define UART_TASK_PRIO      2

#define WIFI_TASK_STACK     4096
// TCP task priority should be handled carefully
#define TCP_TASK_STACK      4096
#define TCP_TASK_PRIO       2

// =======================================
// Global Tags for Logging
// =======================================
#define TAG_UART            "UART_APP"
#define TAG_WIFI            "WIFI_APP"
#define TAG_BRIDGE          "BRIDGE_APP"

#endif // COMMON_DEF_H
