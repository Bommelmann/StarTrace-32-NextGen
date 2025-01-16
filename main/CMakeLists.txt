idf_component_register(SRCS "HandleFileReading.c" "InitializeFilesystem.c"
                             "initWifi.c"
                             "main.c"
                             "createWebserver.c"
                             "InitializeFilesystem.c"
                             "initializeSDCard.c"
                             "isotp_link_containers.c" 
                             "twai_tasks.c"
                             "isotp_link_containers.c" 
                            "isotp.c"
                            "isotp_links.c"
                            "periodic_messages.c"
                            "isotp_link_containers.c"
                            "isotp_tasks.c"
                            "endian_helpers.c"
                            "isotp_callbacks.c" 
                            "HandleUDSRequest.c"
                            "functions.c"
                            "HandleFileReading.c"
                            "HandleLED.c"
                            "monitor_task.c"                           
                                                                                    
                    INCLUDE_DIRS "." 
                    REQUIRES esp_timer driver fatfs esp_wifi nvs_flash spiffs esp_http_server json
                    EMBED_FILES  "")               

# Note: you must have a partition named the first argument (here it's "littlefs")
# in your partition table csv file.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    littlefs_create_partition_image(storage ../flash_data FLASH_IN_PROJECT)
else()
 #   fail_at_build_time(littlefs "Windows does not support LittleFS partition generation")
endif()
