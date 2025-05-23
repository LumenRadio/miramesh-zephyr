#include "miramesh-integration.h"
#include "miramesh-integration-memory.h"
#include "miramesh-integration-thread.h"
#include "swi-callback-handler.h"
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <zephyr/init.h>
#include <zephyr/devicetree.h>
#include <zephyr/devicetree/fixed-partitions.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/irq.h>
#include <zephyr/logging/log.h>
#include <miramesh.h>

#if defined(CONFIG_SOC_SERIES_NRF54LX)
#include <mpsl.h>
#include <nrfx_dppi.h>
#include <nrfx_ppib.h>
#include <zephyr/drivers/timer/nrf_grtc_timer.h>
#endif

#if defined(CONFIG_SOC_SERIES_NRF52X)
#include <nrfx_ppi.h>
#endif

#if USE_PARTITION_MANAGER
#include <zephyr/storage/flash_map.h>
#endif
LOG_MODULE_REGISTER(miramesh_integration, CONFIG_MIRAMESH_LOG_LEVEL);

#if USE_PARTITION_MANAGER
#define FACTORY_CONFIG_PARTITION_START FIXED_PARTITION_OFFSET(FACTORY_CONFIG)
#define FACTORY_CONFIG_PARTITION_LENGTH FIXED_PARTITION_SIZE(FACTORY_CONFIG)
#else
#define FACTORY_CONFIG_PARTITION_NODE DT_NODELABEL(factory_config)
#define FACTORY_CONFIG_PARTITION_START DT_REG_ADDR(FACTORY_CONFIG_PARTITION_NODE)
#define FACTORY_CONFIG_PARTITION_LENGTH DT_REG_SIZE(FACTORY_CONFIG_PARTITION_NODE)
#endif /* USE_PARTITION_MANAGER */

#if defined(CONFIG_SOC_SERIES_NRF52X)
#define SWI_NODE DT_NODELABEL(miramesh_swi)
#define MIRAMESH_SWI_PERIPHERAL_ID \
    ((DT_REG_ADDR(SWI_NODE) - DT_REG_ADDR(DT_NODELABEL(swi0))) / 0x1000)

/* Only RTC2 is supported */
#define RTC_NODE DT_NODELABEL(rtc2)
#define MIRAMESH_RTC_PERIPHERAL_ID 2

#elif defined(CONFIG_SOC_SERIES_NRF54LX)
#define GRTC_NODE DT_NODELABEL(grtc)
#define EGU10_NODE DT_NODELABEL(egu10)

static nrfx_dppi_t dppi10 = NRFX_DPPI_INSTANCE(10);
static nrfx_dppi_t dppi20 = NRFX_DPPI_INSTANCE(20);
static nrfx_ppib_interconnect_t ppib11_21 = NRFX_PPIB_INTERCONNECT_INSTANCE(11, 21);

#endif

static void miramesh_integration_hardware_init(
    miramesh_hardware_cfg_t *hwconfig)
{

#if defined(CONFIG_SOC_SERIES_NRF52X)
    /* Values may be updated to match the application in general */
    hwconfig->rtc = MIRAMESH_RTC_PERIPHERAL_ID;
    hwconfig->rtc_irq_prio = DT_IRQ(RTC_NODE, priority);
    hwconfig->swi = MIRAMESH_SWI_PERIPHERAL_ID;
    hwconfig->swi_irq_prio = DT_IRQ(SWI_NODE, priority);
    nrfx_err_t ret;
    nrf_ppi_channel_t ppi_channel;
    for (int i = 0; i < MIRAMESH_SYS_NUM_PPIS_USED; ++i) {
        ret = nrfx_ppi_channel_alloc(&ppi_channel);
        if (ret != NRFX_SUCCESS) {
            LOG_ERR("nrfx_ppi_channel_alloc(&ppi_channel)) failed (%d)!\n",
                ret);
        }
        hwconfig->ppi_idx[i] = (uint8_t) ppi_channel;
    }

    nrf_ppi_channel_group_t ppi_group;
    for (int i = 0; i < MIRAMESH_SYS_NUM_PPI_GROUPS_USED; ++i) {
        ret = nrfx_ppi_group_alloc(&ppi_group);
        if (ret != NRFX_SUCCESS) {
            LOG_ERR("nrfx_ppi_group_alloc(&ppi_group) failed (%d)!\n",
                ret);
        }
        hwconfig->ppi_group_idx[i] = (uint8_t) ppi_group;
    }

#elif defined(CONFIG_SOC_SERIES_NRF54LX)
    uint8_t channel = 0;
    nrfx_err_t ret;

    for (int i = 0; i < MIRAMESH_SYS_NUM_10_DPPIS_USED; ++i) {
        do {
            ret = nrfx_dppi_channel_alloc(&dppi10, &channel);
            if (ret != NRFX_SUCCESS) {
                LOG_ERR("nrfx_dppi_channel_alloc(&dppi10, &channel) failed (%d)!\n",
                    ret);
            }
        } while ((1UL << channel) & MPSL_DPPIC10_CHANNELS_USED_MASK);

        hwconfig->dppi_10_idx[i] = channel;
    }

    channel = 0;
    for (int i = 0; i < MIRAMESH_SYS_NUM_20_DPPIS_USED; ++i) {

        do {
            ret = nrfx_dppi_channel_alloc(&dppi20, &channel);
            if (ret != NRFX_SUCCESS) {
                LOG_ERR("nrfx_dppi_channel_alloc(&dppi20, &channel) failed (%d)!\n",
                    ret);
            }
        } while ((1UL << channel) & MPSL_DPPIC20_CHANNELS_USED_MASK);

        hwconfig->dppi_20_idx[i] = channel;
    }

    nrf_dppi_channel_group_t group;
    for (int i = 0; i < MIRAMESH_SYS_NUM_10_DPPI_GROUPS_USED; ++i) {
        ret = nrfx_dppi_group_alloc(&dppi10, &group);
        if (ret != NRFX_SUCCESS) {
            LOG_ERR("nrfx_dppi_group_alloc(&dppi10, &group) failed (%d)!\n", ret);
        }

        hwconfig->dppi_group_10_idx[i] = group;
    }

    channel = 0;
    for (int i = 0; i < MIRAMESH_SYS_NUM_11_21_PPIB_CHANNELS_USED; ++i) {
        do {
            ret = nrfx_ppib_channel_alloc(&ppib11_21, &channel);
            if (ret != NRFX_SUCCESS) {
                LOG_ERR(
                    "nrfx_ppib_channel_alloc(&ppib11_21, &channel) failed (%d)!\n",
                    ret);
            }
        } while ((1UL << channel)
                 & (MPSL_PPIB11_CHANNELS_USED_MASK
                    || MPSL_PPIB21_CHANNELS_USED_MASK));

        hwconfig->ppib_11_21_idx[i] = channel;
    }

    hwconfig->grtc_cc_reg_idx[0] = (uint8_t) z_nrf_grtc_timer_chan_alloc();
    hwconfig->grtc_cc_reg_idx[1] = (uint8_t) z_nrf_grtc_timer_chan_alloc();

    hwconfig->grtc_irq = DT_IRQN(GRTC_NODE);
    hwconfig->grtc_irq_prio = DT_IRQ(GRTC_NODE, priority); /* Zephyr expects it to set to one */

    hwconfig->egu_event = DT_PROP(DT_PATH(zephyr_user), miramesh_egu_event);
    hwconfig->egu_irq_prio = DT_IRQ(EGU10_NODE, priority);
#endif
}

#if defined(CONFIG_SOC_SERIES_NRF52X)
ISR_DIRECT_DECLARE(rtc_miramesh_irq_handler)
{
    miramesh_rtc_irq_handler();
    return 1; /* Zephyr semaphores can be modified from this interrupt, tell Zephyr to reschedule after this interrupt */
}

ISR_DIRECT_DECLARE(swi1_irq_handler)
{
    miramesh_swi1_irq_handler();
    return 0; /* No need to reschedule after this interrupt */
}

ISR_DIRECT_DECLARE(swi_miramesh_irq_handler)
{
    miramesh_swi_irq_handler();
    return 0; /* No need to reschedule after this interrupt */
}

#elif defined(CONFIG_SOC_SERIES_NRF54LX)

ISR_DIRECT_DECLARE(egu10_miramesh_irq_handler)
{
    miramesh_egu_irq_handler();
    return 0;
}

#endif

static int32_t miramesh_integration_init(
    void)
{
    static miramesh_config_t miramesh_config;
    memset(&miramesh_config, 0, sizeof(miramesh_config_t));

#if defined(CONFIG_SOC_SERIES_NRF52X)
    IRQ_DIRECT_CONNECT(DT_IRQN(RTC_NODE),
        DT_IRQ(RTC_NODE, priority),
        rtc_miramesh_irq_handler, 0);
    IRQ_DIRECT_CONNECT(SWI1_EGU1_IRQn,
        2,
        swi1_irq_handler,
        0);
    IRQ_DIRECT_CONNECT(DT_IRQN(SWI_NODE),
        DT_IRQ(SWI_NODE, priority),
        swi_miramesh_irq_handler,
        0);
#elif defined(CONFIG_SOC_SERIES_NRF54LX)
    IRQ_CONNECT(
        DT_IRQN(GRTC_NODE),
        DT_IRQ(GRTC_NODE, priority),
        miramesh_grtc_irq_handler,
        NULL,
        0);
    IRQ_DIRECT_CONNECT(
        DT_IRQN(EGU10_NODE),
        DT_IRQ(EGU10_NODE, priority),
        egu10_miramesh_irq_handler,
        0);
#endif

    swi_callback_handler_init();
    miramesh_integration_thread_init(&miramesh_config);

    /*
     * Specify the memory address to the start and end
     * of the certificate area.
     * Use values corresponding to your memory layout.
     */
    miramesh_config.certificate.start = (uint8_t *) FACTORY_CONFIG_PARTITION_START;
    miramesh_config.certificate.end =
        (uint8_t *) (FACTORY_CONFIG_PARTITION_START
                     + FACTORY_CONFIG_PARTITION_LENGTH);
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

#if CONFIG_MIRAMESH_BT_FEM
#include <bluetooth/hci_vs_sdc.h>
#include <hal/nrf_egu.h>

int miramesh_integration_set_events_for_fem(
    enum sdc_hci_vs_set_event_start_task_handle_type type)
{
#if defined(CONFIG_SOC_SERIES_NRF52X)
    nrf_egu_int_enable(NRF_EGU1, NRF_EGU_INT_TRIGGERED0);
    NVIC_EnableIRQ(SWI1_EGU1_IRQn);
#elif defined(CONFIG_SOC_SERIES_NRF54LX)
    nrf_egu_int_enable((NRF_EGU_Type *) DT_REG_ADDR(EGU10_NODE),
        nrf_egu_channel_int_get(DT_PROP(DT_PATH(zephyr_user), miramesh_egu_event)));
    NVIC_EnableIRQ(DT_IRQN(EGU10_NODE));
#endif

    sdc_hci_cmd_vs_set_event_start_task_t cmd_params = {
        .handle_type = type,
        .task_address =
#if defined(CONFIG_SOC_SERIES_NRF52X)
            nrf_egu_task_address_get(NRF_EGU1, NRF_EGU_TASK_TRIGGER0),
#elif defined(CONFIG_SOC_SERIES_NRF54LX)
            nrf_egu_task_address_get((NRF_EGU_Type *) DT_REG_ADDR(EGU10_NODE),
                nrf_egu_trigger_task_get(DT_PROP(DT_PATH(zephyr_user),
                    miramesh_egu_event))),
#endif
    };
    return hci_vs_sdc_set_event_start_task(&cmd_params);
}

#endif
