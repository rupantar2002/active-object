#include "event-driven/active.h"
#include "event-driven/timer.h"
#include "bsp.h"
#include <esp_log.h>

static const char *TAG = __FILE__;

/*-----------------------------TIME BOMB -------------------------------*/
#define INITIAL_BLINK_TIME (ACTIVE_OBJ_TIMEOUT_FREQ_HZ / 4);

/**
 * @brief TimeBomb.
 *
 */
typedef struct
{
    active_base_t super;
    timer_event_t te;
    uint8_t btnCount;
} timeBomb_t;

fsm_status_t TimeBomb_Initial(timeBomb_t *const self, event_base_t const *const evt);

fsm_status_t TimeBomb_WaitForBtn(timeBomb_t *const self, event_base_t const *const evt);

fsm_status_t TimeBomb_Blink(timeBomb_t *const self, event_base_t const *const evt);

fsm_status_t TimeBomb_Paused(timeBomb_t *const self, event_base_t const *const evt);

fsm_status_t TimeBomb_Boom(timeBomb_t *const self, event_base_t const *const evt);

fsm_status_t TimeBomb_Initial(timeBomb_t *const self, event_base_t const *const evt)
{
    ESP_LOGI(TAG, "%d:%s", __LINE__, __func__);
    return FSM_TRANSITION(TimeBomb_WaitForBtn);
}

fsm_status_t TimeBomb_WaitForBtn(timeBomb_t *const self, event_base_t const *const evt)
{
    ESP_LOGI(TAG, "%d:%s", __LINE__, __func__);
    fsm_status_t state;
    switch (evt->sig)
    {
    case EVENT_RESERVED_SIGNAL_ENTRY:
    {
        bsp_GrnLedOn();
        state = FSM_STATUS_HANDLED;
        break;
    }
    case EVENT_RESERVED_SIGNAL_EXIT:
    {
        bsp_GrnLedOff();
        state = FSM_STATUS_HANDLED;
        break;
    }
    case BSP_EVENT_SIG_BTN_PRESSED:
    {
        self->btnCount = 5U;
        state = FSM_TRANSITION(TimeBomb_Blink);
        break;
    }
    default:
    {
        state = FSM_STATUS_IGNORED;
        break;
    }
    }
    return state;
}

fsm_status_t TimeBomb_Blink(timeBomb_t *const self, event_base_t const *const evt)
{
    ESP_LOGI(TAG, "%d:%s", __LINE__, __func__);
    fsm_status_t state;
    switch (evt->sig)
    {
    case EVENT_RESERVED_SIGNAL_ENTRY:
    {
        bsp_RedLedOn();
        timer_ArmEvent(&self->te, TIMER_MS_TO_INTERVAL(500), 0);
        state = FSM_STATUS_HANDLED;
        break;
    }
    case EVENT_RESERVED_SIGNAL_EXIT:
    {
        bsp_RedLedOff();
        state = FSM_STATUS_HANDLED;
        break;
    }
    case BSP_EVENT_SIG_TIMEOUT:
    {
        state = FSM_TRANSITION(TimeBomb_Paused);
        break;
    }
    default:
    {
        state = FSM_STATUS_IGNORED;
        break;
    }
    }
    return state;
}

fsm_status_t TimeBomb_Paused(timeBomb_t *const self, event_base_t const *const evt)
{
    ESP_LOGI(TAG, "%d:%s", __LINE__, __func__);
    fsm_status_t state;
    switch (evt->sig)
    {
    case EVENT_RESERVED_SIGNAL_ENTRY:
    {
        timer_ArmEvent(&self->te, TIMER_MS_TO_INTERVAL(500), 0);
        state = FSM_STATUS_HANDLED;
        break;
    }
    case BSP_EVENT_SIG_TIMEOUT:
    {
        --self->btnCount;
        if (self->btnCount > 0)
            state = FSM_TRANSITION(TimeBomb_Blink);
        else
            state = FSM_TRANSITION(TimeBomb_Boom);
        break;
    }
    default:
    {
        state = FSM_STATUS_IGNORED;
        break;
    }
    }
    return state;
}

fsm_status_t TimeBomb_Boom(timeBomb_t *const self, event_base_t const *const evt)
{
    ESP_LOGI(TAG, "%d:%s", __LINE__, __func__);
    fsm_status_t state;
    switch (evt->sig)
    {
    case EVENT_RESERVED_SIGNAL_ENTRY:
    {
        bsp_GrnLedOn();
        bsp_RedLedOn();
        ESP_LOGE(TAG, "\r\n<------------BOMB SUCCESSFULLY DETONATED-------------->\r\n");
        state = FSM_STATUS_HANDLED;
        break;
    }
    default:
    {
        state = FSM_STATUS_IGNORED;
        break;
    }
    }
    return state;
}

/** -----------------private global---------------------------------*/

static timeBomb_t gTimeBomb;

/** -----------------public global----------------------------------*/

/**
 * this pointer has a visibility scope outside of this translation
 * unit or simply c src file.the exact signature should be used carefully
 * when creating non static global variables in another translation unit or
 * c src file or header files.Redefinition of this variable could case unwanted
 * flow of code.
 *
 */
active_base_t *pgActiveButton = &gTimeBomb.super;

static void timeBomb_Init(timeBomb_t *const self)
{
    active_Init(&self->super, (fsm_stateHandlerFun_t)&TimeBomb_Initial);
    timer_InitEvent(&self->te, BSP_EVENT_SIG_TIMEOUT, &self->super);
}

static void timeBomb_Start(timeBomb_t *const self)
{
    active_Start(&self->super);
}

/*-----------------------------------------------------------------------------*/

void app_main(void)
{
    bsp_Init();
    bsp_Start();
    timer_Start();
    timeBomb_Init(&gTimeBomb);
    timeBomb_Start(&gTimeBomb);
}
