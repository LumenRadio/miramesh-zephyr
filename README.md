# miramesh-zephyr

A Zephyr module integrating MiraMesh with Zephyr. This module depends on libmira, which must be specified via the CMake variable LIBMIRA_LOCATION, or located at the default path '../../vendor/libmira', relative to this module.

For usage examples, see [MiraMesh zephyr network example](https://github.com/LumenRadio/miramesh-zephyr-network-example).

## Tested versions

| MiraMesh  | nRF Connect SDK |
| --------- | --------------- |
| 2.9.0     | v2.5.0          |

## Configuration

### Factory config flash area definition
MiraMesh requires a hardware bound factory config data to be flashed to the device. A flash area called factory_config have to be defined in either a devicetree overlay or in the partition manager yml file. If both are present, the information defined in the partition manager yml file will take precedence.

Examples:  

Devicetree overlay
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

Partition manager
```
factory_config:
  address: 0xff000
  end_address: 0x100000
  size: 0x1000
```

### MPSL Timeslot
MiraMesh uses one MPSL timeslot session internally, so the session count needs to be set properly so that there is enough sessions available.

### MiraMesh resource availability verification
MiraMesh requires a certain amount of runtime to be able to function properly, in this module there is a verification log module available. By enabling `MIRAMESH_INTEGRATION_VERIFICATION_LOG`, the device running will log metrics that can be used to determine if MiraMesh gets enough runtime.

A good rule of thumb is that the following metrics should be zero or close to zero: `TX dropped`, `TX failed`, `Missed RX slots`.

For even more detailed information, there are two additional log options to enable with Kconfig:

* `CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_PACKETS`
    Logs the amount of packets sent and received.

* `CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_EVENTS`
    Logs events from the MAC layer.
