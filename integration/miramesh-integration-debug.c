#include "miramesh-integration-debug.h"
#include <miramesh.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel/thread.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(miramesh_integration, CONFIG_MIRAMESH_LOG_LEVEL);

#ifdef CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG
extern const k_tid_t miramesh_integration_debug_log_thread_id;

#ifdef CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_EVENTS
#include <mira_diag_log.h>

typedef enum
{
#include "events.h.gen"
} events_t;

#define EVENT_VARS(evt)            \
    volatile uint32_t count_##evt; \
    uint32_t last_##evt

EVENT_VARS(EVT_NET_RF_SLOTS_CALL_END_FAILED);
EVENT_VARS(EVT_NET_RF_SLOTS_CALL_START_FAILED);
EVENT_VARS(EVT_NET_RF_SLOTS_DELAY_END_FAILED);
EVENT_VARS(EVT_NET_RF_SLOTS_DELAY_START_FAILED);
EVENT_VARS(EVT_NET_RF_SLOTS_EXTEND_FAILED);
EVENT_VARS(EVT_NET_RF_SLOTS_IRQ_INVALID);
EVENT_VARS(EVT_NET_RF_SLOTS_RX_END_FAILED);
EVENT_VARS(EVT_NET_RF_SLOTS_RX_START_FAILED);
EVENT_VARS(EVT_NET_RF_SLOTS_RX_POWER_END_FAILED);
EVENT_VARS(EVT_NET_RF_SLOTS_SLOT_DISCARD);
EVENT_VARS(EVT_NET_RF_SLOTS_SLOT_DROP);
EVENT_VARS(EVT_NET_RF_SLOTS_SLOT_END);
EVENT_VARS(EVT_NET_RF_SLOTS_SLOT_FAILED);
EVENT_VARS(EVT_NET_RF_SLOTS_SLOT_START);
EVENT_VARS(EVT_NET_RF_SLOTS_SYNC_FAILED);
EVENT_VARS(EVT_NET_RF_SLOTS_TX_END_FAILED);
EVENT_VARS(EVT_NET_RF_SLOTS_TX_START_FAILED);

static void evt_irq_callback(uint32_t event, va_list ap)
{

#define HANDLE_EVT(evt) \
    case evt:           \
        count_##evt++;  \
        break

    switch (event) {
        HANDLE_EVT(EVT_NET_RF_SLOTS_CALL_END_FAILED);
        HANDLE_EVT(EVT_NET_RF_SLOTS_CALL_START_FAILED);
        HANDLE_EVT(EVT_NET_RF_SLOTS_DELAY_END_FAILED);
        HANDLE_EVT(EVT_NET_RF_SLOTS_DELAY_START_FAILED);
        HANDLE_EVT(EVT_NET_RF_SLOTS_EXTEND_FAILED);
        HANDLE_EVT(EVT_NET_RF_SLOTS_IRQ_INVALID);
        HANDLE_EVT(EVT_NET_RF_SLOTS_RX_END_FAILED);
        HANDLE_EVT(EVT_NET_RF_SLOTS_RX_START_FAILED);
        HANDLE_EVT(EVT_NET_RF_SLOTS_RX_POWER_END_FAILED);
        HANDLE_EVT(EVT_NET_RF_SLOTS_SLOT_DISCARD);
        HANDLE_EVT(EVT_NET_RF_SLOTS_SLOT_DROP);
        HANDLE_EVT(EVT_NET_RF_SLOTS_SLOT_END);
        HANDLE_EVT(EVT_NET_RF_SLOTS_SLOT_FAILED);
        HANDLE_EVT(EVT_NET_RF_SLOTS_SLOT_START);
        HANDLE_EVT(EVT_NET_RF_SLOTS_SYNC_FAILED);
        HANDLE_EVT(EVT_NET_RF_SLOTS_TX_END_FAILED);
        HANDLE_EVT(EVT_NET_RF_SLOTS_TX_START_FAILED);
    }
#undef HANDLE_EVT
}

static bool always_true(void)
{
    return true;
}

static const mira_diag_log_callbacks_t evt_callbacks = {
    .is_debug_enabled = always_true,
    .is_info_enabled = always_true,
    .irq_log = {
        .debug = evt_irq_callback,
        .info = evt_irq_callback,
        .warn = evt_irq_callback,
        .fatal = evt_irq_callback,
    },
};
#endif /* CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_EVENTS */

static void log_mac_events(void)
{
#ifdef CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_EVENTS
#define HANDLE_EVT(evt)                                 \
    uint32_t tmp_##evt = count_##evt - last_##evt;      \
    last_##evt = count_##evt;                           \
    if (tmp_##evt != 0) {                               \
        LOG_DBG(#evt ":%lu", (unsigned long)tmp_##evt); \
    }
    HANDLE_EVT(EVT_NET_RF_SLOTS_CALL_END_FAILED);
    HANDLE_EVT(EVT_NET_RF_SLOTS_CALL_START_FAILED);
    HANDLE_EVT(EVT_NET_RF_SLOTS_DELAY_END_FAILED);
    HANDLE_EVT(EVT_NET_RF_SLOTS_DELAY_START_FAILED);
    HANDLE_EVT(EVT_NET_RF_SLOTS_EXTEND_FAILED);
    HANDLE_EVT(EVT_NET_RF_SLOTS_IRQ_INVALID);
    HANDLE_EVT(EVT_NET_RF_SLOTS_RX_END_FAILED);
    HANDLE_EVT(EVT_NET_RF_SLOTS_RX_START_FAILED);
    HANDLE_EVT(EVT_NET_RF_SLOTS_RX_POWER_END_FAILED);
    HANDLE_EVT(EVT_NET_RF_SLOTS_SLOT_DISCARD);
    HANDLE_EVT(EVT_NET_RF_SLOTS_SLOT_DROP);
    HANDLE_EVT(EVT_NET_RF_SLOTS_SLOT_FAILED);
    HANDLE_EVT(EVT_NET_RF_SLOTS_SYNC_FAILED);
    HANDLE_EVT(EVT_NET_RF_SLOTS_TX_END_FAILED);
    HANDLE_EVT(EVT_NET_RF_SLOTS_TX_START_FAILED);
#undef HANDLE_EVT
#endif /* CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_EVENTS */
}

static void log_mac_common_diagnostics(mira_diag_mac_statistics_t* stats)
{
    LOG_DBG("TX dropped: %u", stats->tx_dropped);
    LOG_DBG("TX failed: %u", stats->tx_failed);
    LOG_DBG("TX slots: %u", stats->tx_slots);
    LOG_DBG("Used TX queue %u", stats->used_tx_queue);
    LOG_DBG("Missed RX slots: %u", stats->rx_missed_slots);
    LOG_DBG("RX slots: %u", stats->rx_slots);
    LOG_DBG("Scan slots: %u", stats->scan_slots);
    LOG_DBG("Calibrate slots: %u", stats->calibrate_slots);
    LOG_DBG("Unavailable ms: %u", stats->unavailable_ms);
    LOG_DBG("Bytes sent: %u", stats->bytes_sent);
}

static void log_mac_packet_diagnostics(mira_diag_mac_statistics_t* stats)
{
#ifdef CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_PACKETS
    LOG_DBG("RX not for us: %u", stats->rx_not_for_us_packets);
    LOG_DBG("RX all llmc packets: %u", stats->rx_all_nodes_llmc_packets);
    LOG_DBG("RX unicast packets: %u", stats->rx_unicast_packets);
    LOG_DBG("RX custom llmc packets: %u", stats->rx_custom_llmc_packets);
    LOG_DBG("TX all llmc packets: %u", stats->tx_all_nodes_llmc_packets);
    LOG_DBG("TX unicast packets: %u", stats->tx_unicast_packets);
    LOG_DBG("TX custom llmc packets: %u", stats->tx_custom_llmc_packets);
#endif /* CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_PACKETS */
}

void miramesh_integration_debug_log(void)
{
    mira_diag_mac_statistics_t stats;
    mira_status_t status;
    while (!mira_net_is_init()) {
        k_sleep(K_SECONDS(1));
    }
#ifdef CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_EVENTS
    mira_diag_log_set_callbacks(&evt_callbacks);
#endif
    while (1) {
        status = mira_diag_mac_get_statistics(&stats);
        if (status != MIRA_SUCCESS) {
            LOG_ERR("mira_diag_get_statistics failed with value %d", status);
        } else {
            log_mac_common_diagnostics(&stats);
            log_mac_packet_diagnostics(&stats);
        }
        log_mac_events();
        k_sleep(K_SECONDS(CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_INTERVAL_SECONDS));
    }
}

K_THREAD_DEFINE(miramesh_integration_debug_log_thread_id,
                512,
                miramesh_integration_debug_log,
                NULL,
                NULL,
                NULL,
                10,
                0,
                0);
#endif /* CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG */