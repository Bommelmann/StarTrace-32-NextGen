

#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"


#include "initWifi.h"
#include "createWebserver.h"
#include "InitializeFilesystem.h"
#include "InitializeSDCard.h"
#include "twai_tasks.h"



#include "messages.h"
#include "queues.h"
#include "endian_helpers.h"
#include "isotp_callbacks.h"
#include "isotp_link_containers.h"
#include "isotp_links.h"
#include "isotp_tasks.h"
#include "task_priorities.h"
#include "periodic_messages.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "mutexes.h"

#include "HandleUDSRequests.h"


#define MAIN_TAG      "app_main"


SemaphoreHandle_t done_sem;
SemaphoreHandle_t isotp_send_queue_sem;
SemaphoreHandle_t isotp_mutex;
SemaphoreHandle_t cts_sem;

QueueHandle_t websocket_send_queue;
QueueHandle_t tx_task_queue;
QueueHandle_t isotp_send_message_queue;
QueueHandle_t handle_uds_request_queue;
QueueHandle_t handle_uds_request_queue_container;

uint8_t Diag_Resp [8];

void app_main(void)
{

    //Set Log Level ###################################
    //#################################################
    esp_log_level_set("*", ESP_LOG_INFO);  


    //INIT WIFI #######################################
    //#################################################
    //Initialize NVS
    InitializeFilesystem();
    initializeSDCard();
    InitWifi();
    start_webserver();

    //Create Queues ###################################
    //#################################################
    //tx_task_queue is for the transmission of:
        //items of the data type twai_message_t
        // between the tasks "isotp_send_queue_task" and "twai_transmit_task" 
     tx_task_queue = xQueueCreate(128, sizeof(twai_message_t));
    //isotp_send_message_queue is for the transmission of:
        //items of the data type send_message_can_t
        // between the tasks "handle_uds_request_task" and "isotp_send_queue_task"
    isotp_send_message_queue = xQueueCreate(128, sizeof(send_message_can_t));
    //handle_uds_request_queue is for the transmission of:
        //items of the data type uds_message_string_t
        // between the tasks "XY" and "handle_uds_request_task"
    handle_uds_request_queue = xQueueCreate(128, sizeof(uds_message_string_t));
    //handle_uds_request_queue_container is for the transmission of:
        //items of the data type IsoTpLinkContainer
        // between the tasks "isotp_processing_task" and "handle_uds_request_task"
    handle_uds_request_queue_container = xQueueCreate(10, sizeof(IsoTpLinkContainer*));
    //websocket_send_queue = xQueueCreate(128, sizeof(send_message_can_t));
    

    //Create Semaphores ###################################
    //#################################################
    //done_sem can be used to terminate the app_main() task, which then deletes all semaphores and queues
    done_sem = xSemaphoreCreateBinary();
    //isotp_send_queue_sem is used to block the isotp_send_queue_task, until the twai drivers are installed
    isotp_send_queue_sem = xSemaphoreCreateBinary();
    //Used to take turns on access on IsoTpLinkContainer
    isotp_mutex = xSemaphoreCreateMutex();
    //TODO: Used to signal the clear to send from received flow control message, from task "twai_receive_task" to "isotp_send_queue_task"
    cts_sem = xSemaphoreCreateBinary();

    //Init TWAI ###################################
    //#################################################
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(MAIN_TAG, "CAN/TWAI Driver installed");
    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(MAIN_TAG, "CAN/TWAI Driver started");
    // ISO-TP handler + tasks
    configure_isotp_links();
    ESP_LOGI(MAIN_TAG, "ISO-TP links configured");
    xSemaphoreGive(isotp_send_queue_sem);


    //Create Tasks ###################################
    //#################################################
    // "websocket_sendTask" polls the websocket_send_queue queue. This queue is populated when a ISO-TP PDU is received.

    //xTaskCreatePinnedToCore(websocket_send_task, "websocket_sendTask", 4096, NULL, SOCKET_TASK_PRIO, NULL, tskNO_AFFINITY);
    // "TWAI_rx" polls the receive queue (blocking) and once a message exists, forwards it into the ISO-TP library.
    xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
    // "TWAI_tx" blocks on a send queue which is populated by the callback from the ISO-TP library
    xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
    // "isotp_send_queue_task" pumps the ISOTP library's "poll" method, which will call the send queue callback if a message needs to be sent.
    // "isotp_send_queue_task" also polls the ISOTP library's non-blocking receive method, which will produce a message if one is ready.
    xTaskCreatePinnedToCore(isotp_send_queue_task, "ISOTP_process_send_queue", 4096, NULL, MAIN_TSK_PRIO, NULL, tskNO_AFFINITY);
    //"handle_uds_request_tasks" takes orders from any task which fills the queue "handle_uds_request_queue"
    //It automatically sends the diagnostic request over CAN, waits for the response and sends this back
    xTaskCreatePinnedToCore(handle_uds_request_task, "UDS_handling", 4096, NULL, MAIN_TSK_PRIO, NULL, tskNO_AFFINITY);
    ESP_LOGI(MAIN_TAG, "Tasks started");

    
    //Test ############################################
    //#################################################
    //Delay Task then fill Queue with messages
    while (1){
        vTaskDelay(2000 / portTICK_PERIOD_MS);

            //Test
            uds_message_string_t uds_message_string_queue_send;
            uds_message_string_queue_send.msg_length = 14;
            uds_message_string_queue_send.uds_string = malloc(15);
            strcpy(uds_message_string_queue_send.uds_string, "18DA00F122F100");
            //Test: Send in uds queue
           
           xQueueSend(handle_uds_request_queue, &uds_message_string_queue_send, portMAX_DELAY);
    }

    // lock done_sem
    xSemaphoreTake(done_sem, portMAX_DELAY);
    // uninstall driver
    ESP_ERROR_CHECK(twai_driver_uninstall());
    ESP_LOGI(MAIN_TAG, "Driver uninstalled");
    // cleanup
    vSemaphoreDelete(isotp_send_queue_sem);
    vSemaphoreDelete(done_sem);
    vSemaphoreDelete(isotp_mutex);
    vQueueDelete(tx_task_queue);
    vQueueDelete(isotp_send_message_queue);
    //Uninstall TWAI driver
    ESP_ERROR_CHECK(twai_driver_uninstall());
    ESP_LOGI(MAIN_TAG, "Driver uninstalled");

       
}
