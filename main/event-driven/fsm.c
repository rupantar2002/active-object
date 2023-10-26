#include "fsm.h"
#include "assert.h"

static const event_base_t ENTRY_EVENT = {.sig = EVENT_RESERVED_SIGNAL_ENTRY};
static const event_base_t EXIT_EVENT = {.sig = EVENT_RESERVED_SIGNAL_EXIT};

void fsm_Init(fsm_base_t *const self, fsm_stateHandlerFun_t initial)
{
    self->state = initial;
}

void fsm_InitialTransition(fsm_base_t *const self, event_base_t const *const event)
{
    assert(self->state != (fsm_stateHandlerFun_t)0);
    (*self->state)(self, event);
    (*self->state)(self, &ENTRY_EVENT);
}

void fsm_Dispatch(fsm_base_t *const self, event_base_t const *const event)
{
    fsm_status_t status;
    fsm_stateHandlerFun_t prevState = self->state;
    assert(self->state != (fsm_stateHandlerFun_t)0);
    status = (*self->state)(self, event);

    if (status == FSM_STATUS_TRANSITION)
    {
        (*prevState)(self, &EXIT_EVENT);
        (*self->state)(self, &ENTRY_EVENT);
    }
}
