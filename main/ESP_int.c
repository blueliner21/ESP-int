#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_PIN 2
#define INPUT_PIN 7
#define BLINK_PIN 4


int state = 0;
QueueHandle_t interputQueue = NULL;

static void gpio_interrupt_handler(void *args)
{
    int pinNumber = (int)args;
    xQueueSendFromISR(interputQueue, &pinNumber, NULL);
}

void LED_Control_Task(void *params)
{
    int pinNumber, count = 1;
    while (true)
    {
        if (xQueueReceive(interputQueue, &pinNumber, portMAX_DELAY))
        {
            state = !state;
            gpio_set_level(LED_PIN, state);
            printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, state);
        }
    }
}

void Blink_Task(void *params)
{
    int blink_state = 0;
    while(1)
    {
        gpio_set_level(BLINK_PIN,blink_state);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        blink_state = !blink_state;
        gpio_set_level(BLINK_PIN,blink_state);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        blink_state = !blink_state;
    }
}

void app_main()
{

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    gpio_set_direction(INPUT_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(INPUT_PIN);
    gpio_pullup_dis(INPUT_PIN);
    gpio_set_intr_type(INPUT_PIN, GPIO_INTR_POSEDGE);

    gpio_set_direction(BLINK_PIN, GPIO_MODE_OUTPUT);

    interputQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(LED_Control_Task, "LED_Control_Task", 2048, NULL, 1, NULL);
    xTaskCreate(Blink_Task, "Blinker", 2048, NULL, 1, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(INPUT_PIN, gpio_interrupt_handler, (void *)INPUT_PIN);
}
