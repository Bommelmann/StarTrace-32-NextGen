#ifndef CREATEWEBSERVER_H
#define CREATEWEBSERVER_H

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "cJSON.h"

#include "esp_err.h"
#include "esp_log.h"

#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"
#include "messages.h"
#include "queues.h"
#include "HandleFileReading.h"
#include "mdns.h"

extern led_actuation_t led_actuation_order;


esp_err_t hello_get_handler(httpd_req_t *req);

esp_err_t start_webserver(void);

char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize);

esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename);

esp_err_t http_resp_dir_html(httpd_req_t *req, const char *dirpath);

void remove_question_mark(char *str);

esp_err_t on_client_disconnect(httpd_handle_t server, int sock_fd);

esp_err_t restart_webserver(void);



struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};

#endif // CREATEWEBSERVER_H

extern httpd_handle_t server;
extern QueueHandle_t handle_uds_request_queue;
extern QueueHandle_t handle_uds_response_queue;
extern char ipv4_address[16];