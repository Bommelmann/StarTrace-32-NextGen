#include "monitor_task.h"

void monitor_task(void *pvParameters) {

uint32_t free_heap_size;
    while (1) {
        free_heap_size=esp_get_free_heap_size();
        ESP_LOGI("HeapMonitor", "Freier Heap: %d Bytes", (unsigned int)free_heap_size);
        //ESP_LOGI("HeapMonitor", "Freier Stack: %d Bytes", uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(pdMS_TO_TICKS(2000));  // Alle 5 Sekunden

    }
}