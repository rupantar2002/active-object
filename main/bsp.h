#ifndef __BSP_H__
#define __BSP_H__

typedef enum
{
    BSP_EVENT_SIG_BTN_PRESSED = ACTIVE_OBJ_RESV_SIGNALS_USER,
    BSP_EVENT_SIG_BTN_RELEASED,
    BSP_EVENT_SIG_LED_TIMEOUT,
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
extern active_obj_instance_t *pgActiveButton;

#endif //__BSP_H__