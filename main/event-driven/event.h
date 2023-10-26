#ifndef __EVENT_H__
#define __EVENT_H__

#include <stdint.h>

/**
 * @brief Event signal.
 *
 */
typedef uint16_t event_signal_t;

/**
 * @brief Event Reserved signals
 *
 */
typedef enum event_tagReservedSignal
{
    EVENT_RESERVED_SIGNAL_INIT,
    EVENT_RESERVED_SIGNAL_ENTRY,
    EVENT_RESERVED_SIGNAL_EXIT,
    EVENT_RESERVED_SIGNAL_USER,
} event_reservedSignal_t;

/**
 * @brief
 *
 */
typedef struct event_tagBase
{
    event_signal_t sig;
    /*...*/
} event_base_t;

#endif