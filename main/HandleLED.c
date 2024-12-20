#include "HandleLED.h"

void handleLED_task(){
led_actuation_t led_actuation_order;
led_actuation_t led_actuation_status;
configureLEDs();
while (1){    
        //Rot

        //Block the task as long nothing is received from the queue
    if (xQueueReceive(handle_led_actuation_queue ,&led_actuation_order , portMAX_DELAY) == pdPASS) {
        //ESP_LOGI(TAG, "Received LED actuation order");
        switch (led_actuation_order.LED_color){
            case RED:
                gpio_set_level(5, 1);
                gpio_set_level(6, 0);
                gpio_set_level(7, 0);
                led_actuation_status.LED_color=RED;
                break;
            case GREEN:
                gpio_set_level(5, 0);
                gpio_set_level(6, 1);
                gpio_set_level(7, 0);
                led_actuation_status.LED_color=GREEN;
                break;
            case BLUE:
                gpio_set_level(5, 0);
                gpio_set_level(6, 0);
                gpio_set_level(7, 1);
                led_actuation_status.LED_color=BLUE; 
                break;  
            case DEFAULT: 
                break;          
        }
        if (led_actuation_order.breaktime != 0){
            gpio_set_level(5, 0);
            gpio_set_level(6, 0);
            gpio_set_level(7, 0);
            vTaskDelay(led_actuation_order.breaktime / portTICK_PERIOD_MS);
            //Wieder auf vorherigen Wert zurücksetzen
            switch (led_actuation_status.LED_color){
                case RED:
                    gpio_set_level(5, 1);
                    gpio_set_level(6, 0);
                    gpio_set_level(7, 0);
                    led_actuation_status.LED_color=RED;
                    break;
                case GREEN:
                    gpio_set_level(5, 0);
                    gpio_set_level(6, 1);
                    gpio_set_level(7, 0);
                    led_actuation_status.LED_color=GREEN;
                    break;
                case BLUE:
                    gpio_set_level(5, 0);
                    gpio_set_level(6, 0);
                    gpio_set_level(7, 1);
                    led_actuation_status.LED_color=BLUE; 
                    break; 
                case DEFAULT: 
                    break;          
            }

        }
    }
}
}

void configureLEDs(){
    gpio_set_direction(5, GPIO_MODE_OUTPUT);
    gpio_set_direction(6, GPIO_MODE_OUTPUT);
    gpio_set_direction(7, GPIO_MODE_OUTPUT);

}