#ifndef __BSP_H__
#define __BSP_H__

typedef enum
{
    BSP_EVENT_SIG_BTN_PRESSED = EVENT_RESERVED_SIGNAL_USER,
    BSP_EVENT_SIG_BTN_RELEASED,
    BSP_EVENT_SIG_TIMEOUT,
    /*.....*/
    BSP_MAX_EVENT,
} bsp_eventSig_t;

void bsp_Init(void);

void bsp_Start(void);

void bsp_RedLedOn(void);

void bsp_RedLedOff(void);

void bsp_GrnLedOn(void);

void bsp_GrnLedOff(void);

/**
 * Pointer to a button active object instance,should defined in
 * button active despatcher implemention module,in this case
 * main.c file.
 */
extern active_base_t *pgActiveButton;

#endif //__BSP_H__