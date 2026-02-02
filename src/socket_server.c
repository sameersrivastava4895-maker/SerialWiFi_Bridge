#include "socket_server.h"
#include "common_def.h"
#include "uart_app.h"

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

static int connected_socket = -1;

/**
 * @brief Send data to the connected client
 * Safe to call from other tasks (like UART task)
 */
int socket_send_data(const char* data, int len)
{
    if (connected_socket < 0) {
        return -1; // No connection
    }
    // send() is generally thread safe in lwIP for different sockets, but here we might have contention
    // if multiple tasks write to same socket. 
    // In this simple bridge, only UART task writes to socket. 
    // If the TCP Loop also writes (echo?), we might need a mutex.
    // For now, assume single writer (UART Task).
    int written = send(connected_socket, data, len, 0);
    if (written < 0) {
        ESP_LOGE(TAG_BRIDGE, "Error occurred during sending: errno %d", errno);
    }
    return written;
}

static void tcp_server_task(void *pvParameters)
{
    char rx_buffer[1024]; // Rx Buffer
    int addr_family = AF_INET;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(TCP_PORT);
    ip_protocol = IPPROTO_IP;

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG_BRIDGE, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    // Bind socket
    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG_BRIDGE, "Socket unable to bind: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG_BRIDGE, "Socket bound, port %d", TCP_PORT);

    err = listen(listen_sock, TCP_LISTEN_QUEUE);
    if (err != 0) {
        ESP_LOGE(TAG_BRIDGE, "Error occurred during listen: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        ESP_LOGI(TAG_BRIDGE, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        
        // Accept connection
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG_BRIDGE, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set the global connected socket so UART task can write to it
        connected_socket = sock;
        ESP_LOGI(TAG_BRIDGE, "Socket accepted");

        // Loop to receive data
        while (1) {
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0) {
                ESP_LOGE(TAG_BRIDGE, "recv failed: errno %d", errno);
                break;
            } else if (len == 0) {
                ESP_LOGI(TAG_BRIDGE, "Connection closed");
                break;
            } else {
                rx_buffer[len] = 0; // Null-terminate for logging (optional, be careful with binary)
                ESP_LOGI(TAG_BRIDGE, "Received %d bytes from Wifi", len);
                
                // Forward to UART
                uart_send_data(rx_buffer, len);
            }
        }

        if (sock != -1) {
            ESP_LOGI(TAG_BRIDGE, "Shutting down socket");
            shutdown(sock, 0);
            close(sock);
            connected_socket = -1;
        }
    }

    vTaskDelete(NULL);
}

void socket_server_init(void)
{
    xTaskCreatePinnedToCore(tcp_server_task, "tcp_server", TCP_TASK_STACK, NULL, TCP_TASK_PRIO, NULL, BRIDGE_CORE_ID);
}
