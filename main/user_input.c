#include "user_input.h"
#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <esp_timer.h>
#include <esp_err.h>
#include <esp_log.h>

#define BUTTON1_GPIO (GPIO_NUM_2)
#define SWITCH1_GPIO (GPIO_NUM_3)

#define BUTTON_TRIGGER_STATE (0)
#define BUTTON_IDLE_STATE (1)

typedef enum
{
    INPUT_TYPE_NONE = 0,
    INPUT_TYPE_BUTTON,
    INPUT_TYPE_SWITCH,
    INPUT_TYPE_ENCODER,
    INPUT_TYPE_MAX,
} inputType_t;

typedef enum
{
    INPUT_STATUS_FREE = -1,
    INPUT_STATUS_BUTTON1_PROCESSING = 0,
    INPUT_STATUS_SWITCH1_PROCESSING,
    INPUT_STATUS_MAX,
} inputStatus_t;

typedef struct
{
    gpio_num_t gpio;
    inputType_t type;
    inputStatus_t status;
} inputDescription_t;

__attribute__((__unused__)) static const char *TAG = "user_input";

static const inputDescription_t ARRAY_OF_INPUT_DESC[] = {
    {.gpio = BUTTON1_GPIO, .type = INPUT_TYPE_BUTTON, .status = INPUT_STATUS_BUTTON1_PROCESSING},
    {.gpio = SWITCH1_GPIO, .type = INPUT_TYPE_SWITCH, .status = INPUT_STATUS_SWITCH1_PROCESSING},
};

// Interrupt Shared Variables
static volatile inputStatus_t gInputStatus = INPUT_STATUS_FREE;
static volatile int gTotalEdgeCount = 0;
static volatile int gPreviousEdgeCount = 0;

static struct
{
    uint8_t buttonPressed;
    uint8_t encoderEdge;
    int64_t elapseTime;
    portMUX_TYPE *spinLock;
    esp_timer_handle_t timer;
} gInputObject;

static void IRAM_ATTR GlobalGpioIsr(void *args)
{
    inputStatus_t status = *((inputStatus_t *)args);

    if (gInputStatus == INPUT_STATUS_FREE)
    {
        portENTER_CRITICAL(gInputObject.spinLock);
        gInputStatus = status;
        gPreviousEdgeCount = -1;
        gTotalEdgeCount = 0;
        esp_timer_start_periodic(gInputObject.timer, 5000); // 10ms
        portEXIT_CRITICAL(gInputObject.spinLock);
    }
    else if (gInputStatus == status)
    {
        gTotalEdgeCount++;
    }
}

static void TimerCallback(void *args)
{
    if (gPreviousEdgeCount < 0)
    {
        gPreviousEdgeCount = gTotalEdgeCount;
    }
    else
    {
        if (gPreviousEdgeCount >= gTotalEdgeCount) // finish debounce
        {
            inputDescription_t desc = ARRAY_OF_INPUT_DESC[gInputStatus];
            //printf("gpio %d stats %d type %d\n", desc.gpio, desc.status, desc.type);

            switch (desc.type)
            {
            case INPUT_TYPE_BUTTON:
            {
                if (gpio_get_level(desc.gpio) == BUTTON_TRIGGER_STATE)
                {
                    gInputObject.elapseTime = 12;
                    gInputObject.buttonPressed = true;
                }
                else if (gpio_get_level(desc.gpio) == BUTTON_IDLE_STATE && gInputObject.buttonPressed)
                {
                    gInputObject.elapseTime = false;
                    gInputObject.buttonPressed = false;
                    // printf("button short press event %lld", gInputObject.elapseTime);
                    switch (desc.status)
                    {
                    case INPUT_STATUS_BUTTON1_PROCESSING:
                        user_input_Button1PressedCallback();
                        break;
                    default:
                        break;
                    }
                }
                else if (gInputObject.buttonPressed && (esp_timer_get_time() - gInputObject.elapseTime) >= 2000)
                {
                    gInputObject.elapseTime = false;
                    gInputObject.buttonPressed = false;
                    // printf("button long press event");
                }
            }
            break;
            case INPUT_TYPE_SWITCH:
            {
                uint8_t state = gpio_get_level(desc.gpio);
                switch (desc.status)
                {
                case INPUT_STATUS_SWITCH1_PROCESSING:
                    user_input_Switch1StateCallback(state);
                    break;
                default:
                    break;
                }
            }
            break;
            case INPUT_TYPE_ENCODER:
            {
            }
            break;
            default:
                break;
            }

            // exit timer
            portENTER_CRITICAL(gInputObject.spinLock);
            gInputStatus = INPUT_STATUS_FREE;
            esp_timer_stop(gInputObject.timer);
            portEXIT_CRITICAL(gInputObject.spinLock);
        }
        else
        {
            gPreviousEdgeCount = gTotalEdgeCount;
        }
    }
}

void user_input_Init()
{
    int i;
    // rest object
    memset(&gInputObject, 0, sizeof(gInputObject));

    // init spinlock
    gInputObject.spinLock = (portMUX_TYPE *)pvPortMalloc(sizeof(portMUX_TYPE));
    portMUX_INITIALIZE(gInputObject.spinLock);
    configASSERT(gInputObject.spinLock);

    // init timer
    static const esp_timer_create_args_t timerArgs = {
        .callback = TimerCallback,
        .name = "user_input_timer",
        .skip_unhandled_events = false,
        .dispatch_method = ESP_TIMER_TASK,
    };
    esp_timer_create(&timerArgs, &gInputObject.timer);

    // init gpios
    gpio_install_isr_service(0);
    for (i = 0; i < sizeof(ARRAY_OF_INPUT_DESC) / sizeof(ARRAY_OF_INPUT_DESC[0]); i++)
    {
        gpio_reset_pin(ARRAY_OF_INPUT_DESC[i].gpio);
        gpio_set_direction(ARRAY_OF_INPUT_DESC[i].gpio, GPIO_MODE_INPUT);
        gpio_set_pull_mode(ARRAY_OF_INPUT_DESC[i].gpio, GPIO_PULLUP_ONLY);
        gpio_set_intr_type(ARRAY_OF_INPUT_DESC[i].gpio, GPIO_INTR_ANYEDGE);
        gpio_isr_handler_add(ARRAY_OF_INPUT_DESC[i].gpio, GlobalGpioIsr, (void *)&ARRAY_OF_INPUT_DESC[i].status);
        gpio_intr_disable(ARRAY_OF_INPUT_DESC[i].gpio);
    }
}

void user_input_Start()
{
    int i;
    for (i = 0; i < (sizeof(ARRAY_OF_INPUT_DESC) / sizeof(ARRAY_OF_INPUT_DESC[0])); i++)
    {
        gpio_intr_enable(ARRAY_OF_INPUT_DESC[i].gpio);
    }
}

void user_input_Stop()
{
    int i;
    esp_timer_stop(gInputObject.timer);
    for (i = 0; i < (sizeof(ARRAY_OF_INPUT_DESC) / sizeof(ARRAY_OF_INPUT_DESC[0])); i++)
    {
        gpio_intr_disable(ARRAY_OF_INPUT_DESC[i].gpio);
    }
}

void user_input_deInit()
{
}

__attribute__((__weak__)) void user_input_Button1PressedCallback(void)
{
    ESP_LOGI(TAG, " %s: %d", __func__, __LINE__);
}

__attribute__((__weak__)) void user_input_Switch1StateCallback(uint8_t state)
{
    ESP_LOGI(TAG, " %s: %d", __func__, __LINE__);
}