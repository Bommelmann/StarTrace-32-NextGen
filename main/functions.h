#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "messages.h"


void log_hex_data(const char* tag, const uint8_t* data, size_t length);
uint8_t* hex_string_to_bytes(const uds_message_string_t* uds_message, size_t* out_length);
void log_send_message(const char* tag, const send_message_can_t* msg);