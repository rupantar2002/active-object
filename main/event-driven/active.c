#include "active.h"
#include <esp_log.h>

#define active_MAX_TIME_EVT (10)

static const char *TAG = __FILE__;

static void ActiveObjEventLoop(void *pvParams)
{

    active_base_t *self = (active_base_t *)pvParams;
    configASSERT(self);

    fsm_Dispatch(&self->super, (event_base_t *)0);

    while (1)
    {
        event_base_t *event;
        xQueueReceive(self->queue, &event, portMAX_DELAY);
        fsm_Dispatch(&self->super, event);
    }
}

void active_Init(active_base_t *const self, fsm_stateHandlerFun_t initial)
{
    fsm_Init(&self->super, initial);
}

void active_Start(active_base_t *const self)
{
    self->queue = xQueueCreate(10, sizeof(event_base_t *));
    configASSERT(self->queue);

    xTaskCreate(ActiveObjEventLoop, "ActiveObjEventLoop", 1024 * 4, self, 5, &self->thread);
    configASSERT(self->thread);
}

void active_PostEvent(active_base_t *const self, event_base_t *event)
{
    if (xQueueSend(self->queue, &event, 0) != pdTRUE)
        ESP_LOGE(TAG, "%d:%s,failed to post event", __LINE__, __func__);
    // ESP_LOGI(TAG, "event posted id :%p", &event);
}

void active_PostEventFromIsr(active_base_t *const self, event_base_t *event, BaseType_t highPriorityTaskWoken)
{
    if (xQueueSendFromISR(self->queue, event, highPriorityTaskWoken) != pdTRUE)
        assert(false);
}
