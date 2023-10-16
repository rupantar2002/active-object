#include "active_obj.h"
#include <esp_log.h>

static const char *TAG = __FILE__;

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
}

void active_obj_Start(active_obj_instance_t *const self)
{
    self->queue = xQueueCreate(10, sizeof(active_obj_event_t *));
    configASSERT(self->queue);

    xTaskCreate(ActiveObjEventLoop, "ActiveObjEventLoop", 1024 * 4, self, 1, &self->thread);
    configASSERT(self->thread);
}

void active_obj_PostEvent(active_obj_instance_t *const self, active_obj_event_t *event)
{
    if (xQueueSend(self->queue, &event, 0) != pdTRUE)
    {
        ESP_LOGE(TAG, "%d:%s,ailed to post event", __LINE__, __func__);
    }
    ESP_LOGI(TAG, "event posted id :%p", &event);
}

void active_obj_PostEventFromIsr(active_obj_instance_t *const self, active_obj_event_t *event, BaseType_t highPriorityTaskWoken)
{
    if (xQueueSendFromISR(self->queue, event, highPriorityTaskWoken) != pdTRUE)
        assert(false);
}
