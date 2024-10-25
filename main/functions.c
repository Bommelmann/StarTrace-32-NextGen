#include "functions.h"
#include "esp_log.h"

#define FUNCTIONS_TAG "functions"

uint8_t* hex_string_to_bytes(const uds_message_string_t* uds_message, size_t* out_length) {
    if (uds_message == NULL || uds_message->uds_string == NULL) {
        ESP_LOGE(FUNCTIONS_TAG, "uds_message or uds_string is NULL");
        return NULL;
    }

    size_t len = uds_message->msg_length;
    ESP_LOGI(FUNCTIONS_TAG, "uds_string length: %d", len);

    if (len % 2 != 0) {
        ESP_LOGE(FUNCTIONS_TAG, "Invalid uds_string length");
        return NULL; // Ungültige Länge
    }

    *out_length = len / 2;
    uint8_t* bytes = (uint8_t*)malloc(*out_length);
    if (bytes == NULL) {
        ESP_LOGE(FUNCTIONS_TAG, "Memory allocation failed");
        return NULL; // Speicherzuweisung fehlgeschlagen
    }

    for (size_t i = 0; i < *out_length; ++i) {
        sscanf(uds_message->uds_string + 2 * i, "%2hhx", &bytes[i]);
    }

    return bytes;
}



void log_hex_data(const char* tag, const uint8_t* data, size_t length) {
    char buffer[3 * length + 1]; // 2 Zeichen pro Byte + 1 Leerzeichen + Null-Terminierung
    char* ptr = buffer;

    for (size_t i = 0; i < length; ++i) {
        ptr += sprintf(ptr, "%02X ", data[i]);
    }

    ESP_LOGI(tag, "Hex Data: %s", buffer);
}


void log_send_message(const char* tag, const send_message_can_t* msg) {
    ESP_LOGD(tag, "send_message_t:");
    ESP_LOGD(tag, "  tx_id: 0x%08" PRIX32, msg->tx_id); // Verwenden von PRIX32 für uint32_t
    ESP_LOGD(tag, "  msg_length: %" PRIi32, msg->msg_length); // Verwenden von PRIi32 für int32_t
    ESP_LOGD(tag, "  reuse_buffer: %s", msg->reuse_buffer ? "true" : "false");

    char buffer[3 * msg->msg_length + 1]; // 2 Zeichen pro Byte + 1 Leerzeichen + Null-Terminierung
    char* ptr = buffer;

    for (size_t i = 0; i < msg->msg_length; ++i) {
        ptr += sprintf(ptr, "%02X ", msg->buffer[i]);
    }

    ESP_LOGD(tag, "  buffer: %s", buffer);
}