#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/twai.h"
#include "esp_log.h"
#include "isotp.h"
#include "mutexes.h"
#include "Messages.h"
#include "queues.h"
#include "isotp_link_containers.h"


static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
extern led_actuation_t led_actuation_order;

void twai_receive_task(void *arg);
void twai_transmit_task(void *arg);