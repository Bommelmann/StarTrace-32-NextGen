#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "messages.h"
#include "queues.h"
#include "createWebserver.h"
#include "mdns.h"
#include "HandleFileReading.h"

extern led_actuation_t led_actuation_order;

esp_netif_t *wifi_init_softap(void);
esp_netif_t *wifi_init_sta(void);

void InitWifi(void);

extern char ipv4_address[16];