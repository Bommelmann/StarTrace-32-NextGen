#include "freertos/queue.h"

extern QueueHandle_t websocket_send_queue;
extern QueueHandle_t tx_task_queue;
extern QueueHandle_t isotp_send_message_queue;
extern QueueHandle_t handle_uds_request_queue;
extern QueueHandle_t handle_uds_request_queue_container;
extern QueueHandle_t handle_led_actuation_queue;
