# miramesh-zephyr

A Zephyr module integrating MiraMesh with [Zephyr](https://zephyrproject.org/)
on Nordic Semiconductor's MCUs.

This module depends on libmira, which must be specified via
the [CMake](https://cmake.org/) variable LIBMIRA_LOCATION, or located
at the default path '../../vendor/libmira', relative to this module.

This module is usually downloaded and installed with `west` (Zephyr's
meta-tool) into the Zephyr workspace.

An example of using this module is available here:
[MiraMesh zephyr network example](https://github.com/LumenRadio/miramesh-zephyr-network-example).

## Version tags

The repo is tagged with the combination of versions of MiraMesh and nRF Connect SDK like this:
miramesh-2.11.0-beta1-ncs-v3.0.1

The tags say which versions of MiraMesh and NCS that version of
the integration layer has been built for and tested with.

When the integration layer is updated, it will be tagged with the right versions after
the tests have passed.

Other combinations may work, but they have not (yet) been tested by LumenRadio.

# Known version limits

* An API needed by MiraMesh versions before 2.11.0 was removed in NCS v2.6.0, thus preventing interoperability.
* MiraMesh 2.11.0 requires APIs that were added to NCS v2.9.0.

## Tested versions

| MiraMesh  | nRF Connect SDK |
| --------- | --------------- |
| 2.9.0     | v2.5.0          |
| 2.10.0    | v2.5.1          |
| 2.11.0-beta1    | v3.0.1          |

The current code does not support all combinations above. Use the relevant tagged version.

## Configuration

### Factory config flash area definition

MiraMesh requires a flash page to contain per device specific factory
config data. A flash area called factory_config has to be defined
in either a
[devicetree overlay](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/build/dts/howtos.html#set-devicetree-overlays)
or in the
[partition manager](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/scripts/partition_manager/partition_manager.html)'s
yml file. If both are present, the information defined in the partition manager
yml file will take precedence.

Partition manager is easier to work with, but is a Nordic Semiconductor specific extension to Zephyr.

Examples:

Devicetree overlay (stored in `boards/<board>.overlay` in the example):
```
&flash0 {
    partitions {
        compatible = "fixed-partitions";
        factory_config: partition@FF000 {
          reg = <0xFF000 0x1000>;
        };
    };
};

```

Partition manager (`pm_static_<board>.yml` in applications):
```
factory_config:
  address: 0xff000
  end_address: 0x100000
  size: 0x1000
```

### MPSL Timeslot

Nordic Semiconductor's
[Multiprotocol Service Layer](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrfxlib/mpsl/README.html#mpsl)
(MPSL) is a set of libraries that allow multiple radio protocol share the radio.

MiraMesh uses one MPSL timeslot session, so the session count needs to
be set properly so that there is enough sessions available.

The session count configured via the `MPSL_TIMESLOT_SESSION_COUNT`
[Kconfig](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/build/kconfig/index.html)
symbol.

The [example](https://github.com/LumenRadio/miramesh-zephyr-network-example).
uses two, one for MiraMesh and one for BLE.

### Front End Module support

If using MiraMesh with a FEM, it should be configured in the factory-config
area together with the license. See Mira's documentation for how to do that.

If NCS/Zephyr supports the FEM, it should also be configured in Zephyr.
See Nordic's documentation for how to do that.

If the FEM isn't supported by NCS/Zephyr, the `MIRAMESH_BT_FEM` Kconfig
option should be turned on. That makes the `miramesh_integration_set_events_for_fem`
function available. That function should be called once per value in
the enum `sdc_hci_vs_set_event_start_task_handle_type`, but only after the first time BLE has been used.

### MiraMesh resource availability verification

MiraMesh requires a certain amount of CPU time to be able to function
properly and not be delayed too long when it needs the CPU.

There is a module available that logs many of MiraMesh's performance
counters. Those can be examined to see that MiraMesh is working properly.

By enabling Kconfig symbol `MIRAMESH_INTEGRATION_VERIFICATION_LOG`, that logging is
turned on.

A good rule of thumb is that the following metrics should be zero or
close to zero: `TX dropped`, `TX failed`, `Missed RX slots`.

`TX dropped` means a packet was dropped before it was sent, most often because
the application tries to send more than the bandwidth allows.

`TX failed` and `Missed RX slots` indicates that the CPU or radio wasn't
available in time. It is normal that it sometimes happen and it is more common
if another radio protocol is used together with MiraMesh.

For even more detailed information, there are two additional log options
available:

* `CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_PACKETS`
    Logs the amount of packets sent and received.

* `CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_EVENTS`
    Logs events from the MAC layer.


## Migration Guide: from NCS v2.5.0 to v3.0.1

Hardware configuration for the integration layer has been moved to the device tree.

The following Kconfig options are no longer supported:
- `CONFIG_MIRAMESH_RTC_ID`
- `CONFIG_MIRAMESH_RTC_IRQ_PRIO`
- `CONFIG_MIRAMESH_SWI_ID`
- `CONFIG_MIRAMESH_SWI_IRQ_PRIO`
- `CONFIG_SWI_CALLBACK_HANDLER_IRQ_PRIO`

To configure the equivalent parameters using the device tree, refer to the example
application's device tree overlay file.
