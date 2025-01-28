#include "initWifi.h"

/* STA Configuration */
//#define EXAMPLE_ESP_WIFI_STA_SSID           CONFIG_ESP_WIFI_REMOTE_AP_SSID
//#define EXAMPLE_ESP_WIFI_STA_PASSWD         CONFIG_ESP_WIFI_REMOTE_AP_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY           CONFIG_ESP_MAXIMUM_STA_RETRY

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WAPI_PSK
#endif

/* AP Configuration */
#define EXAMPLE_ESP_WIFI_AP_SSID            CONFIG_ESP_WIFI_AP_SSID
#define EXAMPLE_ESP_WIFI_AP_PASSWD          CONFIG_ESP_WIFI_AP_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL            CONFIG_ESP_WIFI_AP_CHANNEL
#define EXAMPLE_MAX_STA_CONN                CONFIG_ESP_MAX_STA_CONN_AP


/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG_STA = "initWifi.c: STA";
static const char *TAG_AP = "initWifi.c: AP";

char *SSID;
char *password;

static int s_retry_num = 0;

/* FreeRTOS event group to signal when we are connected/disconnected */
static EventGroupHandle_t s_wifi_event_group;

static void start_mdns_service(void)
{
    esp_err_t mdns_init_result;
    mdns_init_result=mdns_init();
    if (mdns_init_result != ESP_OK){
        ESP_LOGE(TAG_STA, "Failed to initialize mDNS service: %s", esp_err_to_name(mdns_init_result));
    }
    mdns_init_result=mdns_hostname_set("startrace");
    if(mdns_init_result != ESP_OK){
        ESP_LOGE(TAG_STA, "Failed to set mDNS hostname: %s", esp_err_to_name(mdns_init_result));
    }
    mdns_init_result=mdns_instance_name_set("startrace server");
    if(mdns_init_result != ESP_OK){
        ESP_LOGE(TAG_STA, "Failed to set mDNS instance name: %s", esp_err_to_name(mdns_init_result));
    }

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"path", "/"}
    };

    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
   
    
}


//Helper Function to print out the wifi event and perform other actions
static const char *wifi_event_check(int32_t event_id) {
    switch (event_id) {
        case WIFI_EVENT_WIFI_READY: return "WIFI_EVENT_WIFI_READY";
        case WIFI_EVENT_SCAN_DONE: return "WIFI_EVENT_SCAN_DONE";
        case WIFI_EVENT_STA_START: return "WIFI_EVENT_STA_START";
        case WIFI_EVENT_STA_STOP: return "WIFI_EVENT_STA_STOP";
        case WIFI_EVENT_STA_CONNECTED: return "WIFI_EVENT_STA_CONNECTED";
        case WIFI_EVENT_STA_DISCONNECTED: return "WIFI_EVENT_STA_DISCONNECTED";
        case WIFI_EVENT_STA_AUTHMODE_CHANGE: return "WIFI_EVENT_STA_AUTHMODE_CHANGE";
        case WIFI_EVENT_STA_WPS_ER_SUCCESS: return "WIFI_EVENT_STA_WPS_ER_SUCCESS";
        case WIFI_EVENT_STA_WPS_ER_FAILED: return "WIFI_EVENT_STA_WPS_ER_FAILED";
        case WIFI_EVENT_STA_WPS_ER_TIMEOUT: return "WIFI_EVENT_STA_WPS_ER_TIMEOUT";
        case WIFI_EVENT_STA_WPS_ER_PIN: return "WIFI_EVENT_STA_WPS_ER_PIN";
        case WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP: return "WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP";
        case WIFI_EVENT_AP_START: return "WIFI_EVENT_AP_START";
        case WIFI_EVENT_AP_STOP: return "WIFI_EVENT_AP_STOP";
        case WIFI_EVENT_AP_STACONNECTED:
            restart_webserver();
            return "WIFI_EVENT_AP_STACONNECTED";
        case WIFI_EVENT_AP_STADISCONNECTED:
            restart_webserver();
            return "WIFI_EVENT_AP_STADISCONNECTED";
        case WIFI_EVENT_AP_PROBEREQRECVED: return "WIFI_EVENT_AP_PROBEREQRECVED";
        case WIFI_EVENT_FTM_REPORT: return "WIFI_EVENT_FTM_REPORT";
        case WIFI_EVENT_STA_BSS_RSSI_LOW: return "WIFI_EVENT_STA_BSS_RSSI_LOW";
        case WIFI_EVENT_ACTION_TX_STATUS: return "WIFI_EVENT_ACTION_TX_STATUS";
        case WIFI_EVENT_ROC_DONE: return "WIFI_EVENT_ROC_DONE";
        case WIFI_EVENT_STA_BEACON_TIMEOUT: return "WIFI_EVENT_STA_BEACON_TIMEOUT";
        case WIFI_EVENT_CONNECTIONLESS_MODULE_WAKE_INTERVAL_START: return "WIFI_EVENT_CONNECTIONLESS_MODULE_WAKE_INTERVAL_START";
        case WIFI_EVENT_AP_WPS_RG_SUCCESS: return "WIFI_EVENT_AP_WPS_RG_SUCCESS";
        case WIFI_EVENT_AP_WPS_RG_FAILED: return "WIFI_EVENT_AP_WPS_RG_FAILED";
        case WIFI_EVENT_AP_WPS_RG_TIMEOUT: return "WIFI_EVENT_AP_WPS_RG_TIMEOUT";
        case WIFI_EVENT_AP_WPS_RG_PIN: return "WIFI_EVENT_AP_WPS_RG_PIN";
        case WIFI_EVENT_AP_WPS_RG_PBC_OVERLAP: return "WIFI_EVENT_AP_WPS_RG_PBC_OVERLAP";
        case WIFI_EVENT_ITWT_SETUP: return "WIFI_EVENT_ITWT_SETUP";
        case WIFI_EVENT_ITWT_TEARDOWN: return "WIFI_EVENT_ITWT_TEARDOWN";
        case WIFI_EVENT_ITWT_PROBE: return "WIFI_EVENT_ITWT_PROBE";
        case WIFI_EVENT_ITWT_SUSPEND: return "WIFI_EVENT_ITWT_SUSPEND";
        case WIFI_EVENT_TWT_WAKEUP: return "WIFI_EVENT_TWT_WAKEUP";
        case WIFI_EVENT_BTWT_SETUP: return "WIFI_EVENT_BTWT_SETUP";
        case WIFI_EVENT_BTWT_TEARDOWN: return "WIFI_EVENT_BTWT_TEARDOWN";
        case WIFI_EVENT_NAN_STARTED: return "WIFI_EVENT_NAN_STARTED";
        case WIFI_EVENT_NAN_STOPPED: return "WIFI_EVENT_NAN_STOPPED";
        case WIFI_EVENT_NAN_SVC_MATCH: return "WIFI_EVENT_NAN_SVC_MATCH";
        case WIFI_EVENT_NAN_REPLIED: return "WIFI_EVENT_NAN_REPLIED";
        case WIFI_EVENT_NAN_RECEIVE: return "WIFI_EVENT_NAN_RECEIVE";
        case WIFI_EVENT_NDP_INDICATION: return "WIFI_EVENT_NDP_INDICATION";
        case WIFI_EVENT_NDP_CONFIRM: return "WIFI_EVENT_NDP_CONFIRM";
        case WIFI_EVENT_NDP_TERMINATED: return "WIFI_EVENT_NDP_TERMINATED";
        case WIFI_EVENT_HOME_CHANNEL_CHANGE: return "WIFI_EVENT_HOME_CHANNEL_CHANGE";
        case WIFI_EVENT_STA_NEIGHBOR_REP: return "WIFI_EVENT_STA_NEIGHBOR_REP";
        case WIFI_EVENT_MAX: return "WIFI_EVENT_MAX";
        default: return "UNKNOWN_EVENT_ID";
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    const char *event_name = wifi_event_check(event_id);
    ESP_LOGD("WiFi Event Handler", "Event ID: %d (%s)", (unsigned int) event_id, event_name);

    led_actuation_order.LED_color=DEFAULT;
    led_actuation_order.breaktime=100;
    xQueueSend(handle_led_actuation_queue, &led_actuation_order, portMAX_DELAY);
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGD(TAG_AP, "Station "MACSTR" joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGD(TAG_AP, "Station "MACSTR" left, AID=%d, reason:%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG_STA, "Station started");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *) event_data;
        ESP_LOGI(TAG_STA, "Station disconnected, reason:%d", event->reason);
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG_STA, "Retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG_STA, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


/* Initialize soft AP */
esp_netif_t *wifi_init_softap(void)
{
    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_AP_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_AP_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_AP_PASSWD,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };

    if (strlen(EXAMPLE_ESP_WIFI_AP_PASSWD) == 0) {
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));

    ESP_LOGD(TAG_AP, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_AP_SSID, EXAMPLE_ESP_WIFI_AP_PASSWD, EXAMPLE_ESP_WIFI_CHANNEL);

    return esp_netif_ap;
}

/* Initialize wifi station */
esp_netif_t *wifi_init_sta(void)
{
    SSID=getConfigData("SSID", "WIFI");
    password=getConfigData("password", "WIFI");

    // Ensure SSID and password are not null
    if (SSID == NULL || password == NULL) {
        ESP_LOGE(TAG_STA, "SSID or password is null");
        return NULL;
    }
    ESP_LOGI(TAG_STA,"SSID from Config.json: %s", SSID);
    ESP_LOGI(TAG_STA,"SSID from Config.json: %s", password);

    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_sta_config = {
        .sta = {
            // Initialize SSID and password correctly
            .ssid = "",
            .password = "",
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    // Copy SSID and password to the config structure
    strncpy((char *)wifi_sta_config.sta.ssid, SSID, sizeof(wifi_sta_config.sta.ssid) - 1);
    strncpy((char *)wifi_sta_config.sta.password, password, sizeof(wifi_sta_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config) );

    ESP_LOGD(TAG_STA, "wifi_init_sta finished.");



    return esp_netif_sta;
}

// static void wifi_timeout_handler(void *arg)
// {
//     xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
// }


void InitWifi(void){
    led_actuation_order.LED_color=DEFAULT;
    led_actuation_order.breaktime=50;
    xQueueSend(handle_led_actuation_queue, &led_actuation_order, portMAX_DELAY);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Initialize event group */
    s_wifi_event_group = xEventGroupCreate();

    /* Register Event handler */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    &wifi_event_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                    IP_EVENT_STA_GOT_IP,
                    &wifi_event_handler,
                    NULL,
                    NULL));

    /*Initialize WiFi */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    /* Initialize AP */
    ESP_LOGD(TAG_AP, "ESP_WIFI_MODE_AP");
    esp_netif_t *esp_netif_ap = wifi_init_softap();

    /* Initialize STA */
    ESP_LOGD(TAG_STA, "ESP_WIFI_MODE_STA");
    esp_netif_t *esp_netif_sta = wifi_init_sta();

    /* Start WiFi */
    ESP_ERROR_CHECK(esp_wifi_start() );

    //     /* Start the timeout timer */
    // const esp_timer_create_args_t timer_args = {
    //     .callback = &wifi_timeout_handler,
    //     .name = "wifi_timeout"
    // };
    // esp_timer_handle_t timer;
    // ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    // ESP_ERROR_CHECK(esp_timer_start_once(timer, 5000000)); // 30 seconds timeout

    /*
     * Wait until either the connection is established (WIFI_CONNECTED_BIT) or
     * connection failed for the maximum number of re-tries (WIFI_FAIL_BIT).
     * The bits are set by event_handler() (see above)
     */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned,
     * hence we can test which event actually happened. */
    led_actuation_order.LED_color=DEFAULT;
    led_actuation_order.breaktime=100;
    xQueueSend(handle_led_actuation_queue, &led_actuation_order, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_STA, "connected to ap SSID:%s password:%s",
                 SSID, password);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_STA, "Failed to connect to SSID:%s, password:%s",
                 SSID, password);
    } else {
        ESP_LOGE(TAG_STA, "UNEXPECTED EVENT");
        return;
    }

    /* Set sta as the default interface */
    esp_netif_set_default_netif(esp_netif_sta);

    /* Enable napt on the AP netif */
    if (esp_netif_napt_enable(esp_netif_sta) != ESP_OK) {
        ESP_LOGE(TAG_STA, "NAPT not enabled on the netif: %p", esp_netif_ap);
    }

    start_mdns_service();
        // Free SSID and password after use
    free(SSID);
    free(password);
}
