#include "HandleFileReading.h"

#define TAG "HandleFileReading.c"


esp_err_t checkonFilename(const char* filename, httpd_req_t *req, struct stat *file_stat, char *filepath){

        // ESP_LOGI(TAG,"Filepath: %s", filepath);
        // ESP_LOGI(TAG, "Base path: %s", ((struct file_server_data *)req->user_ctx)->base_path);
        // ESP_LOGI(TAG,"req->uri: %s", req->uri);                    
        // ESP_LOGI(TAG,"Filename: %s", filename);
    ESP_LOGD(TAG,"Filepath at beginning of CheckonFilename: %s", filepath);
    //If LittleFS is used:
    if (strcmp(((struct file_server_data *)req->user_ctx)->base_path, "/data") == 0) {
        if (strlen(filename) >= ESP_VFS_PATH_MAX) {

        if (!filename) {
        ESP_LOGE(TAG, "Filename is too long");

        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
        }
        }
    }else if (strcmp(((struct file_server_data *)req->user_ctx)->base_path, "/sdcard") == 0) {
        if (strlen(filename) >= FILE_PATH_MAX_FATFS) {

        if (!filename) {
        ESP_LOGE(TAG, "Filename is too long");

        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
        }
        }
    }
    /* If name has trailing '/', respond with directory contents */
    if (filename[strlen(filename) - 1] == '/') {
        //return http_resp_dir_html(req, filepath);
    }
    if (stat(filepath, file_stat) == -1) {
    //       /* If file not present on SPIFFS check if URI
    //        * corresponds to one of the hardcoded paths */
            ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
            /* Respond with 404 Not Found */
            httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
    //     return ESP_FAIL;
    }

    return ESP_OK;

}

esp_err_t readsendFile(const char *filename, char *filepath, httpd_req_t *req, struct stat *file_stat,FILE *fd){

    ESP_LOGD(TAG,"Filepath at beginning of readsendFile: %s", filepath);
    // ESP_LOGI(TAG, "Base path: %s", ((struct file_server_data *)req->user_ctx)->base_path);
    // ESP_LOGI(TAG,"req->uri: %s", req->uri);                    
    // ESP_LOGI(TAG,"Filename: %s", filename);
    fd = fopen(filepath, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat->st_size);
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
    ESP_LOGI(TAG, "File sending successful");
    
    httpd_resp_send_chunk(req, NULL, 0); // Signalisiert das Ende der Übertragung
return ESP_OK;
}


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
    } else if (IS_FILE_EXT(filename, ".gz")) {
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        return httpd_resp_set_type(req, "application/json");
    }    
    /* This is a limited set only */
    /* For any other type always set as plain text */
    return httpd_resp_set_type(req, "text/plain");
}

int extract_hex_value_from_filename(const char *filename) {
    const char *hex_start = strstr(filename, "0x");
    if (hex_start) {
        return (int)strtol(hex_start + 2, NULL, 16);  // Konvertiere Hex-String zu int
    }
    return -1;  // Falls kein Hex-Wert vorhanden ist, setze -1
}

int extract_hex_value_from_DiagVers(const char *DiagVersion) {
    int hex_value = (int)strtol(DiagVersion, NULL, 16);  // Konvertiere Hex-String zu int
    if (hex_value) {
        return hex_value;  // Gib den int-Wert zurück
    }
    return -1;  // Falls kein Hex-Wert gefunden, gib -1 zurück
}



bool findFile(const char *ECUName, const char *DiagVersion, char *filepath, char *filename) {
    DIR *dir;
    struct dirent *dp;
    bool found_exact_match = false;
    char temp_filename[FILE_PATH_MAX] = {0};
    int closest_hex_value = -1;  // Nächstmöglicher niedriger Hex-Wert

    // Ziel-Hex-Wert aus DiagVersion extrahieren
    int target_hex_value = extract_hex_value_from_DiagVers(DiagVersion);
    ESP_LOGI(TAG, "DiagVersion in Hex: %x", target_hex_value);
    if (target_hex_value == -1) {
        ESP_LOGE(TAG, "Could not convert into hex: %s", DiagVersion);
        return false;
    }
    //strcat(filepath,"/");
    ESP_LOGI(TAG, "Durchsuche Verzeichnis: %s", filepath);
    ESP_LOGD(TAG, "Length of filepath: %x", strlen(filepath));
    // Öffne das Verzeichnis
    dir=opendir(filepath);
    //res = f_opendir(&dir, filepath);
    if (dir == NULL) {
        ESP_LOGE(TAG, "Fehler beim Öffnen (opendir) des Verzeichnisses: %s", filepath);
        return false;
    }

    // Durchsuche alle Dateien im Verzeichnis
    while (1) {
        dp=readdir(dir);
        if (dp==NULL){
            ESP_LOGE(TAG, "Fehler beim Lesen (readdir) des Verzeichnisses: %s", filepath);
            return false;
        }

        // Überprüfe auf exakte Übereinstimmung
        if (strstr(dp->d_name, ECUName) && strstr(dp->d_name, DiagVersion)) {
            strcpy(filename, dp->d_name);
            found_exact_match = true;
            ESP_LOGI(TAG,"Found exact match!");
            ESP_LOGI(TAG,"Filepath: %s", filepath);
            ESP_LOGI(TAG,"Filename: %s", filename);
            //Nullterminator einfügen
            filename[strlen(filename)] = '\0';
            break;
        }

        // Überprüfe auf alternative Übereinstimmung
        if (strstr(dp->d_name, ECUName)) {
            int hex_value = extract_hex_value_from_filename(dp->d_name);
            if (hex_value >= 0 && hex_value < target_hex_value && (closest_hex_value == -1 || hex_value > closest_hex_value)) {
                closest_hex_value = hex_value;
                strncpy(temp_filename, dp->d_name, FILE_PATH_MAX);
                ESP_LOGI(TAG,"Found alternative match!");
                ESP_LOGI(TAG,"Filepath: %s", filepath);
                ESP_LOGI(TAG,"Temp_Filename: %s", temp_filename);

            }
        }
    }

    closedir(dir);

    // Wenn keine exakte Übereinstimmung gefunden wurde, setze den alternativen Treffer
    if (!found_exact_match && temp_filename[0] != '\0') {
        strncpy(filename, temp_filename, FILE_PATH_MAX);
        ESP_LOGI(TAG,"Found alternative match!");
        ESP_LOGI(TAG,"Filepath: %s", filepath);
        ESP_LOGI(TAG,"Filename: %s", filename);
    }

    return found_exact_match;
}


bool hasNTerminator(const char *str, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (str[i] == '\0') {
            ESP_LOGI(TAG, "Found NULL terminator at position %d", i);
            return true;
        }
    }
    ESP_LOGI(TAG, "No NULL terminator found");
    return false;
}