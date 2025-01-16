#include "monitor_task.h"

void monitor_task(void *pvParameters) {
    while (1) {
        ESP_LOGI("HeapMonitor", "Freier Heap: %d Bytes", (unsigned int) esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(2000));  // Alle 5 Sekunden
    }
}