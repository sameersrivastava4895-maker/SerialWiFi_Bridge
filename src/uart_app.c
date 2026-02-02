#include "uart_app.h"
#include "common_def.h"
#include "socket_server.h" 

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/queue.h"

static QueueHandle_t uart_event_queue;

/**
 * @brief Task to handle UART events (RX)
 */
static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t* dtmp = (uint8_t*) malloc(UART_BUF_SIZE);

    ESP_LOGI(TAG_UART, "UART Event Task Started on Core %d", xPortGetCoreID());

    for (;;) {
        // Wait for UART event
        if (xQueueReceive(uart_event_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            bzero(dtmp, UART_BUF_SIZE);
            
            switch (event.type) {
                case UART_DATA:
                    // Read data from the UART buffer
                    int len = uart_read_bytes(UART_PORT_NUM, dtmp, event.size, portMAX_DELAY);
                    if (len > 0) {
                        ESP_LOGI(TAG_UART, "Recv %d bytes: %.*s", len, len, dtmp);
                        // Forward to TCP Socket
                        // We use a non-blocking or short-timeout send here safely? 
                        // For simplicity, let's call the bridge function directly or put in queue.
                        // Direct call is okay if socket sending is thread-safe (lwip write is generally ok).
                        // Better: Put into a Queue or StreamBuffer for the TCP task to read? 
                        // To keep it simple: Try to send directly to connected socket.
                        socket_send_data((const char*)dtmp, len);
                    }
                    break;
                
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG_UART, "hw fifo overflow");
                    uart_flush_input(UART_PORT_NUM);
                    xQueueReset(uart_event_queue);
                    break;
                
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG_UART, "ring buffer full");
                    uart_flush_input(UART_PORT_NUM);
                    xQueueReset(uart_event_queue);
                    break;

                default:
                    ESP_LOGI(TAG_UART, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void uart_app_init(void)
{
    ESP_LOGI(TAG_UART, "Initializing UART...");

    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Install UART driver, and get the queue.
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE * 2, UART_BUF_SIZE * 2, 20, &uart_event_queue, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    
    // Set UART pins (using default for Console for now, but explicit is good)
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Create the task to handle UART events
    xTaskCreatePinnedToCore(uart_event_task, "uart_event_task", UART_TASK_STACK, NULL, UART_TASK_PRIO, NULL, BRIDGE_CORE_ID);
}

int uart_send_data(const char* data, int len)
{
    // Write data to UART
    // This is a blocking call if tx buffer is full
    const int txBytes = uart_write_bytes(UART_PORT_NUM, data, len);
    return txBytes;
}
