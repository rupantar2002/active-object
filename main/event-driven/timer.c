#include "timer.h"
#include <freertos/FreeRTOS.h>
#include <esp_timer.h>
#include <esp_err.h>
#include <esp_log.h>

static const char *TAG = __FILE__;

static esp_timer_handle_t gTimerHandle = NULL;
static timer_event_t *gTimeEvtArray[TIMER_MAX_TIME_EVENT_CONT];
static uint_fast8_t gTimeEvtCount = 0;
static portMUX_TYPE gSpinLock = portMUX_INITIALIZER_UNLOCKED;

static void TimerEventCallback(void *args)
{
    // ESP_LOGI(TAG, "%d:%s", __LINE__, __func__);
    uint_fast8_t i;
    for (i = 0U; i < gTimeEvtCount; ++i)
    {
        timer_event_t *const te = gTimeEvtArray[i];
        if (te && (te->act != (active_base_t *)0))
        {
            if (te->timeout > 0) // time event armed
            {
                if ((--(te->timeout) == 0U))
                {
                    // ESP_LOGI(TAG, "event posted");
                    active_PostEvent(te->act, &te->super);
                    te->timeout = te->interval; // rearm or disarm
                }
            }
        }
    }
}

void timer_Start(void)
{
    const esp_timer_create_args_t args = {
        .arg = NULL,
        .callback = TimerEventCallback,
        .name = "timer",
        .dispatch_method = ESP_TIMER_TASK,
        .skip_unhandled_events = false,
    };
    ESP_ERROR_CHECK(esp_timer_create(&args, &gTimerHandle));
    ESP_ERROR_CHECK(esp_timer_start_periodic(gTimerHandle, (1000000U / TIMER_FREQ_IN_HZ))); // 10 ms timer resolution
}

void timer_Stop(void)
{
    ESP_ERROR_CHECK(esp_timer_stop(gTimerHandle));
}

void timer_InitEvent(timer_event_t *const self,
                     event_signal_t sig,
                     active_base_t *act)
{
    self->super.sig = sig;
    self->act = act;
    self->timeout = 0;
    self->interval = 0;

    taskENTER_CRITICAL(&gSpinLock);
    assert(gTimeEvtCount < sizeof(gTimeEvtArray) / sizeof(gTimeEvtArray[0]));
    gTimeEvtArray[gTimeEvtCount] = self;
    ++gTimeEvtCount;
    taskEXIT_CRITICAL(&gSpinLock);
}

void timer_ArmEvent(timer_event_t *const self,
                    uint32_t timeout,
                    uint32_t interval)
{
    taskENTER_CRITICAL(&gSpinLock);
    self->timeout = timeout;
    self->interval = interval;
    taskEXIT_CRITICAL(&gSpinLock);
}

void timer_DisarmEvent(timer_event_t *const self)
{
    taskENTER_CRITICAL(&gSpinLock);
    self->timeout = 0;
    taskEXIT_CRITICAL(&gSpinLock);
}