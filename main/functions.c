#include "functions.h"
#include "esp_log.h"

#define FUNCTIONS_TAG "functions"

uint8_t* hex_string_to_bytes(const uds_message_string_t* uds_message, size_t* out_length) {
    if (uds_message == NULL || uds_message->uds_request_string == NULL) {
        ESP_LOGE(FUNCTIONS_TAG, "uds_message or uds_request_string is NULL");
        return NULL;
    }

    size_t len = uds_message->uds_request_length;
    ESP_LOGD(FUNCTIONS_TAG, "uds_request_string length: %d", len);

    if (len % 2 != 0) {
        ESP_LOGE(FUNCTIONS_TAG, "Invalid uds_request_string length");
        return NULL; // Ungültige Länge
    }

    *out_length = len / 2;
    uint8_t* bytes = (uint8_t*)malloc(*out_length);
    if (bytes == NULL) {
        ESP_LOGE(FUNCTIONS_TAG, "Memory allocation failed");
        return NULL; // Speicherzuweisung fehlgeschlagen
    }

    for (size_t i = 0; i < *out_length; ++i) {
        sscanf(uds_message->uds_request_string + 2 * i, "%2hhx", &bytes[i]);
    }

    return bytes;
}



void log_hex_data(const char* tag, const uint8_t* data, size_t length) {
    char buffer[3 * length + 1]; // 2 Zeichen pro Byte + 1 Leerzeichen + Null-Terminierung
    char* ptr = buffer;

    for (size_t i = 0; i < length; ++i) {
        ptr += sprintf(ptr, "%02X ", data[i]);
    }

    ESP_LOGD(tag, "Hex Data: %s", buffer);
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

//Function for when the ECU doesnt respond, to create an artificial response
void create_artificial_response (uds_message_string_t *uds_rqst_rspns_string) {

    if (uds_rqst_rspns_string->uds_request_string == NULL || uds_rqst_rspns_string->uds_response_string == NULL) {
    ESP_LOGE("create_artificial_response", "Invalid pointer received.");
    return; // Funktion abbrechen, wenn Zeiger ungültig sind
    }
    // 1. Kopiere die ersten 4 Zeichen unverändert
    strncpy(uds_rqst_rspns_string->uds_response_string, uds_rqst_rspns_string->uds_request_string, 4);

    // 2. Vertausche Zeichen 5&6 mit 7&8
    uds_rqst_rspns_string->uds_response_string[4] = uds_rqst_rspns_string->uds_request_string[6];
    uds_rqst_rspns_string->uds_response_string[5] = uds_rqst_rspns_string->uds_request_string[7];
    uds_rqst_rspns_string->uds_response_string[6] = uds_rqst_rspns_string->uds_request_string[4];
    uds_rqst_rspns_string->uds_response_string[7] = uds_rqst_rspns_string->uds_request_string[5];

    // 3. Setze Zeichen 9&10 auf "7F"
    uds_rqst_rspns_string->uds_response_string[8] = '7';
    uds_rqst_rspns_string->uds_response_string[9] = 'F';

    // 4. Übertrage Zeichen 9&10 aus uds_request_string auf Stellen 11&12
    uds_rqst_rspns_string->uds_response_string[10] = uds_rqst_rspns_string->uds_request_string[8];
    uds_rqst_rspns_string->uds_response_string[11] = uds_rqst_rspns_string->uds_request_string[9];

    // 5. Setze Zeichen 13&14 auf "FF"
    uds_rqst_rspns_string->uds_response_string[12] = 'F';
    uds_rqst_rspns_string->uds_response_string[13] = 'F';

    // Null-terminiere den String
    uds_rqst_rspns_string->uds_response_string[14] = '\0';
}

void bytes_to_hex_string(IsoTpLinkContainer *uds_rspns_isotp, uds_message_string_t *uds_rqst_rspns_string){
// 1. HexCode für receive_arbitration_id erstellen
    char arbitration_id_hex[9]; // Maximal 8 Zeichen für Hex und 1 für Null-Terminierung
    snprintf(arbitration_id_hex, sizeof(arbitration_id_hex), "%08lX", uds_rspns_isotp->link.receive_arbitration_id);

    // 2. HexCode für payload_buf erstellen
    int payload_length = uds_rspns_isotp->link.receive_size;
    char *payload_hex = malloc(payload_length * 2 + 1); // 2 Zeichen pro Byte + 1 für Null-Terminierung

    if (payload_hex == NULL) {
        ESP_LOGE("bytes_to_hex_string", "Memory allocation for payload_hex failed.");
        return;
    }

    for (int i = 0; i < payload_length; i++) {
        snprintf(payload_hex + i * 2, 3, "%02X", uds_rspns_isotp->payload_buf[i]);
    }

    // 3. Gesamtzahl der Zeichen berechnen (Arbitration ID + Payload)
    int total_length = 8 + payload_length * 2; // 8 Zeichen für Arbitration ID + 2 * Payload Length

    // 4. Speicher für uds_response_string allokieren
    uds_rqst_rspns_string->uds_response_string = malloc(total_length + 1); // +1 für Null-Terminierung

    if (uds_rqst_rspns_string->uds_response_string == NULL) {
        ESP_LOGE("bytes_to_hex_string", "Memory allocation for uds_response_string failed.");
        free(payload_hex); // Aufräumen, wenn der Speicher für uds_response_string nicht allokiert werden konnte
        return;
    }

    // 5. Hex-Codes in uds_response_string zusammenfügen
    snprintf(uds_rqst_rspns_string->uds_response_string, total_length + 1, "%s%s", arbitration_id_hex, payload_hex);

    // 6. Länge der neuen Zeichenkette speichern
    uds_rqst_rspns_string->uds_response_length = total_length;

    // 7. Aufräumen des temporären Buffers
    free(payload_hex);


}

void configureLEDs(){
    gpio_set_direction(5, GPIO_MODE_OUTPUT);
    gpio_set_direction(6, GPIO_MODE_OUTPUT);
    gpio_set_direction(7, GPIO_MODE_OUTPUT);

}

void actuateLEDs(LED_color color){
    
    //Rot
    switch (color){
        case RED:
            gpio_set_level(5, 1);
            gpio_set_level(6, 0);
            gpio_set_level(7, 0);
            break;
        case GREEN:
            gpio_set_level(5, 0);
            gpio_set_level(6, 1);
            gpio_set_level(7, 0);
            break;
        case BLUE:
            gpio_set_level(5, 0);
            gpio_set_level(6, 0);
            gpio_set_level(7, 1); 
            break;            
    }

}