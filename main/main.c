

/*
 * SPDX-FileCopyrightText: 2010-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

/*
 * The following example demonstrates a master node in a TWAI network. The master
 * node is responsible for initiating and stopping the transfer of data messages.
 * The example will execute multiple iterations, with each iteration the master
 * node will do the following:
 * 1) Start the TWAI driver
 * 2) Repeatedly send ping messages until a ping response from slave is received
 * 3) Send start command to slave and receive data messages from slave
 * 4) Send stop command to slave and wait for stop response from slave
 * 5) Stop the TWAI driver
 */

/*
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"

 --------------------- Definitions and static variables ------------------ */
//Example Configuration
/*
#define PING_PERIOD_MS          1000
#define NO_OF_DATA_MSGS         50
#define NO_OF_ITERS             3
#define ITER_DELAY_MS           1000
#define RX_TASK_PRIO            8
#define TX_TASK_PRIO            9
#define CTRL_TSK_PRIO           10
#define TX_GPIO_NUM             CONFIG_EXAMPLE_TX_GPIO_NUM
#define RX_GPIO_NUM             CONFIG_EXAMPLE_RX_GPIO_NUM
#define EXAMPLE_TAG             "DIAG Client"

#define DIAG_REQ_ID             0x18DA00F1
#define DIAG_RESP_ID            0x18DAF100


static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);

static const twai_message_t diag_request = {
    // Message type and format settings
    .extd = 1,              // Standard Format message (11-bit ID)
    .rtr = 0,               // Send a data frame
    .ss = 0,                // Is single shot (won't retry on error or NACK)
    .self = 0,              // Not a self reception request
    .dlc_non_comp = 0,      // DLC is less than 8
    // Message ID and payload
    .identifier = DIAG_REQ_ID,
    .data_length_code = 8,
    .data = {0x03, 0x22, 0xF1, 0x00, 0xFF, 0xFF, 0xFF, 0xFF},
};


int k=0;
bool endflag=false;

static SemaphoreHandle_t done_sem;
--------------------------- Tasks and Functions -------------------------- */
/*
static void twai_receive_task(void *arg)
{
    while (1) {

                ESP_LOGI(EXAMPLE_TAG, "In receive task");
                twai_message_t rx_msg;
                twai_receive(&rx_msg, portMAX_DELAY);
                if (rx_msg.identifier == DIAG_RESP_ID) {
                    ESP_LOGI(EXAMPLE_TAG, "Response Received!");
                    k++;
                    if (k>1000){
                        xSemaphoreGive(done_sem);
                        endflag=true;
                        vTaskDelete(NULL);
                        }
                }
            }
    
    vTaskDelete(NULL);
}

static void twai_transmit_task(void *arg)
{
    while (1) {

            ESP_LOGI(EXAMPLE_TAG, "In Transmit Task");
            twai_transmit(&diag_request, portMAX_DELAY);
                vTaskDelay(pdMS_TO_TICKS(PING_PERIOD_MS));
                if(endflag==true){
                    break;
                                   }
    }
    vTaskDelete(NULL);
}


void app_main(void)
{
     //Install TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(EXAMPLE_TAG, "Driver installed");
    //Create tasks
    xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
    done_sem = xSemaphoreCreateBinary();


   

    xSemaphoreTake(done_sem, portMAX_DELAY);    //Wait for completion

    //Uninstall TWAI driver
    ESP_ERROR_CHECK(twai_driver_uninstall());
    ESP_LOGI(EXAMPLE_TAG, "Driver uninstalled");

}

*/


/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

/*
 * The following example demonstrates a master node in a TWAI network. The master
 * node is responsible for initiating and stopping the transfer of data messages.
 * The example will execute multiple iterations, with each iteration the master
 * node will do the following:
 * 1) Start the TWAI driver
 * 2) Repeatedly send ping messages until a ping response from slave is received
 * 3) Send start command to slave and receive data messages from slave
 * 4) Send stop command to slave and wait for stop response from slave
 * 5) Stop the TWAI driver
 */


////////////////////////////////////////////
//Working Example before 19.09.2024
/* 
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"
#include "initWifi.h"
#include "createWebserver.h"
#include "InitializeFilesystem.h"
#include "InitializeSDCard.h"


--------------------- Definitions and static variables ------------------
//Example Configuration
#define PING_PERIOD_MS          1000
#define NO_OF_DATA_MSGS         50
#define NO_OF_ITERS             3
#define ITER_DELAY_MS           1000
#define RX_TASK_PRIO            8
#define TX_TASK_PRIO            9
#define LED_TASK_PRIO           7
#define CTRL_TSK_PRIO           10
#define TX_GPIO_NUM             CONFIG_EXAMPLE_TX_GPIO_NUM
#define RX_GPIO_NUM             CONFIG_EXAMPLE_RX_GPIO_NUM
#define EXAMPLE_TAGRX            "RX Task"
#define EXAMPLE_TAGTX            "TX Task"
#define EXAMPLE_TAGMain           "App_Main"
#define CTRL_TASK_TAG           "Ctrl_Task"


#define ID_Diag_Resp  0x18DAF100

bool n=false;

static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);

//
static const twai_message_t Request_Message = {.identifier = 0x18DA00F1 , .data_length_code = 8,
                                           .ss = 1, .extd = 1, .data = {0x03, 0x22 , 0xF1 , 0x00 ,0xFF ,0xFF ,0xFF ,0xFF}};
//static const twai_message_t Random_Message = {.identifier = 0x18F00100 , .data_length_code = 8,
 //                                          .ss = 1, .extd = 1, .data = {0xFF, 0xFF , 0xFF , 0xCF ,0xFA ,0xFF ,0xFF ,0xFF}};

int k=0;
bool endflag=false;

static SemaphoreHandle_t done_sem;

uint8_t Diag_Resp [8];

 //--------------------------- Tasks and Functions -------------------------- 

static void twai_receive_task(void *arg)
{
    while (1) {
        //ESP_LOGI(EXAMPLE_TAGRX, "In Rx Task" );          
        twai_message_t rx_msg;
        if (twai_receive(&rx_msg, portMAX_DELAY) == ESP_OK) {
            if (rx_msg.identifier == ID_Diag_Resp) {
                //ESP_LOGI(EXAMPLE_TAGRX, "Response Received" );
                *Diag_Resp=rx_msg.data;
                k++;
                vTaskDelay(pdMS_TO_TICKS(PING_PERIOD_MS)); // Add delay to give other tasks a chance to run
                if (k > 1000) {
                    xSemaphoreGive(done_sem);
                    endflag = true;
                    break;
                }
            } 
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to allow other tasks to run
    }
    
    //ESP_LOGI(EXAMPLE_TAGRX, "Rx Task Deleted");
    vTaskDelete(NULL);
}


static void twai_transmit_task(void *arg)
{
    while (1) {

                    //ESP_LOGI(EXAMPLE_TAGTX, "In Tx Task" );   
                    if(twai_transmit(&Request_Message, portMAX_DELAY)==ESP_OK){
                    //ESP_LOGI(EXAMPLE_TAGTX, "Request Transmitted");
                    vTaskDelay(pdMS_TO_TICKS(PING_PERIOD_MS));
                    if(endflag==true){
                    break;
                                   }
                    }
      //ESP_LOGI(EXAMPLE_TAGTX, "End Of Tx While Loop");                    
    }
    //ESP_LOGI(EXAMPLE_TAGTX, "Tx Task Deleted");
    vTaskDelete(NULL);

}

static void LED_driver_task(void *arg){

    while (1){
        vTaskDelay(pdMS_TO_TICKS(PING_PERIOD_MS));
        
    }


    vTaskDelete(NULL);
}

//App_main content

//Create tasks, queues, and semaphores
done_sem = xSemaphoreCreateBinary();

xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
xTaskCreatePinnedToCore(LED_driver_task, "LED_drvr", 4096, NULL, LED_TASK_PRIO, NULL, tskNO_AFFINITY);
ESP_LOGI(EXAMPLE_TAGMain, "App_Main Before Semaphore Take");   
xSemaphoreTake(done_sem, portMAX_DELAY);    //Wait for completion
vSemaphoreDelete(done_sem);

*/

#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"
#include "initWifi.h"
#include "createWebserver.h"
#include "InitializeFilesystem.h"
#include "InitializeSDCard.h"
#include "HandleUDSRequest.h"

//ISOTP Stuff
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

#define EXAMPLE_TAGRX            "RX Task"
#define EXAMPLE_TAGTX            "TX Task"
#define EXAMPLE_TAGMain          "App_Main"
#define CTRL_TASK_TAG           "Ctrl_Task"
#define MAIN_TAG                   "main"



SemaphoreHandle_t done_sem;
SemaphoreHandle_t isotp_send_queue_sem;
SemaphoreHandle_t isotp_mutex;

QueueHandle_t websocket_send_queue;
QueueHandle_t tx_task_queue;
QueueHandle_t isotp_send_message_queue;

uint8_t Diag_Resp [8];

void app_main(void)
{

    esp_log_level_set("*", ESP_LOG_INFO);

    //INIT CAN #######################################
    //#################################################
    //Install TWAI driver
    //while((twai_driver_install(&g_config, &t_config, &f_config)) != ESP_OK){}
    //ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    //ESP_LOGI(EXAMPLE_TAGMain, "TWAI Driver installed");

    //while (twai_start()!= ESP_OK){}
    //ESP_ERROR_CHECK(twai_start());
    //ESP_LOGI(EXAMPLE_TAGMain, "TWAI Driver started");
    


    //INIT WIFI #######################################
    //#################################################
    //Initialize NVS
    InitializeFilesystem();
    initializeSDCard();
    InitWifi();
    start_webserver();

     //Create semaphores and tasks
    tx_task_queue = xQueueCreate(128, sizeof(twai_message_t));
    //websocket_send_queue = xQueueCreate(128, sizeof(send_message_t));
    isotp_send_message_queue = xQueueCreate(128, sizeof(send_message_t));
    done_sem = xSemaphoreCreateBinary();
    isotp_send_queue_sem = xSemaphoreCreateBinary();
    isotp_mutex = xSemaphoreCreateMutex();

    // "TWAI" is knockoff CAN. Install TWAI driver.
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(MAIN_TAG, "CAN/TWAI Driver installed");
        
    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(MAIN_TAG, "CAN/TWAI Driver started");
    // ISO-TP handler + tasks
    configure_isotp_links();
    ESP_LOGI(MAIN_TAG, "ISO-TP links configured");
    xSemaphoreGive(isotp_send_queue_sem);
    // Tasks :
    // "websocket_sendTask" polls the websocket_send_queue queue. This queue is populated when a ISO-TP PDU is received.
    // "TWAI_rx" polls the receive queue (blocking) and once a message exists, forwards it into the ISO-TP library.
    // "TWAI_tx" blocks on a send queue which is populated by the callback from the ISO-TP library
    // "ISOTP_process" pumps the ISOTP library's "poll" method, which will call the send queue callback if a message needs to be sent.
    // ISOTP_process also polls the ISOTP library's non-blocking receive method, which will produce a message if one is ready.
    // "MAIN_process_send_queue" processes queued messages from the BLE stack. These messages are dynamically allocated when they are queued and freed in this task.
    //xTaskCreatePinnedToCore(websocket_send_task, "websocket_sendTask", 4096, NULL, SOCKET_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(isotp_send_queue_task, "ISOTP_process_send_queue", 4096, NULL, MAIN_TSK_PRIO, NULL, tskNO_AFFINITY);
    ESP_LOGI(MAIN_TAG, "Tasks started");
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
    //ESP_ERROR_CHECK(twai_driver_uninstall());
    //ESP_LOGI(EXAMPLE_TAGMain, "Driver uninstalled");

       
}
