#ifndef __ACTIVE_OBJ_H__
#define __ACTIVE_OBJ_H__
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

/*---------------------Event---------------------*/
typedef uint32_t active_obj_signal_t;

typedef enum active_obj_tagResvSignals
{
    ACTIVE_OBJ_RESV_SIGNALS_INIT,
    ACTIVE_OBJ_RESV_SIGNALS_USER,

} active_obj_resvSignals_t;

typedef struct active_obj_tagEvent
{
    active_obj_signal_t sig;
    /* event params will be added in sub classes*/
} active_obj_event_t;

/*-----------------------------------------------*/

/*--------------------Active Obj-----------------*/
typedef struct active_obj_tagIntance active_obj_instance_t;

typedef void (*active_obj_dispatchFun_t)(active_obj_instance_t *const self, active_obj_event_t const *const event);

struct active_obj_tagIntance
{
    TaskHandle_t thread;
    QueueHandle_t queue;
    active_obj_dispatchFun_t dispatch;
    /* private data  will be added in sub classes*/
};

void active_obj_Init(active_obj_instance_t *const self, active_obj_dispatchFun_t *dispatch);

void active_obj_Start(active_obj_instance_t *const self);

void active_obj_PostEvent(active_obj_instance_t *const self, active_obj_event_t *event);

void active_obj_PostEventFromIsr(active_obj_instance_t *const self, active_obj_event_t *event, BaseType_t highPriorityTaskWoken);

/*-----------------------------------------------*/

#endif //__ACTIVE_OBJ_H__