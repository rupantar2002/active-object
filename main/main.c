#include "active_obj.h"
#include "bsp.h"
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>

/**
 * button inherite active object as a super class.
 */
typedef struct
{
    active_obj_instance_t super;
} Button_t;

static const char *TAG = __FILE__;

/** -----------------private global---------------------------------*/
static Button_t gButton;

/** -----------------public global----------------------------------*/

/**
 * this pointer has a visibility scope outside of this translation
 * unit or simply c src file.the exact signature should be used carefully
 * when creating non static global variables in another translation unit or
 * c src file or header files.Redefinition of this variable could case unwanted
 * flow of code.
 *
 */
active_obj_instance_t *pgActiveButton = &gButton.super;

static void ButtonEventDespatcher(Button_t *const self, active_obj_event_t const *const evt)
{
    switch (evt->sig)
    {
    case ACTIVE_OBJ_RESV_SIGNALS_INIT:
        ESP_LOGI(TAG, "%d:%s,%s", __LINE__, __func__, "ACTIVE_OBJ_RESV_SIGNALS_INIT");
        bsp_GrnLedOff();
        break;
    case BSP_EVENT_SIG_BTN_PRESSED:
        bsp_GrnLedOn();
        break;
    case BSP_EVENT_SIG_BTN_RELEASED:
        bsp_GrnLedOff();
        break;
    default:
        break;
    }
}

static void ButtonInit(Button_t *const self)
{
    active_obj_Init((active_obj_instance_t *)self, ButtonEventDespatcher);
}

static void LedBlinkTask(void *pvPrams)
{
    while (1)
    {
        bsp_RedLedOn();
        vTaskDelay(pdMS_TO_TICKS(500));
        bsp_RedLedOff();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    bsp_Init();
    // led blinking routine
    xTaskCreate(LedBlinkTask, "LedBlinkTask", 1024 * 2, NULL, 1, NULL);
    // button active object
    ButtonInit((active_obj_instance_t *)&gButton);
    active_obj_Start((active_obj_instance_t *)&gButton);
    // bsp
    bsp_Start();
}
