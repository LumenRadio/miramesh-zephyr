#ifndef MIRAMESH_INTEGRATION_H
#define MIRAMESH_INTEGRATION_H

#if CONFIG_MIRAMESH_BT_FEM

#include <sdc_hci_vs.h>

/**
 * @brief Setup the BLE-stack to activate MiraMesh's FEM settings.
 *
 * This configures the Softdevice Controller to ask MiraMesh
 * to set the FEM in bypass mode before using the radio.
 *
 * It needs to be called each time after scan/initiator/connection/advertising has started.
 *
 * @args type The type of BLE event to configure for.
 *
 * @returns 0 if it succeeds.
 */
int miramesh_integration_set_events_for_fem(
    enum sdc_hci_vs_set_event_start_task_handle_type type);

#endif

#endif
