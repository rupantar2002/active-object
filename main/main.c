#include "active_obj.h"
#include "bsp.h"
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>

#define INITIAL_BLINK_TIME (ACTIVE_OBJ_TIMEOUT_FREQ_HZ / 4);

/**
 * button inherite active object as a super class.
 */
typedef struct
{
    active_obj_instance_t super;
    // state var
    enum
    {
        WAIT_FOR_BUTTON_STATE,
        BLINK_STATE,
        PAUSED_STATE,
        BOOM_STATE,
    } state;
    active_obj_timeEvent_t te;
    uint8_t btnCount;
} TimeBomb_t;

/** -----------------------constants---------------------------------*/

static const char *TAG = __FILE__;

/** -----------------private global---------------------------------*/
static TimeBomb_t gBlinkyBtn;

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

static void TimeBombEventDespatcher(TimeBomb_t *const self,
                                    active_obj_event_t const *const evt)
{
    if (evt->sig == ACTIVE_OBJ_RESV_SIGNALS_INIT)
    {
        bsp_GrnLedOn();
        self->state = WAIT_FOR_BUTTON_STATE;
        return;
    }

    switch (self->state)
    {
    case WAIT_FOR_BUTTON_STATE:
    {
        switch (evt->sig)
        {
        case BSP_EVENT_SIG_BTN_PRESSED:
        {
            bsp_GrnLedOff();
            bsp_RedLedOn();
            active_obj_TimeEventArm(&self->te, ACTIVE_OBJ_MS_TO_INTERVAL(500), 0);
            self->btnCount = 3;
            self->state = BLINK_STATE;
            break;
        }
        }
    }
    break;
    case BLINK_STATE:
    {
        switch (evt->sig)
        {
        case BSP_EVENT_SIG_TIMEOUT:
        {
            bsp_RedLedOff();
            active_obj_TimeEventArm(&self->te, ACTIVE_OBJ_MS_TO_INTERVAL(500), 0);
            self->state = PAUSED_STATE;
            break;
        }
        }
        break;
    }
    case PAUSED_STATE:
    {
        switch (evt->sig)
        {
        case BSP_EVENT_SIG_TIMEOUT:
        {
            --self->btnCount;
            if (self->btnCount > 0)
            {
                bsp_RedLedOn();
                active_obj_TimeEventArm(&self->te, ACTIVE_OBJ_MS_TO_INTERVAL(500), 0);
                self->state = BLINK_STATE;
            }
            else
            {
                bsp_GrnLedOn();
                bsp_RedLedOn();
                ESP_LOGE(TAG, "\r\n<------------BOMB SUCCESSFULLY DETONATED-------------->\r\n");
                self->state = BOOM_STATE;
            }
            break;
        }
        }
    }
    break;
    case BOOM_STATE:
        break;
    }
}

static void TimeBombInit(TimeBomb_t *const self)
{
    active_obj_Init((active_obj_instance_t *)self, TimeBombEventDespatcher);
    active_obj_TimeEventInit(&self->te, BSP_EVENT_SIG_TIMEOUT, &self->super);
}
/*-------------------------------------------------------*/

void app_main(void)
{
    bsp_Init();
    TimeBombInit((active_obj_instance_t *)&gBlinkyBtn);
    active_obj_Start((active_obj_instance_t *)&gBlinkyBtn);
    // bsp
    bsp_Start();
}
