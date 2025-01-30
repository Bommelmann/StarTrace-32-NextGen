#include "HandleUDSRequests.h"

#define P2_CLIENT   pdMS_TO_TICKS(CONFIG_P2_CLIENT)

void handle_uds_request_task(){

//Actuate LED ###################################
//#################################################
led_actuation_order.LED_color=DEFAULT;
led_actuation_order.breaktime=50;
xQueueSend(handle_led_actuation_queue, &led_actuation_order, portMAX_DELAY);

//Concept description #############################
//################################################# 
//Task is blocked until uds_message_string_t is received via handle_uds_request_queue
//Then the string is converted into a hex via hex_string_to_bytes()
//Then the hex bytes are written into send_message_can_t and sent via isotp_send_message_queue to isotp_send_queue_task to send the diagnostic request
//Then this task is blocked until a diagnostic response is received via handle_uds_request_queue_container
//Then it is checked, if the response fits to the request
//If no diagnostic response is received within EXAMPLE_P2_CLIENT ticks, a negative response is artificially sent

//length of uds_rqst_hex
size_t length;
//uds_message_string_t object, which contains both request and response data
uds_message_string_t uds_rqst_rspns_string;
//string received from handle_uds_request_queue converted to hex
uint8_t * uds_rqst_hex;
//message for the isotp_send_message_queue
send_message_can_t uds_rqst_isotp;
//received container from isotp_processing_task
IsoTpLinkContainer *uds_rspns_isotp;

    while (1){

        // Receive from handle_uds_request_queue and block task as long nothing is received
        if (xQueueReceive(handle_uds_request_queue, &uds_rqst_rspns_string, portMAX_DELAY) == pdPASS) {
            ESP_LOGE(UDS_TAG, "Received request in HandleUDSRequest Queue");
            //Actuate LED ###################################
            //#################################################
            led_actuation_order.LED_color=DEFAULT;
            led_actuation_order.breaktime=20;
            xQueueSend(handle_led_actuation_queue, &led_actuation_order, portMAX_DELAY);
            if (uds_rqst_rspns_string.uds_request_string != NULL) {
                ESP_LOGD(UDS_TAG, "UDS String Length: %d", (int)uds_rqst_rspns_string.uds_request_length);
                ESP_LOGD(UDS_TAG, "UDS String Content: %s", uds_rqst_rspns_string.uds_request_string);

                // Convert string to hex, Übergabge Parameter 1: uds_message_string_t, Rückgabe Parameter 2: length
                uds_rqst_hex = hex_string_to_bytes(&uds_rqst_rspns_string, &length);

                if (uds_rqst_hex != NULL) {
                    // Log the hex data
                    log_hex_data(UDS_TAG, uds_rqst_hex, length);

                    // Extract the first 4 bytes from uds_rqst_hex and write to msg.tx_id
                    uint32_t tx_id = (uds_rqst_hex[0] << 24) | (uds_rqst_hex[1] << 16) | (uds_rqst_hex[2] << 8) | uds_rqst_hex[3];
                    uds_rqst_isotp.tx_id = tx_id;

                    // Extract the rest of the bytes from uds_rqst_hex an write to msg.buffer
                    size_t buffer_length = length - sizeof(tx_id);
                    uds_rqst_isotp.buffer = (uint8_t *)malloc(buffer_length);

                    if (uds_rqst_isotp.buffer != NULL) {
                        // Copy the remaining bytes to msg.buffer
                        memcpy(uds_rqst_isotp.buffer, uds_rqst_hex + sizeof(tx_id), buffer_length);
                        uds_rqst_isotp.msg_length = buffer_length;
                        uds_rqst_isotp.reuse_buffer = true;
                        log_send_message(UDS_TAG, &uds_rqst_isotp);
                        free(uds_rqst_hex);
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
            if (xQueueSend(isotp_send_message_queue, &uds_rqst_isotp, portMAX_DELAY) != pdPASS) {
                ESP_LOGE(UDS_TAG, "Failed to send message to isoTP_message_queue");
            } else {
                ESP_LOGE(UDS_TAG, "Message sent to isoTP_message_queue");
            }
            ESP_LOGD("P2_LOG", "CONFIG_P2_CLIENT: %d ms", (int)CONFIG_P2_CLIENT);
            ESP_LOGD("P2_LOG", "P2_CLIENT: %d ticks", (int)P2_CLIENT);
                        
            // Receive the response from the queue
            //Block Task for the timespan of EXAMPLE_P2_CLIENT which is specified in SDK Config
            //If nothing is received within EXAMPLE_P2_CLIENT, the task goes to the else statement

            /////////NRC 0x78 Handling////////////////////////
            //A while loop has to be implemented, to cover the NRC 0x78
            //0x78 (Response Pending) can be sent unlimited times by the ECU, until the actual response is sent
            //This is why we have to wait for the actual response
            bool response_pending = true;
            while(response_pending){
                //Set response_pending directly to false, it will be set to true, only in the sections, where the continue statement is used
                response_pending = false;
                if (xQueueReceive(handle_uds_request_queue_container, &uds_rspns_isotp, P2_CLIENT) == pdPASS) {
                    ESP_LOGE(UDS_TAG,"Received ISO-TP message from isotp_processing_task");
                    //ESP_LOGI(UDS_TAG, "Received response from isotp_processing_task");
                    //Print out received data                                   
                    ESP_LOGD(UDS_TAG, "Received ISO-TP message with length: %04X", uds_rspns_isotp->link.receive_size);
                        for (int i = 0; i < uds_rspns_isotp->link.receive_size; i++) {
                            ESP_LOGD(UDS_TAG, "payload_buf[%d] = %02x", i, uds_rspns_isotp->payload_buf[i]);
                        }
                    //First Check if it is Tester Present
                    if((uds_rspns_isotp->payload_buf[0] == 0x7E)&&(uds_rspns_isotp->payload_buf[1] == 0x00)){
                        ESP_LOGI(UDS_TAG, "Tester Present: Received ISO-TP message");
                        response_pending = true;
                        continue;
                    }
                    
                    // Check if the response fits to the request
                    //First Check for the correct ID
                    if (uds_rspns_isotp->link.send_arbitration_id == uds_rqst_isotp.tx_id) {

                        //Then check for the correct payload
                        /////////////////NRC Handling////////////////////////
                        //Check if it is a negative response
                        if(uds_rspns_isotp->payload_buf[0] == 0x7F) {
                            //Then check if the subsequent bytes match the request
                            if(uds_rspns_isotp->payload_buf[1] == uds_rqst_isotp.buffer[0]){
                                //Then check if the NRC is "Response Pending", or it is the 
                                //If so, we have to wait to receive the actual response
                                if(uds_rspns_isotp->payload_buf[2] == 0x78){
                                    ESP_LOGI(UDS_TAG, "NRC: Response Pending. Waiting for actual response");
                                    //Jetzt zurück zu xQueueReceive
                                    response_pending = true;
                                    continue;
                                }
                                ESP_LOGI(UDS_TAG, "NRC: Received ISO-TP message matches request");
                                bytes_to_hex_string(uds_rspns_isotp, &uds_rqst_rspns_string);
                                ESP_LOGD(UDS_TAG, "uds_rqst_rspns_string.uds_request_string: %s", uds_rqst_rspns_string.uds_request_string);
                                ESP_LOGD(UDS_TAG, "uds_rqst_rspns_string.uds_response_string %s", uds_rqst_rspns_string.uds_response_string);
                                if (xQueueSend(handle_uds_response_queue, &uds_rqst_rspns_string, portMAX_DELAY) != pdPASS) {
                                    ESP_LOGE(UDS_TAG, "Failed to send message to handle_uds_response_queue");
                                } else {
                                    ESP_LOGE(UDS_TAG, "Negative Response: Message sent to handle_uds_response_queue");
                                }

                            }
                        //Secondly check if it is a positive response and if the subsequent bytes match the request
                        ///////////////////Positive Response Handling////////////////////////
                        }else if((uds_rspns_isotp->payload_buf[0] == (uds_rqst_isotp.buffer[0] | 0x40 )) && compare_buffers(&uds_rqst_isotp,uds_rspns_isotp)){
                            ESP_LOGI(UDS_TAG, "Positive Response: Received ISO-TP message matches request");
                            //End While Loop for NRC 0x78
                            response_pending = false;
                            bytes_to_hex_string(uds_rspns_isotp, &uds_rqst_rspns_string);
                            ESP_LOGD(UDS_TAG, "uds_rqst_rspns_string.uds_request_string: %s", uds_rqst_rspns_string.uds_request_string);
                            ESP_LOGD(UDS_TAG, "uds_rqst_rspns_string.uds_response_string %s", uds_rqst_rspns_string.uds_response_string);
                            if (xQueueSend(handle_uds_response_queue, &uds_rqst_rspns_string, portMAX_DELAY) != pdPASS) {
                                ESP_LOGE(UDS_TAG, "Failed to send message to handle_uds_response_queue");
                            } else {
                                ESP_LOGE(UDS_TAG, "Positive Response: Message sent to handle_uds_response_queue");
                            }
                            
                        }else{
                            ESP_LOGI(UDS_TAG, "Same ID, but different payload: Waiting for actual response");
                            //Jetzt zurück zu xQueueReceive
                            response_pending = true;
                            continue;
                        }
                    }else{
                        ESP_LOGI(UDS_TAG, "Different ID: Waiting for actual response");
                        //Jetzt zurück zu xQueueReceive
                        response_pending = true;
                        continue;
                    }        

                }
                //If nothing was received from the queue within EXAMPLE_P2_CLIENT, this code is being run
                //Meaning the ECU has not responded
                else{
                    //If the ECU has not responded, we have to create an artificial response to the webserver
                    //The artificial response always has:
                    // the ID (8 characters), the "7F" (2 characters), the SID of the response (2 characters) and the NRC FF (2 characters)
                    //The length is ALWAYS 14 characters
                    ESP_LOGE(UDS_TAG, "No response from ECU");
                    //End While Loop for NRC 0x78
                    response_pending = false;
                    uds_rqst_rspns_string.uds_response_length = 14;
                    uds_rqst_rspns_string.uds_response_string = malloc(14+1);
                    create_artificial_response (&uds_rqst_rspns_string);  
                    ESP_LOGD(UDS_TAG, "uds_rqst_rspns_string.uds_request_string: %s", uds_rqst_rspns_string.uds_request_string);
                    ESP_LOGD(UDS_TAG, "uds_rqst_rspns_string.uds_response_string %s", uds_rqst_rspns_string.uds_response_string);

                            //In the end of Queue Recieve handle_uds_request_queue If statement, send result to Queue handle_uds_response_queue
                    if (xQueueSend(handle_uds_response_queue, &uds_rqst_rspns_string, portMAX_DELAY) != pdPASS) {
                        ESP_LOGE(UDS_TAG, "Failed to send message to handle_uds_response_queue");
                    } else {
                        ESP_LOGE(UDS_TAG, "No Response: Message sent to handle_uds_response_queue");
                    }
                    
                }        

            }  
        }
        if (uds_rqst_isotp.buffer != NULL) {
            safe_free((void**)&uds_rqst_isotp.buffer);
        }                
    }
}


// Vergleichsfunktion
bool compare_buffers(const send_message_can_t *request, const IsoTpLinkContainer *response) {
    /////////////////////
    //Achtung: Sonderfall
    //Wenn der Service ReadDTCExtendedDataRecord (0x19 06) ist, dann wird das letzte Byte nicht verglichen
    //Das sollte eigentlich "DTCExtDataRecordNumber" des Requests beinhalten, tut es aber nicht immer aufgrund falscher Implementierung in der ECU
    // Iteriere durch die Bytes bis zur angegebenen Länge
    if(((request->buffer[0]==0x19)&&(request->buffer[1]==0x06))||((request->buffer[0]==0x19)&&(request->buffer[1]==0x02))){
        for (size_t i = 1; i < (request->msg_length-1); i++) {
            if (request->buffer[i] != response->payload_buf[i]) {
                return false; // Unterschied gefunden
            }
        }   
    }else{
        for (size_t i = 1; i < request->msg_length; i++) {
            if (request->buffer[i] != response->payload_buf[i]) {
                return false; // Unterschied gefunden
            }
        }
    }
    
    return true; // Alle verglichenen Bytes sind gleich
}