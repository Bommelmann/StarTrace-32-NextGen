#ifndef __MESSAGES_H__
#define __MESSAGES_H__

// Simple struct for a dynamically sized send message

#include <stdint.h>
#include <stdbool.h> // Für bool

typedef struct send_message
{
    int32_t msg_length;
    uint8_t *buffer;
    uint32_t rx_id;
    uint32_t tx_id;
    bool reuse_buffer;
} send_message_can_t;

typedef struct uds_message_string
{
    int32_t msg_length;
    char *uds_string;
    enum {
        WEBSOCKET,
        BLE,
        HTTP,
        INTERNAL
    }Sending_Instance;
} uds_message_string_t;


#endif
