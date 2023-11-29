#include "miramesh-integration-memory.h"
#include "miramesh-integration-thread.h"
#include "swi-callback-handler.h"
#include <stddef.h>
#include <string.h>
#include <zephyr/init.h>
#include <zephyr/devicetree.h>
#include <zephyr/devicetree/fixed-partitions.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/irq.h>
#include <zephyr/logging/log.h>
#include <miramesh.h>

LOG_MODULE_REGISTER(miramesh_integration, CONFIG_MIRAMESH_LOG_LEVEL);

#if CONFIG_MIRAMESH_RTC_ID == 2
#define RTC_ID 2
#define RTC_IRQn DT_IRQN(DT_NODELABEL(rtc2))
#else
#error "RTC ID not supported"
#endif

#if CONFIG_MIRAMESH_SWI_ID == 0
#define SWI_IRQn DT_IRQN(DT_NODELABEL(swi0))
#elif CONFIG_MIRAMESH_SWI_ID == 2
#define SWI_IRQn DT_IRQN(DT_NODELABEL(swi2))
#elif CONFIG_MIRAMESH_SWI_ID == 4
#define SWI_IRQn DT_IRQN(DT_NODELABEL(swi4))
#else
#error "SWI not supported"
#endif

#define FACTORY_CONFIG_PARTITION_NODE DT_NODELABEL(factory_config)
#define FACTORY_CONFIG_PARTITION_START DT_REG_ADDR(FACTORY_CONFIG_PARTITION_NODE)
#define FACTORY_CONFIG_PARTITION_LENGTH DT_REG_SIZE(FACTORY_CONFIG_PARTITION_NODE)

static void miramesh_integration_hardware_init(miramesh_hardware_cfg_t* hwconfig)
{

    /* Values may be updated to match the application in general */
    hwconfig->rtc = RTC_ID;
    hwconfig->rtc_irq_prio = CONFIG_MIRAMESH_RTC_IRQ_PRIO;
    hwconfig->swi = CONFIG_MIRAMESH_SWI_ID;
    hwconfig->swi_irq_prio = CONFIG_MIRAMESH_SWI_IRQ_PRIO;
}

ISR_DIRECT_DECLARE(rtc_miramesh_irq_handler)
{
    miramesh_rtc_irq_handler();
    return 0;
}

ISR_DIRECT_DECLARE(swi1_irq_handler)
{
    miramesh_swi1_irq_handler();
    return 0;
}

ISR_DIRECT_DECLARE(swi_miramesh_irq_handler)
{
    miramesh_swi_irq_handler();
    return 0;
}

static int32_t miramesh_integration_init(void)
{
    static miramesh_config_t miramesh_config;
    memset(&miramesh_config, 0, sizeof(miramesh_config_t));

    IRQ_DIRECT_CONNECT(RTC_IRQn, CONFIG_MIRAMESH_RTC_IRQ_PRIO, rtc_miramesh_irq_handler, 0);
    irq_enable(RTC_IRQn);
    IRQ_DIRECT_CONNECT(SWI1_IRQn, 2, swi1_irq_handler, 0);
    irq_enable(SWI1_IRQn);
    IRQ_DIRECT_CONNECT(SWI_IRQn, CONFIG_MIRAMESH_SWI_IRQ_PRIO, swi_miramesh_irq_handler, 0);
    irq_enable(SWI_IRQn);
    swi_callback_handler_init();
    miramesh_integration_thread_init(&miramesh_config);

    /*
     * Specify the memory address to the start and end
     * of the certificate area.
     * Use values corresponding to your memory layout.
     */
    miramesh_config.certificate.start = (uint8_t*)FACTORY_CONFIG_PARTITION_START;
    miramesh_config.certificate.end =
      (uint8_t*)(FACTORY_CONFIG_PARTITION_START + FACTORY_CONFIG_PARTITION_LENGTH);
    miramesh_integration_hardware_init(&miramesh_config.hardware);

    mira_status_t ret_val = miramesh_init(&miramesh_config, NULL);
    if (ret_val != MIRA_SUCCESS) {
        LOG_ERR("miramesh_init(): %d", ret_val);
        return -1;
    }
    miramesh_integration_memory_init();
    return 0;
}

SYS_INIT(miramesh_integration_init, APPLICATION, CONFIG_MIRAMESH_INIT_PRIO);
