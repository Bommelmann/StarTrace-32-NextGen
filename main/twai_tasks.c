#include "twai_tasks.h"

#define TWAI_TAG "twai"

void twai_receive_task(void *arg)
{
    //Actuate LED ###################################
    //#################################################    
    led_actuation_order.LED_color=DEFAULT;
    led_actuation_order.breaktime=50;
    xQueueSend(handle_led_actuation_queue, &led_actuation_order, portMAX_DELAY);
    while (1)
    {
        twai_message_t twai_rx_msg;
        twai_receive(&twai_rx_msg, portMAX_DELAY); // If no message available, should block and yield.
        /*
        ESP_LOGD(TWAI_TAG, "Received TWAI message with identifier %08X and length %08X", (unsigned int)twai_rx_msg.identifier, (unsigned int)twai_rx_msg.data_length_code);
        if (esp_log_level_get("*") == ESP_LOG_DEBUG) {
            for (int i = 0; i < twai_rx_msg.data_length_code; i++) {
                ESP_LOGD(TWAI_TAG, "RX Data: %02X", twai_rx_msg.data[i]);
            }
        }
        */
        
        // short circuit on low-level traffic
        if (twai_rx_msg.identifier == 0x18DAF100||twai_rx_msg.identifier == 0x18DAF101||twai_rx_msg.identifier == 0x18DAF103||twai_rx_msg.identifier == 0x18DAF13D) {

            int isotp_link_container_index = find_isotp_link_container_index_by_receive_arbitration_id(twai_rx_msg.identifier);
            if (isotp_link_container_index != -1) {
                IsoTpLinkContainer *isotp_link_container = &isotp_link_containers[isotp_link_container_index];
                ESP_LOGD(TWAI_TAG, "Taking isotp_mutex");
                xSemaphoreTake(isotp_mutex, (TickType_t)100);
                ESP_LOGD(TWAI_TAG, "Took isotp_mutex");
                isotp_on_can_message(&isotp_link_container->link, twai_rx_msg.data, twai_rx_msg.data_length_code);
                ESP_LOGD(TWAI_TAG, "twai_receive_task: giving isotp_mutex");
                xSemaphoreGive(isotp_mutex);
                ESP_LOGD(TWAI_TAG, "twai_receive_task: giving wait_for_isotp_data_sem");
                xSemaphoreGive(isotp_link_container->wait_for_isotp_data_sem);
            }
        }
    }
    vTaskDelete(NULL);
}

void twai_transmit_task(void *arg)
{
    //Actuate LED ###################################
    //#################################################
    led_actuation_order.LED_color=DEFAULT;
    led_actuation_order.breaktime=50;
    xQueueSend(handle_led_actuation_queue, &led_actuation_order, portMAX_DELAY);
    while (1)
    {
        twai_message_t tx_msg;
        xQueueReceive(tx_task_queue, &tx_msg, portMAX_DELAY);
        //ESP_LOGD(TWAI_TAG, "Sending TWAI Message with ID %08X", (unsigned int)tx_msg.identifier);
        if (esp_log_level_get("*") == ESP_LOG_DEBUG) {
            for (int i = 0; i < tx_msg.data_length_code; i++) {
                ESP_LOGD(TWAI_TAG, "TX Data: %02X", tx_msg.data[i]);
            }
        }
        twai_transmit(&tx_msg, portMAX_DELAY);
        //isotp_user_debug("Twai Transmit ID: 0x%08X\n", tx_msg.identifier);
        ESP_LOGD(TWAI_TAG, "Sent TWAI Message with ID %08X", (unsigned int)tx_msg.identifier);
    }
    vTaskDelete(NULL);
}
