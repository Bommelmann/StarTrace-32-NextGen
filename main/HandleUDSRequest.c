#include "HandleUDSRequests.h"

#define EXAMPLE_P2_CLIENT   pdMS_TO_TICKS(CONFIG_P2_CLIENT)

void handle_uds_request_task(){

//Concept description #############################
//################################################# 
//Task is blocked until uds_message_string_t is received via handle_uds_request_queue
//Then the string is converted into a hex via hex_string_to_bytes()
//Then the hex bytes are written into send_message_can_t and sent via isotp_send_message_queue to isotp_send_queue_task to send the diagnostic request
//Then this task is blocked until a diagnostic response is received via handle_uds_request_queue_container
//Then it is checked, if the response fits to the request

//length of uds_message_hex
size_t length;
//string received from handle_uds_request_queue converted to hex
uint8_t * uds_message_hex;
//message for the isotp_send_message_queue
send_message_can_t uds_message_hex_queue_send;
//received container from isotp_processing_task
IsoTpLinkContainer *received_container;

    while (1){
        uds_message_string_t uds_message_string_queue_receive;

        // Receive from handle_uds_request_queue and block task as long nothing is received
        if (xQueueReceive(handle_uds_request_queue, &uds_message_string_queue_receive, portMAX_DELAY) == pdPASS) {
            if (uds_message_string_queue_receive.uds_string != NULL) {
                ESP_LOGD(UDS_TAG, "UDS String Length: %d", (int)uds_message_string_queue_receive.msg_length);
                ESP_LOGD(UDS_TAG, "UDS String Content: %s", uds_message_string_queue_receive.uds_string);

                // Convert string to hex, Übergabge Parameter 1: uds_message_string_t, Rückgabe Parameter 2: length
                uds_message_hex = hex_string_to_bytes(&uds_message_string_queue_receive, &length);

                if (uds_message_hex != NULL) {
                    // Log the hex data
                    log_hex_data(UDS_TAG, uds_message_hex, length);

                    // Extract the first 4 bytes from uds_message_hex and write to msg.tx_id
                    uint32_t tx_id = (uds_message_hex[0] << 24) | (uds_message_hex[1] << 16) | (uds_message_hex[2] << 8) | uds_message_hex[3];
                    uds_message_hex_queue_send.tx_id = tx_id;

                    // Extract the rest of the bytes from uds_message_hex an write to msg.buffer
                    size_t buffer_length = length - sizeof(tx_id);
                    uds_message_hex_queue_send.buffer = (uint8_t *)malloc(buffer_length);

                    if (uds_message_hex_queue_send.buffer != NULL) {
                        // Copy the remaining bytes to msg.buffer
                        memcpy(uds_message_hex_queue_send.buffer, uds_message_hex + sizeof(tx_id), buffer_length);
                        uds_message_hex_queue_send.msg_length = buffer_length;
                        uds_message_hex_queue_send.reuse_buffer = true;
                        log_send_message(UDS_TAG, &uds_message_hex_queue_send);
                        free(uds_message_hex);
                    } else {
                        ESP_LOGE(UDS_TAG, "Failed to allocate memory for buffer");
                        continue;
                    }
                } else {
                    ESP_LOGE(UDS_TAG, "Failed to convert string to hex");
                    continue;
                }
            } else {
                ESP_LOGE(UDS_TAG, "UDS string is NULL");
                continue;
            }

            // Request versenden
            // Senden der Nachricht in die Queue
            if (xQueueSend(isotp_send_message_queue, &uds_message_hex_queue_send, portMAX_DELAY) != pdPASS) {
                ESP_LOGE(UDS_TAG, "Failed to send message to isoTP_message_queue");
            } else {
                            ESP_LOGI(UDS_TAG, "Message sent to isoTP_message_queue");
                        }

                        //Response empfangen
                        // Receive the response from the queue
                        if (xQueueReceive(handle_uds_request_queue_container, &received_container, EXAMPLE_P2_CLIENT) == pdPASS) {
                    
                        //Print out received data                                   
                            ESP_LOGI(UDS_TAG, "Received ISO-TP message with length: %04X", received_container->link.receive_size);
                                for (int i = 0; i < received_container->link.receive_size; i++) {
                                    ESP_LOGI(UDS_TAG, "payload_buf[%d] = %02x", i, received_container->payload_buf[i]);
                                }
                            
                            // Check if the response fits to the request
                            //First Check for the correct ID
                            //ESP_LOGI(UDS_TAG, "Received container: send_arbitration_id = 0x%08X", (unsigned int)received_container->link.send_arbitration_id);
                            //ESP_LOGI(UDS_TAG, "uds_message_hex_queue_send.tx_id = 0x%08X", (unsigned int)uds_message_hex_queue_send.tx_id);
                            if (received_container->link.send_arbitration_id == uds_message_hex_queue_send.tx_id) {
                                //Then check for the correct payload
                                //First check if it is a negative response
                                    ESP_LOGI(UDS_TAG, "received_container->payload_buf[0] = %02x", received_container->payload_buf[0]);
                                    ESP_LOGI(UDS_TAG, "uds_message_hex_queue_send.buffer[1] = %02x", uds_message_hex_queue_send.buffer[1]);
                                    ESP_LOGI(UDS_TAG, "uds_message_hex_queue_send.buffer[1] | 0x40  = %02x", uds_message_hex_queue_send.buffer[1] | 0x40 );
                                    if(received_container->payload_buf[0] == 0x7f) {
                                        //Then check if the subsequent bytes match the request
                                        if(received_container->payload_buf[1] == uds_message_hex_queue_send.buffer[0]){
                                            ESP_LOGI(UDS_TAG, "NRC: Received ISO-TP message matches request");
                                        
                                        }
                                    //Secondly check if it is a positive response

                                    }else if(received_container->payload_buf[0] == (uds_message_hex_queue_send.buffer[0] | 0x40 )){
                                        ESP_LOGI(UDS_TAG, "Positive Response: Received ISO-TP message matches request");
                                    }
                            }        

                            if (uds_message_hex_queue_send.buffer != NULL) {
                                free(uds_message_hex_queue_send.buffer);
                            }
                        }        

        }                  
    }
}

