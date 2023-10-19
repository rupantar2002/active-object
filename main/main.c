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
        BLINK_OFF_STATE,
        BLINK_ON_STATE,
    } state;
    active_obj_timeEvent_t te;
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
    if (evt->sig == ACTIVE_OBJ_RESV_SIGNALS_INIT)
    {
        bsp_RedLedOff();
        active_obj_TimeEventArm(&self->te, self->blinkTime, 0);
        self->state = BLINK_OFF_STATE;
        return;
    }

    switch (self->state)
    {
    case BLINK_OFF_STATE:
    {
        ESP_LOGI(TAG, "%d:%s,BLINK_OFF_STATE", __LINE__, __func__);
        switch (evt->sig)
        {
        case BSP_EVENT_SIG_LED_TIMEOUT:
        {
            bsp_RedLedOn();
            active_obj_TimeEventArm(&self->te, self->blinkTime, 0);
            self->state = BLINK_ON_STATE;
            break;
        }
        case BSP_EVENT_SIG_BTN_PRESSED:
        {
            bsp_GrnLedOn();
            self->blinkTime >>= 1; // shorten blink time by factor of 2
            if (self->blinkTime == 0U)
                self->blinkTime = INITIAL_BLINK_TIME;
            break;
        }
        case BSP_EVENT_SIG_BTN_RELEASED:
        {
            bsp_GrnLedOff();
            break;
        }
        default:
        {
            ESP_LOGE(TAG, "%d:%s,undefine event", __LINE__, __func__);
            assert(false);
            break;
        }
        }
        break;
    }
    case BLINK_ON_STATE:
    {
        ESP_LOGI(TAG, "%d:%s,BLINK_ON_STATE", __LINE__, __func__);
        switch (evt->sig)
        {
        case BSP_EVENT_SIG_LED_TIMEOUT:
        {
            bsp_RedLedOff();
            active_obj_TimeEventArm(&self->te, self->blinkTime, 0);
            self->state = BLINK_OFF_STATE;
            break;
        }
        case BSP_EVENT_SIG_BTN_PRESSED:
        {
            bsp_GrnLedOn();
            self->blinkTime >>= 1; // shorten blink time by factor of 2
            if (self->blinkTime == 0U)
                self->blinkTime = INITIAL_BLINK_TIME;
            break;
        }
        case BSP_EVENT_SIG_BTN_RELEASED:
        {
            bsp_GrnLedOff();
            break;
        }
        default:
        {
            ESP_LOGE(TAG, "%d:%s,undefine event", __LINE__, __func__);
            assert(false);
            break;
        }
        }
        break;
    }
    default:
    {
        ESP_LOGE(TAG, "%d:%s,undefine state", __LINE__, __func__);
        assert(false);
        break;
    }
    }

    // switch (evt->sig)
    // {
    // case ACTIVE_OBJ_RESV_SIGNALS_INIT:
    // case BSP_EVENT_SIG_LED_TIMEOUT:
    //     if (self->ledState)
    //     {
    //         bsp_RedLedOff();
    //         active_obj_TimeEventArm(&self->te, self->blinkTime, 0);
    //     }
    //     else
    //     {
    //         bsp_RedLedOn();
    //         active_obj_TimeEventArm(&self->te, self->blinkTime, 0);
    //     }
    //     break;
    // case BSP_EVENT_SIG_BTN_PRESSED:
    //     bsp_GrnLedOn();
    //     self->blinkTime >>= 1; // shorten blink time by factor of 2
    //     if (self->blinkTime == 0U)
    //         self->blinkTime = INITIAL_BLINK_TIME;
    //     break;
    // case BSP_EVENT_SIG_BTN_RELEASED:
    //     bsp_GrnLedOff();
    //     break;
    // default:
    //     break;
    // }
}

static void BlinkyButtonInit(BlinkyButton_t *const self)
{
    active_obj_Init((active_obj_instance_t *)self, BlinkyButtonEventDespatcher);
    active_obj_TimeEventInit(&self->te, BSP_EVENT_SIG_LED_TIMEOUT, &self->super);
    self->blinkTime = INITIAL_BLINK_TIME;
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
