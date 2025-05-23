import os
import re
import yaml


def parse_yaml(file_path):
    """Parses a YAML file and returns the data as a Python dictionary."""
    try:
        with open(file_path, "r", encoding="utf-8") as file:
            data = yaml.safe_load(file)
        return data
    except FileNotFoundError as exc:
        raise FileNotFoundError(
            f"Error: File '{file_path}' not found."
        ) from exc
    except yaml.YAMLError as exc:
        raise yaml.YAMLError("Error parsing YAML file") from exc


def get_partitions_info(build_dir):
    """Returns the partitions data as a Python dictionary."""

    partitions_file = os.path.join(build_dir, "partitions.yml")
    return parse_yaml(partitions_file)


def get_target_device_type(build_dir):
    """Get device type from Kconfig"""
    zephyr_config_file = os.path.join(build_dir, "zephyr", ".config")
    if not os.path.isfile(zephyr_config_file):
        raise FileNotFoundError(
            f"Zephyr config file '{zephyr_config_file}' not found!"
        )

    config_soc_pattern = re.compile(r'SB_CONFIG_SOC="(?P<soc>\S+)"')

    with open(zephyr_config_file, "r", encoding="utf-8") as file:
        for line in file:
            match = config_soc_pattern.match(line)
            if match:
                soc = match.group("soc").rstrip()
                return soc
        raise RuntimeError("Cannot find device type!")


def get_default_programmer_for_device(device_type):
    default_programmer_map = {
        "nrf52832": "nrfjprog",
        "nrf52840": "nrfjprog",
        "nrf54l05": "jlink",
        "nrf54l10": "jlink",
        "nrf54l15": "jlink",
    }

    if device_type in default_programmer_map:
        return default_programmer_map[device_type]

    raise KeyError(f"Device type '{device_type}' not supported.")
