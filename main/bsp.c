#include "event-driven/active.h"
#include "bsp.h"
#include "user_input.h"
#include <driver/gpio.h>
#include <esp_log.h>

#define BSP_GREEN_LED_GPIO (15)

#define BSP_RED_LED_GPIO (16)

#define BSP_BUTTON_GPIO (2)

static const char *TAG = __FILE__;

static event_base_t gButtonEvent;

void bsp_Init(void)
{
    gpio_set_direction(BSP_RED_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BSP_GREEN_LED_GPIO, GPIO_MODE_OUTPUT);
    user_input_Init();
}

void bsp_Start(void)
{
    bsp_GrnLedOff();
    bsp_RedLedOff();
    user_input_Start();
}

void bsp_RedLedOn(void)
{
    gpio_set_level(BSP_RED_LED_GPIO, 1);
}

void bsp_RedLedOff(void)
{
    gpio_set_level(BSP_RED_LED_GPIO, 0);
}

void bsp_GrnLedOn(void)
{
    gpio_set_level(BSP_GREEN_LED_GPIO, 1);
}

void bsp_GrnLedOff(void)
{
    gpio_set_level(BSP_GREEN_LED_GPIO, 0);
}

/**
 * function internaly handles button debouncting reated operation.
 */
void user_input_Switch1StateCallback(uint8_t state)
{
    // ESP_LOGI(TAG, "%d:%s,state: %d", __LINE__, __func__, state);
    state ? (gButtonEvent.sig = BSP_EVENT_SIG_BTN_RELEASED) : (gButtonEvent.sig = BSP_EVENT_SIG_BTN_PRESSED);
    active_PostEvent(pgActiveButton, &gButtonEvent);
}