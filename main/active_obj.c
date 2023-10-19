#include "active_obj.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_err.h>

#define ACTIVE_OBJ_MAX_TIME_EVT (10)

static const char *TAG = __FILE__;

static esp_timer_handle_t gTimerHandle = NULL;
static active_obj_timeEvent_t *gTimeEvtArray[ACTIVE_OBJ_MAX_TIME_EVT];
static uint_fast8_t gTimeEvtCount = 0;
static portMUX_TYPE gSpinLock = portMUX_INITIALIZER_UNLOCKED;

static void TimerEventCallback(void *args)
{
    uint_fast8_t i;
    for (i = 0U; i < gTimeEvtCount; ++i)
    {
        active_obj_timeEvent_t *const t = gTimeEvtArray[i];
        if (t)
        {
            if (t->timeout > 0) // time event armed
            {
                if (--(t->timeout) == 0U)
                {
                    active_obj_PostEvent(t->act, &t->super);
                    t->timeout = t->interval; // rearm or disarm
                }
            }
        }
    }
}

static void ActiveObjEventLoop(void *pvParams)
{

    active_obj_instance_t *self = (active_obj_instance_t *)pvParams;
    configASSERT(self);

    static const active_obj_event_t initEvent = {
        .sig = ACTIVE_OBJ_RESV_SIGNALS_INIT,
    };

    self->dispatch(self, &initEvent);

    while (1)
    {
        active_obj_event_t *event;
        xQueueReceive(self->queue, &event, portMAX_DELAY);
        self->dispatch(self, event);
    }
}

void active_obj_Init(active_obj_instance_t *const self, active_obj_dispatchFun_t *dispatch)
{
    self->dispatch = dispatch;
    const esp_timer_create_args_t args = {
        .arg = NULL,
        .callback = TimerEventCallback,
        .name = "active obj timer",
        .dispatch_method = ESP_TIMER_TASK,
        .skip_unhandled_events = false,
    };

    ESP_ERROR_CHECK(esp_timer_create(&args, &gTimerHandle));
}

void active_obj_Start(active_obj_instance_t *const self)
{

    self->queue = xQueueCreate(10, sizeof(active_obj_event_t *));
    configASSERT(self->queue);

    xTaskCreate(ActiveObjEventLoop, "ActiveObjEventLoop", 1024 * 4, self, 1, &self->thread);
    configASSERT(self->thread);

    ESP_ERROR_CHECK(esp_timer_start_periodic(gTimerHandle, (1000000U / ACTIVE_OBJ_TIMEOUT_FREQ_HZ))); // 10 ms timer resolution
}

void active_obj_PostEvent(active_obj_instance_t *const self, active_obj_event_t *event)
{
    if (xQueueSend(self->queue, &event, 0) != pdTRUE)
        ESP_LOGE(TAG, "%d:%s,ailed to post event", __LINE__, __func__);
    // ESP_LOGI(TAG, "event posted id :%p", &event);
}

void active_obj_PostEventFromIsr(active_obj_instance_t *const self, active_obj_event_t *event, BaseType_t highPriorityTaskWoken)
{
    if (xQueueSendFromISR(self->queue, event, highPriorityTaskWoken) != pdTRUE)
        assert(false);
}

void active_obj_TimeEventInit(active_obj_timeEvent_t *const self,
                              active_obj_signal_t sig,
                              active_obj_instance_t *act)
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

void active_obj_TimeEventArm(active_obj_timeEvent_t *const self,
                             uint32_t timeout,
                             uint32_t interval)
{
    taskENTER_CRITICAL(&gSpinLock);
    self->timeout = timeout;
    self->interval = interval;
    taskEXIT_CRITICAL(&gSpinLock);
}

void active_obj_TimeEventDisarm(active_obj_timeEvent_t *const self)
{
    taskENTER_CRITICAL(&gSpinLock);
    self->timeout = 0;
    taskEXIT_CRITICAL(&gSpinLock);
}
