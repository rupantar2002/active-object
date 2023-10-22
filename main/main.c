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
        /*.....*/
        MAX_STATE,
    } state;
    active_obj_timeEvent_t te;
    uint8_t btnCount;
} TimeBomb_t;

typedef enum
{
    TRAN_STATUS,
    HANDLED_STATUS,
    IGNORE_STATUS,
    INIT_STATUS,
} TimeBombStatus_t;

/**
 * @brief Time Bomb state transition function.
 *
 */
typedef TimeBombStatus_t (*TimeBombAction_t)(TimeBomb_t *const self, active_obj_event_t const *const evt);

/** -----------------------constants---------------------------------*/

static const char *TAG = __FILE__;

static TimeBombStatus_t TimeBomb_Init(TimeBomb_t *const self, active_obj_event_t const *const evt);

static TimeBombStatus_t TimeBomb_Ignore(TimeBomb_t *const self, active_obj_event_t const *const evt);

static TimeBombStatus_t TimeBomb_WaitForBtn_Entry(TimeBomb_t *const self, active_obj_event_t const *const evt);

static TimeBombStatus_t TimeBomb_WaitForBtn_Exit(TimeBomb_t *const self, active_obj_event_t const *const evt);

static TimeBombStatus_t TimeBomb_WaitForBtn_Pressed(TimeBomb_t *const self, active_obj_event_t const *const evt);

static TimeBombStatus_t TimeBomb_Blink_Entry(TimeBomb_t *const self, active_obj_event_t const *const evt);

static TimeBombStatus_t TimeBomb_Blink_Exit(TimeBomb_t *const self, active_obj_event_t const *const evt);

static TimeBombStatus_t TimeBomb_Blink_Timeout(TimeBomb_t *const self, active_obj_event_t const *const evt);

static TimeBombStatus_t TimeBomb_Paused_Entry(TimeBomb_t *const self, active_obj_event_t const *const evt);

static TimeBombStatus_t TimeBomb_Paused_Timeout(TimeBomb_t *const self, active_obj_event_t const *const evt);

static TimeBombStatus_t TimeBomb_Boom_Entry(TimeBomb_t *const self, active_obj_event_t const *const evt);

static const TimeBombAction_t TIME_BOMB_TABLE[MAX_STATE][BSP_MAX_EVENT] = {
    /*                     |   INIT    |  | ENTRY  |     |  EXIT |          | PRESSED  |            |  RELEASED  |     |  TIMEOUT  | */
    /*  WAIT_FOR_BTN    */ {&TimeBomb_Init, &TimeBomb_WaitForBtn_Entry, &TimeBomb_WaitForBtn_Exit, &TimeBomb_WaitForBtn_Pressed, &TimeBomb_Ignore, &TimeBomb_Ignore},
    /*      BLINK       */ {&TimeBomb_Ignore, &TimeBomb_Blink_Entry, &TimeBomb_Blink_Exit, &TimeBomb_Ignore, &TimeBomb_Ignore, &TimeBomb_Blink_Timeout},
    /*     PAUSED       */ {&TimeBomb_Ignore, &TimeBomb_Paused_Entry, &TimeBomb_Ignore, &TimeBomb_Ignore, &TimeBomb_Ignore, &TimeBomb_Paused_Timeout},
    /*      BOOM        */ {&TimeBomb_Ignore, &TimeBomb_Boom_Entry, &TimeBomb_Ignore, &TimeBomb_Ignore},
};

/** -----------------private global---------------------------------*/
static TimeBomb_t gTimeBomb;

/** -----------------public global----------------------------------*/

/**
 * this pointer has a visibility scope outside of this translation
 * unit or simply c src file.the exact signature should be used carefully
 * when creating non static global variables in another translation unit or
 * c src file or header files.Redefinition of this variable could case unwanted
 * flow of code.
 *
 */
active_obj_instance_t *pgActiveButton = &gTimeBomb.super;

/*--------------------BUTTON----------------------------*/

static void TimeBombEventDespatcher(TimeBomb_t *const self,
                                    active_obj_event_t const *const evt)
{
    TimeBombStatus_t status;
    int prevState = self->state;
    configASSERT((self->state < MAX_STATE) && (evt->sig < BSP_MAX_EVENT));
    status = (*TIME_BOMB_TABLE[self->state][evt->sig])(self, evt);

    if (status == TRAN_STATUS)
    {
        (*TIME_BOMB_TABLE[prevState][ACTIVE_OBJ_RESV_SIGNALS_EXIT])(self, (active_obj_event_t *)0);
        (*TIME_BOMB_TABLE[self->state][ACTIVE_OBJ_RESV_SIGNALS_ENTRY])(self, (active_obj_event_t *)0);
    }
    else if (status == INIT_STATUS)
    {
        (*TIME_BOMB_TABLE[self->state][ACTIVE_OBJ_RESV_SIGNALS_ENTRY])(self, (active_obj_event_t *)0);
    }
}

static void TimeBombAoInit(TimeBomb_t *const self)
{
    active_obj_Init((active_obj_instance_t *)self, (active_obj_dispatchFun_t)&TimeBombEventDespatcher);
    active_obj_TimeEventInit(&self->te, BSP_EVENT_SIG_TIMEOUT, &self->super);
}
/*-------------------------------------------------------*/

void app_main(void)
{
    bsp_Init();
    bsp_Start();
    TimeBombAoInit((active_obj_instance_t *)&gTimeBomb);
    active_obj_Start((active_obj_instance_t *)&gTimeBomb);
}

static TimeBombStatus_t TimeBomb_Init(TimeBomb_t *const self, active_obj_event_t const *const evt)
{
    self->state = WAIT_FOR_BUTTON_STATE;
    return INIT_STATUS;
}

static TimeBombStatus_t TimeBomb_Ignore(TimeBomb_t *const self, active_obj_event_t const *const evt)
{
    return IGNORE_STATUS;
}

static TimeBombStatus_t TimeBomb_WaitForBtn_Entry(TimeBomb_t *const self, active_obj_event_t const *const evt)
{
    bsp_GrnLedOn();
    return HANDLED_STATUS;
}

static TimeBombStatus_t TimeBomb_WaitForBtn_Exit(TimeBomb_t *const self, active_obj_event_t const *const evt)
{
    bsp_GrnLedOff();
    return HANDLED_STATUS;
}

static TimeBombStatus_t TimeBomb_WaitForBtn_Pressed(TimeBomb_t *const self, active_obj_event_t const *const evt)
{
    self->btnCount = 5U;
    self->state = BLINK_STATE;
    return TRAN_STATUS;
}

static TimeBombStatus_t TimeBomb_Blink_Entry(TimeBomb_t *const self, active_obj_event_t const *const evt)
{
    bsp_RedLedOn();
    active_obj_TimeEventArm(&self->te, ACTIVE_OBJ_MS_TO_INTERVAL(500), 0);
    return HANDLED_STATUS;
}

static TimeBombStatus_t TimeBomb_Blink_Exit(TimeBomb_t *const self, active_obj_event_t const *const evt)
{
    bsp_RedLedOff();
    return HANDLED_STATUS;
}

static TimeBombStatus_t TimeBomb_Blink_Timeout(TimeBomb_t *const self, active_obj_event_t const *const evt)
{
    self->state = PAUSED_STATE;
    return TRAN_STATUS;
}

static TimeBombStatus_t TimeBomb_Paused_Entry(TimeBomb_t *const self, active_obj_event_t const *const evt)
{
    active_obj_TimeEventArm(&self->te, ACTIVE_OBJ_MS_TO_INTERVAL(500), 0);
    return HANDLED_STATUS;
}

static TimeBombStatus_t TimeBomb_Paused_Timeout(TimeBomb_t *const self, active_obj_event_t const *const evt)
{
    --self->btnCount;
    if (self->btnCount > 0)
        self->state = BLINK_STATE;
    else
        self->state = BOOM_STATE;
    return TRAN_STATUS;
}

static TimeBombStatus_t TimeBomb_Boom_Entry(TimeBomb_t *const self, active_obj_event_t const *const evt)
{
    bsp_GrnLedOn();
    bsp_RedLedOn();
    ESP_LOGE(TAG, "\r\n<------------BOMB SUCCESSFULLY DETONATED-------------->\r\n");
    return HANDLED_STATUS;
}
