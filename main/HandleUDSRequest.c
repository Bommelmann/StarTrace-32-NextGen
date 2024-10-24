#include "HandleUDSRequests.h"

#define EXAMPLE_P2_CLIENT   pdMS_TO_TICKS(CONFIG_P2_CLIENT)

void handle_uds_request_task(){

            //String received from handle_uds_request_queue
            
            //length of uds_message_hex
            size_t length;
            //string received from handle_uds_request_queue converted to hex
            uint8_t * uds_message_hex;
            //message for the isotp_send_message_queue
            send_message_can_t uds_message_hex_queue_send;
            //received container from isotp_processing_task
            IsoTpLinkContainer *received_container;

    while (1){
         
            vTaskDelay(1000 / portTICK_PERIOD_MS);

                    //Test: Receive from uds queue
            uds_message_string_t uds_message_string_queue_receive;
            //Receive from handle_uds_request_queue and block task as long nothing is received
           if(xQueueReceive(handle_uds_request_queue, &uds_message_string_queue_receive, portMAX_DELAY)==pdPASS){
                if ( uds_message_string_queue_receive.uds_string != NULL)
                {
                    // Logge die Nachricht und ihre Länge
                    ESP_LOGI(UDS_TAG, "Message length: %d", (int)uds_message_string_queue_receive.msg_length);
                    ESP_LOGI(UDS_TAG, "UDS String: %s", uds_message_string_queue_receive.uds_string);
                }
                else
                {
                    ESP_LOGI(UDS_TAG, "string is NULL");
                }
                

                        //Convert string to hex, Übergabge Parameter 1: uds_message_string_t, Rückgabe Parameter 2: length
                        uds_message_hex = hex_string_to_bytes(&uds_message_string_queue_receive, &length);

                        if (uds_message_hex != NULL) {
                                // Log the hex data
                                log_hex_data(UDS_TAG, uds_message_hex, length);

                                // Extract the first 4 bytes from uds_message_hex and write to msg.tx_id
                                uint32_t tx_id = (uds_message_hex[0] << 24) | (uds_message_hex[1] << 16) | (uds_message_hex[2] << 8) | uds_message_hex[3];
                                // Send in isotp queue
                                uds_message_hex_queue_send.tx_id = tx_id;

                                // Allocate memory for the buffer
                                size_t buffer_length = length - sizeof(tx_id);
                                uds_message_hex_queue_send.buffer = (uint8_t *)malloc(buffer_length);
                                if (uds_message_hex_queue_send.buffer == NULL) {
                                    // Fehlerbehandlung bei fehlgeschlagener Speicherzuweisung
                                    ESP_LOGE(UDS_TAG, "Failed to allocate memory for buffer");
                                    free(uds_message_hex);
                                    return;
                                }

                                // Copy the remaining bytes to msg.buffer
                                memcpy(uds_message_hex_queue_send.buffer, uds_message_hex + sizeof(tx_id), buffer_length);

                                uds_message_hex_queue_send.msg_length = buffer_length;
                                uds_message_hex_queue_send.reuse_buffer = true;

                                log_send_message(UDS_TAG, &uds_message_hex_queue_send);
            };
                        //Request versenden
                        // Senden der Nachricht in die Queue
                        if (xQueueSend(isotp_send_message_queue, &uds_message_hex_queue_send, portMAX_DELAY) != pdPASS) {
                            ESP_LOGE(UDS_TAG, "Failed to send message to isoTP_message_queue");
                        } else {
                            ESP_LOGI(UDS_TAG, "Message sent to isoTP_message_queue");
                        }

                        //Response empfangen
                        // Receive the response from the queue
                        if (xQueueReceive(handle_uds_request_queue, &received_container, EXAMPLE_P2_CLIENT) == pdPASS) {
                    
                        //Print out received data                                   
                        ESP_LOGI(UDS_TAG, "Received ISO-TP message with length: %04X", received_container->link.receive_size);
                            for (int i = 0; i < received_container->link.receive_size; i++) {
                                ESP_LOGI(UDS_TAG, "payload_buf[%d] = %02x", i, received_container->payload_buf[i]);
                            }
                        } 


                }
                else {
                        ESP_LOGE(UDS_TAG, "Failed to convert string to hex");
                     }
                     
        }
}
