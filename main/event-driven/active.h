#ifndef __ACTIVE_H__
#define __ACTIVE_H__
#include "fsm.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

typedef struct active_tagBase
{
    fsm_base_t super;
    TaskHandle_t thread;
    QueueHandle_t queue;
    /*....*/
} active_base_t;

void active_Init(active_base_t *const self, fsm_stateHandlerFun_t initial);

void active_Start(active_base_t *const self);

void active_PostEvent(active_base_t *const self, event_base_t *event);

void active_PostEventFromIsr(active_base_t *const self, event_base_t *event, BaseType_t highPriorityTaskWoken);

/*---------------------TimeEvent----------------*/

/*----------------------------------------------*/

#endif //__ACTIVE_H__