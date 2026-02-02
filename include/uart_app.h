#ifndef UART_APP_H
#define UART_APP_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Initialize the UART driver and start the RX task
 */
void uart_app_init(void);

/**
 * @brief Send data out via UART
 * 
 * @param data Pointer to data buffer
 * @param len Length of data
 * @return int Bytes sent
 */
int uart_send_data(const char* data, int len);

#endif // UART_APP_H
