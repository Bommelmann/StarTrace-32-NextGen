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
    int32_t uds_request_length;
    char *uds_request_string;
    int32_t uds_response_length;
    char *uds_response_string;
    enum {
        WEBSOCKET,
        BLE,
        HTTP,
        INTERNAL
    }Client_Interface;
} uds_message_string_t;



typedef struct led_actuation
{
    enum {
        RED,
        BLUE,
        GREEN,
        DEFAULT,
        }LED_color;
    uint16_t breaktime;
}led_actuation_t;

#endif
