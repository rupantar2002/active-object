#ifndef __TIMER_H__
#define __TIMER_H__

#include "active.h"

#define TIMER_MAX_TIME_EVENT_CONT (10)

#define TIMER_FREQ_IN_HZ (100)

typedef struct timer_tagEvent
{
    event_base_t super;
    active_base_t *act;
    uint32_t timeout;
    uint32_t interval;
} timer_event_t;

void timer_Start(void);

void timer_Stop(void);

void timer_InitEvent(timer_event_t *const self,
                     event_signal_t sig,
                     active_base_t *act);

void timer_ArmEvent(timer_event_t *const self,
                    uint32_t timeout,
                    uint32_t interval);

void timer_DisarmEvent(timer_event_t *const self);

#define TIMER_MS_TO_INTERVAL(ms) (((ms) * (TIMER_FREQ_IN_HZ)) / 1000U)

#endif //__ACTIVE_TIMER_H__