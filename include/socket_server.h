#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

/**
 * @brief Initialize and start the TCP Server Task
 */
void socket_server_init(void);

/**
 * @brief Send data to the connected TCP client
 * 
 * @param data Pointer to data
 * @param len Length of data
 * @return int Bytes sent, or <0 on error
 */
int socket_send_data(const char* data, int len);

#endif // SOCKET_SERVER_H
