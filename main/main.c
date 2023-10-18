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
    active_obj_timeEvent_t te;
    bool ledState;
    uint32_t blinkTime;
} BlinkyButton_t;

/** -----------------------constants---------------------------------*/

static const char *TAG = __FILE__;

/** -----------------private global---------------------------------*/
static BlinkyButton_t gBlinkyBtn;

/** -----------------public global----------------------------------*/

/**
 * this pointer has a visibility scope outside of this translation
 * unit or simply c src file.the exact signature should be used carefully
 * when creating non static global variables in another translation unit or
 * c src file or header files.Redefinition of this variable could case unwanted
 * flow of code.
 *
 */
active_obj_instance_t *pgActiveButton = &gBlinkyBtn.super;

/*--------------------BUTTON----------------------------*/

static void BlinkyButtonEventDespatcher(BlinkyButton_t *const self,
                                        active_obj_event_t const *const evt)
{
    switch (evt->sig)
    {
    case ACTIVE_OBJ_RESV_SIGNALS_INIT:
        bsp_GrnLedOff();
    case BSP_EVENT_SIG_LED_TIMEOUT:
        ESP_LOGI(TAG, "timeout event arise");
        if (self->ledState)
        {
            bsp_RedLedOff();
            self->ledState = false;
            active_obj_TimeEventArm(&self->te, ACTIVE_OBJ_MS_TO_INTERVAL(500), 0);
        }
        else
        {
            bsp_RedLedOn();
            self->ledState = true;
            active_obj_TimeEventArm(&self->te, ACTIVE_OBJ_MS_TO_INTERVAL(500), 0);
        }
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

static void BlinkyButtonInit(BlinkyButton_t *const self)
{
    active_obj_Init((active_obj_instance_t *)self, BlinkyButtonEventDespatcher);
    active_obj_TimeEventInit(&self->te, BSP_EVENT_SIG_LED_TIMEOUT, &self->super);
    self->ledState = false;
}
/*-------------------------------------------------------*/

void app_main(void)
{
    bsp_Init();
    BlinkyButtonInit((active_obj_instance_t *)&gBlinkyBtn);
    active_obj_Start((active_obj_instance_t *)&gBlinkyBtn);
    // bsp
    bsp_Start();
}
