#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <sys/stat.h>
#include "esp_vfs.h"
#include "ff.h"
#include <string.h>
#include <stdlib.h>
#include "errno.h"
#include "messages.h"
#include "queues.h"

extern led_actuation_t led_actuation_order;

esp_err_t checkonFilename(const char *filename, httpd_req_t *req, struct stat *file_stat, char *filepath);
esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename);
esp_err_t readsendFile(const char *filename, char * filepath, httpd_req_t *req, struct stat *file_stat,FILE *fd);
bool findFile(const char *ECUName, const char *DiagVersion, char *filepath, char *filename);
int extract_hex_value_from_filename(const char *filename);
int extract_hex_value_from_DiagVers(const char *DiagVersion);
bool hasNTerminator(const char *str, size_t length);
#define FILE_PATH_MAX_LFS (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)
#define FILE_PATH_MAX_FATFS  CONFIG_FATFS_MAX_LFN
#define FILE_PATH_MAX 256

/* Scratch buffer size */
#define SCRATCH_BUFSIZE  8192

struct file_server_data {
    /* Base path of file storage */
    char base_path_sd[ESP_VFS_PATH_MAX + 1];
    char base_path_flash[ESP_VFS_PATH_MAX + 1];

    /* Scratch buffer for temporary storage during file transfer */
    char scratch[SCRATCH_BUFSIZE];
};

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)


