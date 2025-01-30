#include "createWebserver.h"

static const char *TAG = "createwebserver.c";
/* Initialize file storage */
char* base_path_flash="/data";
char* base_path_sd="/sdcard";

//boolean to prevent endless loops when restarting the server:
static bool webserver_restarted=false;
//counter to prevent, that index.html is sent twice
static uint8_t index_uri_counter=0;

esp_err_t IPAddress_handler(httpd_req_t *req)
{
    extern char ipv4_address[16];
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, ipv4_address);
    return ESP_OK;
}

esp_err_t uds_request_handler(httpd_req_t *req)
{
    ESP_LOGE(TAG,"req->uri: %s", req->uri);  
    // Länge des Bodys ermitteln
    size_t buf_len = req->content_len;
    if (buf_len == 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No JSON body in Request in UDS Request Handler");
        return ESP_FAIL;
    }

    // Speicher für den Body reservieren (+1 für Nullterminator)
    char *buf = malloc(buf_len + 1);
    if (!buf) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed in UDS Request Handler");
        return ESP_FAIL;
    }

    // Body aus der Anfrage lesen
    int ret = httpd_req_recv(req, buf, buf_len);
    if (ret <= 0) {
        free(buf);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to read request body in UDS Request Handler");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    ESP_LOGD(TAG, "JSON-Daten: %s", buf);
    // Nullterminator hinzufügen, um den Body als String zu behandeln
    

    // JSON parsen
    cJSON *uds_rqst_rspns_json = cJSON_Parse(buf);
    free(buf); // Speicher des Buffers freigeben, da er nicht mehr benötigt wird
    if (!uds_rqst_rspns_json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON format in UDS Request Handler");
        return ESP_FAIL;
    }

    
    cJSON *uds_rqst_json = cJSON_GetObjectItem(uds_rqst_rspns_json, "request");
    if (!cJSON_IsString(uds_rqst_json)){
        cJSON_Delete(uds_rqst_json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or invalid 'Request' field in UDS Request Handler");
        return ESP_FAIL;
    }

    // Werte aus dem JSON extrahieren
    uds_message_string_t uds_rqst_rspns_string;
    uds_rqst_rspns_string.uds_request_length = strlen(uds_rqst_json->valuestring);
    uds_rqst_rspns_string.uds_request_string = malloc(uds_rqst_rspns_string.uds_request_length+1);
    strcpy(uds_rqst_rspns_string.uds_request_string,uds_rqst_json->valuestring);
    ESP_LOGD(TAG, "RequestData in uds_message_string_t: %s", uds_rqst_rspns_string.uds_request_string);
    ESP_LOGD(TAG, "RequestLength in uds_message_string_t: %d", (int)uds_rqst_rspns_string.uds_request_length);
    ESP_LOGE(TAG, "Sending to handle_uds_request_queue");
    xQueueSend(handle_uds_request_queue, &uds_rqst_rspns_string, portMAX_DELAY);
    xQueueReceive(handle_uds_response_queue, &uds_rqst_rspns_string, portMAX_DELAY);
    ESP_LOGE(TAG, "Received from handle_uds_response_queue");
    ESP_LOGD(TAG, "ResponseData in uds_message_string_t: %s", uds_rqst_rspns_string.uds_response_string);
    ESP_LOGD(TAG, "ResponseLength in uds_message_string_t: %d", (int)uds_rqst_rspns_string.uds_response_length);

    cJSON *uds_rspns_json = cJSON_GetObjectItem(uds_rqst_rspns_json, "response");
    cJSON_SetValuestring(uds_rspns_json,uds_rqst_rspns_string.uds_response_string);
    ESP_LOGD(TAG, "uds_rspns_json->valuestring: %s", uds_rspns_json->valuestring);
    // Das JSON-Objekt in einen String umwandeln
    char *json_str = cJSON_Print(uds_rqst_rspns_json);
    ESP_LOGI(TAG, "json_str: %s", json_str);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json_str);

    // Objekte freigeben
    free(json_str);
    cJSON_Delete(uds_rqst_rspns_json);
    free(uds_rqst_rspns_string.uds_request_string);
    free(uds_rqst_rspns_string.uds_response_string);

    return ESP_OK;
}



esp_err_t start_download_handler(httpd_req_t *req)
{
    char filepathlfs[FILE_PATH_MAX_LFS];
    char filepathfatfs[FILE_PATH_MAX_FATFS];
    
    struct stat file_stat;
    //char* ECUName;      // Linker Teil
    //char* DiagVersion;
    //char *ECUName_DiagVersion;

    ESP_LOGI(TAG,"req->uri: %s", req->uri);
    if (strcmp(req->uri, "/home.html") == 0){
        index_uri_counter=0;
    }      
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //Case that this is the first request for index.html:
    if (strcmp(req->uri, "/") == 0)
    {   
        //In order to prevent the server to send index.html twice (for unkknown reasons) the counter is used
        if((index_uri_counter==0)||(index_uri_counter==2)){
            //Index.html is stored in the basepath /data
            char *filename_index = get_path_from_uri(filepathlfs, ((struct file_server_data *)req->user_ctx)->base_path_flash,
                                                            "/index.html", sizeof(filepathlfs));

            //Check if Filename has the correct format and if the file is existing
            esp_err_t ret;
            ret=checkonFilename(filename_index, req, &file_stat, filepathlfs);
            //Only if it is existing and has the right format, it shall be sent
            if(ret==ESP_OK){
              //ReadFiles and send over HTTP
              readsendFile(filename_index, filepathlfs, req, &file_stat);
            }else{
            ESP_LOGE(TAG,"CheckonFilename returned: %d", ret);
            }

       }
        if(index_uri_counter==3){
            index_uri_counter=0;
        }else{
            index_uri_counter++;
        }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////                                                   
    //Case that the client requests a DiagDescription, the filename has to be deducted otherwise                                                        
    }else if(strstr(req->uri, "sdcard?/") != NULL){


                char *filename;
                filename=malloc(FILE_PATH_MAX_FATFS);
                if (filename == NULL) {
                ESP_LOGE(TAG, "Fehler beim Zuweisen von Speicher für filename");
                return ESP_FAIL;
                }

                strcpy(filepathfatfs,req->uri);
                remove_question_mark(filepathfatfs);
                strcpy (filename,filepathfatfs);
                ESP_LOGI(TAG,"DiagDataFilepath: %s", filepathfatfs);
                ESP_LOGI(TAG,"Filename: %s", filename);

            /*
            //DiagDescriptions are stored on the sd card
            // Länge des Bodys ermitteln
            size_t buf_len = req->content_len;
            if (buf_len == 0) {
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No JSON body in Request");
                free (filename);
                return ESP_FAIL;
            }

            // Speicher für den Body reservieren (+1 für Nullterminator)
            char *buf = malloc(buf_len + 1);
            if (!buf) {
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
                free (filename);
                return ESP_FAIL;
            }

            // Body aus der Anfrage lesen
            int ret = httpd_req_recv(req, buf, buf_len);
            if (ret <= 0) {
                free(buf);
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to read request body");
                free (filename);
                return ESP_FAIL;
            }
            buf[ret] = '\0';

            ESP_LOGI(TAG, "JSON-Daten DiagDescriptions: %s", buf);
            // Nullterminator hinzufügen, um den Body als String zu behandeln
            

            // JSON parsen
            cJSON *ECUName_DiagVersion_json = cJSON_Parse(buf);
            free(buf); // Speicher des Buffers freigeben, da er nicht mehr benötigt wird
            if (!ECUName_DiagVersion_json) {
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON format");
                free (filename);
                return ESP_FAIL;
            }

            cJSON *data_json = cJSON_GetObjectItem(ECUName_DiagVersion_json, "ECUName_DiagVersion");
            if (!cJSON_IsString(data_json)){
                cJSON_Delete(data_json);
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or invalid 'ECUName_DiagVersion' field");
                free (filename);
                return ESP_FAIL;
            }

            //Get relevant Data
            uint32_t length =strlen(data_json->valuestring);
            ECUName_DiagVersion = malloc(length +1 );
            strcpy(ECUName_DiagVersion,data_json->valuestring);
            //ESP_LOGI(TAG,"ECUName_DiagVersion, %s", ECUName_DiagVersion);
            //ESP_LOGI(TAG,"ECUName_DiagVersion, %s",ECUName_DiagVersion);
            char* separator = strchr(ECUName_DiagVersion, '_'); // Suche nach dem Unterstrich
            if (separator != NULL) {
                // Teile den String an der Position des Unterstrichs
                *separator = '\0'; // Unterstrich durch Nullzeichen ersetzen
            }
            ECUName = ECUName_DiagVersion;
            DiagVersion = separator + 1;        // Rechter Teil
            //free(ECUName_DiagVersion);
            ESP_LOGI(TAG,"ECUName: %s", ECUName);
            ESP_LOGD(TAG,"DiagVersion: %s", DiagVersion);

            strcpy(filepathfatfs, base_path_sd);
            strcat(filepathfatfs,"/DiagDescriptions");
            //Get Correct Filename
            */

            //TestSD
            /*
            //findFile(ECUName, DiagVersion,filepathfatfs,filename);
            strcat(filepathfatfs,"/");
            if(strcmp(ECUName, "CPC") == 0){
                strcpy(filename, "CPC04T_0x0612.json.gz");
            }else if(strcmp(ECUName, "MCM") == 0){
                strcpy (filename,"MCM21T_0x0CF2.json.gz");
            }else if(strcmp(ECUName, "ACM") == 0){
                strcpy (filename, "ACM301T_0x1E58.json.gz");
            }
            strcat(filepathfatfs,filename);
            ESP_LOGI(TAG,"DiagDataFilepath: %s", filepathfatfs);
            */

            //Check if Filename has the correct format and if the file is existing
            esp_err_t ret;
            ret=checkonFilename(filename, req, &file_stat, filepathfatfs);
            //Only if it is existing and has the right format, it shall be sent
            if(ret==ESP_OK){
              //ReadFiles and send over HTTP
              readsendFile(filename, filepathfatfs, req, &file_stat);
            }else{
            ESP_LOGE(TAG,"CheckonFilename returned: %d", ret);
            }
            free (filename);}
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////                                                        
    //In any other cases, the filename is contained in the URI                                                     
    else{
        //Anything else is stored in the basepath /data

        char *filename_data = get_path_from_uri(filepathlfs, ((struct file_server_data *)req->user_ctx)->base_path_flash,
                                                         req->uri, sizeof(filepathlfs));

        //Check if Filename has the correct format and if the file is existing
        esp_err_t ret;
        ret=checkonFilename(filename_data, req, &file_stat, filepathlfs);
        //Only if it is existing and has the right format, it shall be sent
        if(ret==ESP_OK){
            //ReadFiles and send over HTTP
            readsendFile(filename_data, filepathlfs, req, &file_stat);
        }else{
            ESP_LOGE(TAG,"CheckonFilename returned: %d", ret);
        }

    }
    
    return ESP_OK;
}

esp_err_t ws_handler(httpd_req_t *req)
{
 if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }
    ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);
    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
        strcmp((char*)ws_pkt.payload,"Trigger async") == 0) {
        free(buf);
        //return trigger_async_send(req->handle, req);
    }

    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }
    free(buf);
    return ret;
}


// Helper Function to print out the HTTP server event
static const char *http_event_check(int32_t event_id) {

    //In case of:
        //The Webserver was restarted in the call before
        //The Server Event is not "Disconnected" or "Error"
    //Set the bool to "false"
    if(webserver_restarted==true && event_id != HTTP_SERVER_EVENT_DISCONNECTED && event_id != HTTP_SERVER_EVENT_ERROR){
        webserver_restarted=false;
    }

    switch (event_id) {
        case HTTP_SERVER_EVENT_ERROR:
            if (webserver_restarted==false){
                restart_webserver();
                index_uri_counter=0;
                webserver_restarted=true;
            } 
            return "HTTP_SERVER_EVENT_ERROR";
        case HTTP_SERVER_EVENT_START: return "HTTP_SERVER_EVENT_START";
        case HTTP_SERVER_EVENT_ON_CONNECTED:
            return "HTTP_SERVER_EVENT_ON_CONNECTED";
        case HTTP_SERVER_EVENT_ON_HEADER: return "HTTP_SERVER_EVENT_ON_HEADER";
        case HTTP_SERVER_EVENT_HEADERS_SENT: return "HTTP_SERVER_EVENT_HEADERS_SENT";
        case HTTP_SERVER_EVENT_ON_DATA: return "HTTP_SERVER_EVENT_ON_DATA";
        case HTTP_SERVER_EVENT_SENT_DATA: return "HTTP_SERVER_EVENT_SENT_DATA";
        case HTTP_SERVER_EVENT_DISCONNECTED:
            if (webserver_restarted==false){
                //restart_webserver();
                webserver_restarted=true;
            }
            return "HTTP_SERVER_EVENT_DISCONNECTED";
        case HTTP_SERVER_EVENT_STOP: return "HTTP_SERVER_EVENT_STOP";
        default: return "UNKNOWN_EVENT_ID";
    }
}

// Event handler for HTTP server events
static void http_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    const char *event_name = http_event_check(event_id);
    ESP_LOGD(TAG, "HTTP Event ID: %d (%s)", (unsigned int)event_id, event_name);

}

// Function to register the HTTP server event handler
static void register_http_event_handler(void) {
    ESP_ERROR_CHECK(esp_event_handler_instance_register(ESP_HTTP_SERVER_EVENT,
                    ESP_EVENT_ANY_ID,
                    &http_event_handler,
                    NULL,
                    NULL));
}

static struct file_server_data *server_data=NULL;
/* HTTP-Server initialisieren */
esp_err_t start_webserver(void)
{
    led_actuation_order.LED_color=DEFAULT;
    led_actuation_order.breaktime=50;
    xQueueSend(handle_led_actuation_queue, &led_actuation_order, portMAX_DELAY);
    
   
    // Handle Flash server data
    if (server_data) {
        ESP_LOGE(TAG, "File server already started");
        return ESP_ERR_INVALID_STATE;
    }

    /* Allocate memory for server data */
    server_data = calloc(1, sizeof(struct file_server_data));
    if (!server_data) {
        ESP_LOGE(TAG, "Failed to allocate memory for server data");
        return ESP_ERR_NO_MEM;
    }
    strlcpy(server_data->base_path_flash, base_path_flash,
            sizeof(server_data->base_path_flash));
    strlcpy(server_data->base_path_sd, base_path_sd,
            sizeof(server_data->base_path_sd));

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 30; // Increase this value as needed
    //config.max_open_sockets=1;
    //config.close_fn = on_client_disconnect;

    // Starten des HTTP-Servers
    ESP_LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server!");
        return ESP_FAIL;
    }

    // Register the HTTP server event handler
    register_http_event_handler();

    // Array von URIs und deren Handler
    httpd_uri_t uris[] = {
        { .uri = "/identification.html", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/home.html", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/config.json", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/00_StartUp_getAssets.js", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/01_StartUp_ScanECus.js", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/02_StartUp_getDiagDescriptions.js", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/03_StartUp_PopulateTables.js", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/04_Common_DiagnosticRequest.js", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/05_Common_ShowContent.js", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/06_Common_CreateContent.js", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/07_Common_HandleIdentifications.js", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/08_Common_HandleMeasurements.js", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/09_Common_HandleFaultCodes.js", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/09_Common_InterpretData.js", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/favicon.ico", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/99_Trash.js", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/sdcard", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/", .method = HTTP_GET, .handler = start_download_handler, .user_ctx = server_data },
        { .uri = "/uds_request", .method = HTTP_POST, .handler = uds_request_handler, .user_ctx = NULL },
        { .uri = "/ws", .method = HTTP_GET, .handler = ws_handler, .user_ctx = NULL, .is_websocket = true },
        { .uri = "/ipv4address", .method = HTTP_GET, .handler = IPAddress_handler, .user_ctx = NULL}
    };

    // URIs registrieren
    for (int i = 0; i < sizeof(uris) / sizeof(uris[0]); i++) {
        httpd_register_uri_handler(server, &uris[i]);
    }

    return ESP_OK;
}


char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest) {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash) {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize) {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, base_path);
    strlcpy(dest + base_pathlen, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest + base_pathlen;
}

/*
esp_err_t http_resp_dir_html(httpd_req_t *req, const char *dirpath)
{
    char entrypath[FILE_PATH_MAX];
    char entrysize[16];
    const char *entrytype;

    struct dirent *entry;
    struct stat entry_stat;

    DIR *dir = opendir(dirpath);
    const size_t dirpath_len = strlen(dirpath);

    //Retrieve the base path of file storage to construct the full path
    strlcpy(entrypath, dirpath, sizeof(entrypath));

    if (!dir) {
        ESP_LOGE(TAG, "Failed to stat dir : %s", dirpath);
        //Respond with 404 Not Found
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Directory does not exist");
        return ESP_FAIL;
    }

    //Send HTML file header 
    httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html><body>");

    //Get handle to embedded file upload script
    extern const unsigned char upload_script_start[] asm("_binary_upload_script_html_start");
    extern const unsigned char upload_script_end[]   asm("_binary_upload_script_html_end");
    const size_t upload_script_size = (upload_script_end - upload_script_start);

    //Add file upload form and script which on execution sends a POST request to /upload
    httpd_resp_send_chunk(req, (const char *)upload_script_start, upload_script_size);

    //Send file-list table definition and column labels
    httpd_resp_sendstr_chunk(req,
        "<table class=\"fixed\" border=\"1\">"
        "<col width=\"800px\" /><col width=\"300px\" /><col width=\"300px\" /><col width=\"100px\" />"
        "<thead><tr><th>Name</th><th>Type</th><th>Size (Bytes)</th><th>Delete</th></tr></thead>"
        "<tbody>");

    //Iterate over all files / folders and fetch their names and sizes
    while ((entry = readdir(dir)) != NULL) {
        entrytype = (entry->d_type == DT_DIR ? "directory" : "file");

        strlcpy(entrypath + dirpath_len, entry->d_name, sizeof(entrypath) - dirpath_len);
        if (stat(entrypath, &entry_stat) == -1) {
            ESP_LOGE(TAG, "Failed to stat %s : %s", entrytype, entry->d_name);
            continue;
        }
        sprintf(entrysize, "%ld", entry_stat.st_size);
        ESP_LOGD(TAG, "Found %s : %s (%s bytes)", entrytype, entry->d_name, entrysize);

        //Send chunk of HTML file containing table entries with file name and size 
        httpd_resp_sendstr_chunk(req, "<tr><td><a href=\"");
        httpd_resp_sendstr_chunk(req, req->uri);
        httpd_resp_sendstr_chunk(req, entry->d_name);
        if (entry->d_type == DT_DIR) {
            httpd_resp_sendstr_chunk(req, "/");
        }
        httpd_resp_sendstr_chunk(req, "\">");
        httpd_resp_sendstr_chunk(req, entry->d_name);
        httpd_resp_sendstr_chunk(req, "</a></td><td>");
        httpd_resp_sendstr_chunk(req, entrytype);
        httpd_resp_sendstr_chunk(req, "</td><td>");
        httpd_resp_sendstr_chunk(req, entrysize);
        httpd_resp_sendstr_chunk(req, "</td><td>");
        httpd_resp_sendstr_chunk(req, "<form method=\"post\" action=\"/delete");
        httpd_resp_sendstr_chunk(req, req->uri);
        httpd_resp_sendstr_chunk(req, entry->d_name);
        httpd_resp_sendstr_chunk(req, "\"><button type=\"submit\">Delete</button></form>");
        httpd_resp_sendstr_chunk(req, "</td></tr>\n");
    }
    closedir(dir);

    //Finish the file list table
    httpd_resp_sendstr_chunk(req, "</tbody></table>");

    //Send remaining chunk of HTML file to complete it
    httpd_resp_sendstr_chunk(req, "</body></html>");

    //Send empty chunk to signal HTTP response completion
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}
*/

void remove_question_mark(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (*src != '?') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0'; // Nullterminator hinzufügen

}

/*
esp_err_t on_client_disconnect(httpd_handle_t server, int sock_fd)
{
    ESP_LOGI(TAG, "Client disconnected: socket_fd=%d", sock_fd);
          // Ensure handle is non NULL
       if (server != NULL) {
           // Stop the httpd server
           httpd_stop(server);
       }
    // Hier kannst du Ressourcen freigeben oder Statistiken aktualisieren
    return ESP_OK;
}
*/

esp_err_t restart_webserver(void)
{
        // Stop the httpd server
        httpd_stop(server);
        ESP_LOGI(TAG, "Server stopped");
        // Free the server_data if it is already allocated
        if (server_data) {
            free(server_data);
            server_data = NULL;
            ESP_LOGI(TAG, "Server data freed");
        }
        // Start the httpd server
        start_webserver();
        return ESP_OK;


    return ESP_OK;

}