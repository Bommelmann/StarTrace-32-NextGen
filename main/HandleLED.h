#include "driver/gpio.h"
#include "messages.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "queues.h"

#define TAG "HandleLED.c"

void handleLED_task();
void configureLEDs();