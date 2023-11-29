#include "nrf.h"
#include <zephyr/irq.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>

#include "swi-callback-handler.h"

LOG_MODULE_DECLARE(miramesh_integration, CONFIG_MIRAMESH_LOG_LEVEL);

static swi_callback_t registered_swi_callbacks[CONFIG_SWI_CALLBACK_LIST_SIZE];
static volatile int swi_callbacks_invoked[CONFIG_SWI_CALLBACK_LIST_SIZE];

ISR_DIRECT_DECLARE(swi_cb_isr)
{
    ISR_DIRECT_PM();
    for (int i = 0; i < CONFIG_SWI_CALLBACK_LIST_SIZE; ++i) {
        if (swi_callbacks_invoked[i]) {
            swi_callbacks_invoked[i] = 0;
            registered_swi_callbacks[i]();
        }
    }
    return 1;
}

void swi_callback_handler_init(void)
{
    for (int i = 0; i < CONFIG_SWI_CALLBACK_LIST_SIZE; ++i) {
        registered_swi_callbacks[i] = NULL;
        swi_callbacks_invoked[i] = 0;
    }

    IRQ_DIRECT_CONNECT(SWI3_EGU3_IRQn, CONFIG_SWI_CALLBACK_HANDLER_IRQ_PRIO, swi_cb_isr, 0);
    irq_enable(SWI3_EGU3_IRQn);
}

void invoke_swi_callback(swi_callback_t callback)
{
    for (int i = 0; i < CONFIG_SWI_CALLBACK_LIST_SIZE; ++i) {
        if (callback == registered_swi_callbacks[i]) {
            swi_callbacks_invoked[i] = 1;
            NVIC_SetPendingIRQ(SWI3_EGU3_IRQn);
            return;
        }
    }
}

int register_swi_callback(swi_callback_t callback)
{
    int free_callback_idx = -1;
    for (int i = 0; i < CONFIG_SWI_CALLBACK_LIST_SIZE; ++i) {
        /* Check if already is registered */
        if (callback == registered_swi_callbacks[i]) {
            return 0;
        }

        if (registered_swi_callbacks[i] == NULL && free_callback_idx == -1) {
            free_callback_idx = i;
        }
    }

    if (free_callback_idx == -1) {
        LOG_ERR("No room for SWI callback");
        return -1;
    }
    registered_swi_callbacks[free_callback_idx] = callback;
    return 0;
}
