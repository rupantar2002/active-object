#ifndef __FSM_H__
#define __FSM_H__

#include "event.h"

/**
 * @typedef typedef struct fsm_tagBase fsm_base_t.
 *
 */
typedef struct fsm_tagBase fsm_base_t;

/**
 * @typedef typedef enum fsm_tagStatus fsm_status_t.
 *
 */
typedef enum fsm_tagStatus fsm_status_t;

/**
 * @brief An enum represents all fsm states.
 *
 */
enum fsm_tagStatus
{
    FSM_STATUS_INIT,
    FSM_STATUS_HANDLED,
    FSM_STATUS_IGNORED,
    FSM_STATUS_TRANSITION,
};

/**
 * @brief Finite State Machine despatch function.
 *
 */
typedef fsm_status_t (*fsm_stateHandlerFun_t)(fsm_base_t *const self, event_base_t const *const event);

/**
 * @brief Finite State Machine base struct.
 *
 */
struct fsm_tagBase
{
    fsm_stateHandlerFun_t state;
    /*...*/
};

/**
 * @brief Macro used for state transition.
 * @param target state handler function pointer.
 *
 */
#define FSM_TRANSITION(target) (((fsm_base_t *)self)->state = (fsm_stateHandlerFun_t)target, FSM_STATUS_TRANSITION)

/**
 * @brief Initialize fsm.
 *
 * @param self pointer to fsm base.
 * @param initial function poinetr to state handler.
 */
void fsm_Init(fsm_base_t *const self, fsm_stateHandlerFun_t initial);

/**
 * @brief Execute initial transition.
 *
 * @param self self poineter.
 * @param event event pointer.
 */
void fsm_InitialTransition(fsm_base_t *const self, event_base_t const *const event);

/**
 * @brief Dispatch state transition and actions according to events.
 *
 * @param self self poineter.
 * @param event event pointer.
 */
void fsm_Dispatch(fsm_base_t *const self, event_base_t const *const event);

#endif //__FSM_H__