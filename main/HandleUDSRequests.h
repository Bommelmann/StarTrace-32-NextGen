#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include "esp_log.h"
#include "messages.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "isotp_link_containers.h"
#include "freertos/semphr.h"
#include "mutexes.h"

//Own includes
#include "functions.h"


extern QueueHandle_t isotp_send_message_queue;
extern QueueHandle_t handle_uds_request_queue;
extern QueueHandle_t handle_uds_request_queue_container;

#define UDS_TAG       "UDS_handling"


void handle_uds_request_task();