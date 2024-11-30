#include "createWebserver.h"

static const char *TAG = "createwebserver.c";
/* Initialize file storage */
const char* base_path = "/data";


esp_err_t uds_request_handler(httpd_req_t *req)
{
    
    // Länge des Bodys ermitteln
    size_t buf_len = req->content_len;
    if (buf_len == 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No JSON body in Request");
        return ESP_FAIL;
    }

    // Speicher für den Body reservieren (+1 für Nullterminator)
    char *buf = malloc(buf_len + 1);
    if (!buf) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }

    // Body aus der Anfrage lesen
    int ret = httpd_req_recv(req, buf, buf_len);
    if (ret <= 0) {
        free(buf);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to read request body");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "JSON-Daten: %s", buf);
    // Nullterminator hinzufügen, um den Body als String zu behandeln
    

    // JSON parsen
    cJSON *uds_rqst_rspns_json = cJSON_Parse(buf);
    free(buf); // Speicher des Buffers freigeben, da er nicht mehr benötigt wird
    if (!uds_rqst_rspns_json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON format");
        return ESP_FAIL;
    }

    
    cJSON *uds_rqst_json = cJSON_GetObjectItem(uds_rqst_rspns_json, "request");
    if (!cJSON_IsString(uds_rqst_json)){
        cJSON_Delete(uds_rqst_json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or invalid 'Request' field");
        return ESP_FAIL;
    }

    // Werte aus dem JSON extrahieren
    uds_message_string_t uds_rqst_rspns_string;
    uds_rqst_rspns_string.uds_request_length = strlen(uds_rqst_json->valuestring);
    uds_rqst_rspns_string.uds_request_string = malloc(uds_rqst_rspns_string.uds_request_length+1);
    strcpy(uds_rqst_rspns_string.uds_request_string,uds_rqst_json->valuestring);
    ESP_LOGI(TAG, "RequestData in uds_message_string_t: %s", uds_rqst_rspns_string.uds_request_string);
    ESP_LOGI(TAG, "RequestLength in uds_message_string_t: %d", (int)uds_rqst_rspns_string.uds_request_length);
    xQueueSend(handle_uds_request_queue, &uds_rqst_rspns_string, portMAX_DELAY);
    xQueueReceive(handle_uds_response_queue, &uds_rqst_rspns_string, portMAX_DELAY);
    ESP_LOGI(TAG, "ResponseData in uds_message_string_t: %s", uds_rqst_rspns_string.uds_response_string);
    ESP_LOGI(TAG, "ResponseLength in uds_message_string_t: %d", (int)uds_rqst_rspns_string.uds_response_length);

    cJSON *uds_rspns_json = cJSON_GetObjectItem(uds_rqst_rspns_json, "response");
    cJSON_SetValuestring(uds_rspns_json,uds_rqst_rspns_string.uds_response_string);
    // Das JSON-Objekt in einen String umwandeln
    char *json_str = cJSON_Print(uds_rqst_rspns_json);
    ESP_LOGI(TAG, "json_str: %s", json_str);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json_str);

    // Objekte freigeben
    free(json_str);
    cJSON_Delete(uds_rqst_rspns_json);

    return ESP_OK;
}



esp_err_t start_download_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat file_stat;
    const char *filename;

    //Case that this is the first request for index.html:
    if (strcmp(req->uri, "/") == 0)
    {
        filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                                         "/index.html", sizeof(filepath));
    //In any other cases, the filename is contained in the URI                                                     
    }else{
        filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                                         req->uri, sizeof(filepath));
    }
    //ESP_LOGI(TAG,"Filepath: %s", filepath);
    //ESP_LOGI(TAG, "Base path: %s", ((struct file_server_data *)req->user_ctx)->base_path);
    //ESP_LOGI(TAG,"req->uri: %s", req->uri);                    
    //ESP_LOGI(TAG,"Filename: %s", filename);
        
    if (!filename) {
        ESP_LOGE(TAG, "Filename is too long");

        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    /* If name has trailing '/', respond with directory contents */
    if (filename[strlen(filename) - 1] == '/') {
        //return http_resp_dir_html(req, filepath);
    }

    
        if (stat(filepath, &file_stat) == -1) {
    //       /* If file not present on SPIFFS check if URI
    //        * corresponds to one of the hardcoded paths */
            ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
            /* Respond with 404 Not Found */
            httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
    //     return ESP_FAIL;
    }
    

        //////////////////////////////////////
        //LittleFS

    fd = fopen(filepath, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);
    set_content_type_from_file(req, filename);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
    size_t chunksize;
    do {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        if (chunksize > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                fclose(fd);
                ESP_LOGE(TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
            return ESP_FAIL;
        }
        }

        /* Keep looping till the whole file is sent */
    } while (chunksize != 0);

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(TAG, "LittfleFS File sending complete");
    
    /*

    //////////////////////////////////////
    //SDCard
    fd = fopen("/sdcard/hello.txt", "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        //Respond with 500 Internal Server Error
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);
    set_content_type_from_file(req, filename);

    //Retrieve the pointer to scratch buffer for temporary storage
    chunk = ((struct file_server_data *)req->user_ctx)->scratch;
    
    do {
        //Read file in chunks into the scratch buffer
        chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        if (chunksize > 0) {
            //Send the buffer contents as HTTP response chunk
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                fclose(fd);
                ESP_LOGE(TAG, "File sending failed!");
                //Abort sending file
                httpd_resp_sendstr_chunk(req, NULL);
                //Respond with 500 Internal Server Error
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
               return ESP_FAIL;
           }
        }

        //Keep looping till the whole file is sent
    } while (chunksize != 0);

    //Close file after sending complete
    fclose(fd);
    ESP_LOGI(TAG, "SD File sending complete");

    //Respond with an empty chunk to signal HTTP response completion
    httpd_resp_send_chunk(req, NULL, 0);

    */
    //Mit Übermittlung eines leeren Chunks, signalisiert das Ende derübertragung
    httpd_resp_send_chunk(req, NULL, 0); // Signalisiert das Ende der Übertragung

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
/* HTTP-Server initialisieren */
esp_err_t start_webserver(void)
{
   static struct file_server_data *server_data = NULL;

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
    strlcpy(server_data->base_path, base_path,
            sizeof(server_data->base_path));

    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Starten des HTTP-Servers
        ESP_LOGD(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }

        httpd_uri_t start_download_uri_config = {
            .uri       = "/config.json",
            .method    = HTTP_GET,
            .handler   = start_download_handler,
            .user_ctx  = server_data
        };
        httpd_register_uri_handler(server, &start_download_uri_config);

        httpd_uri_t start_download_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = start_download_handler,
            .user_ctx  = server_data
        };
        httpd_register_uri_handler(server, &start_download_uri);

        httpd_uri_t uds_request_uri = {
            .uri       = "/uds_request",
            .method    = HTTP_POST,
            .handler   = uds_request_handler,
            .user_ctx  = NULL
        };

        httpd_register_uri_handler(server, &uds_request_uri);

        httpd_uri_t ws_uri = {
            .uri        = "/ws",
            .method     = HTTP_GET,
            .handler    = ws_handler,
            .user_ctx   = NULL,
            .is_websocket = true
        };
        httpd_register_uri_handler(server, &ws_uri);
    
    return ESP_OK;
}


const char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
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

esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename)
{
    if (IS_FILE_EXT(filename, ".pdf")) {
        return httpd_resp_set_type(req, "application/pdf");
    } else if (IS_FILE_EXT(filename, ".html")) {
        return httpd_resp_set_type(req, "text/html");
    } else if (IS_FILE_EXT(filename, ".jpeg")) {
        return httpd_resp_set_type(req, "image/jpeg");
    } else if (IS_FILE_EXT(filename, ".ico")) {
        return httpd_resp_set_type(req, "image/x-icon");
    } else if (IS_FILE_EXT(filename, ".json")) {
        return httpd_resp_set_type(req, "application/json");
    }
    /* This is a limited set only */
    /* For any other type always set as plain text */
    return httpd_resp_set_type(req, "text/plain");
}