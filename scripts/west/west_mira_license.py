"""west_mira_license.py

West extension to flash mira license."""

import os
import subprocess

from textwrap import dedent

from west.commands import WestCommand

import helper


class MiraLicense(WestCommand):

    def __init__(self):
        super().__init__(
            "mira-license",
            "Flash Mira license to target device",
            dedent(
                """Automate the licensing of the target device according to the
            built target."""
            ),
        )

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(
            self.name, help=self.help, description=self.description
        )
        parser.add_argument(
            "-d",
            "--build-dir-name",
            default="build",
            help="Name of the build directory (default: %(default)s)",
        )
        parser.add_argument(
            "-l",
            "--libmira-dir",
            default=os.path.join(os.getcwd(), "vendor", "libmira"),
            help="Extracted location of Libmira (default: %(default)s)",
        )
        parser.add_argument(
            "-s",
            "--serial-number",
            help="Serial number of target debugger",
            required=True,
        )
        parser.add_argument(
            "-I",
            "--lic-file",
            help="Extracted location of Libmira (default: %(default)s)",
            required=True,
        )

        return parser

    def do_run(self, args, unknown_args):
        build_dir = os.path.join(os.getcwd(), args.build_dir_name)
        if not os.path.isdir(build_dir):
            raise FileNotFoundError(
                f"Build directory '{build_dir}' not found!"
            )

        if not os.path.isdir(args.libmira_dir):
            raise FileNotFoundError(
                f"Libmira directory '{args.libmira_dir}' not found!"
            )

        license_python_file = os.path.join(
            args.libmira_dir, "tools", "mira_license.py"
        )
        if not os.path.isfile(license_python_file):
            raise FileNotFoundError(
                f"Mira License file '{license_python_file}' not found!"
            )

        license_requirements_file = os.path.join(
            args.libmira_dir, "tools", "requirements.txt"
        )
        if not os.path.isfile(license_requirements_file):
            raise FileNotFoundError(
                f"Requirements file '{license_requirements_file}' not found!"
            )

        partitions = helper.get_partitions_info(build_dir)
        factory_config_addr = hex(int(partitions["factory_config"]["address"]))
        factory_config_len = hex(int(partitions["factory_config"]["size"]))
        device_type = helper.get_target_device_type(build_dir)
        programmer = helper.get_default_programmer_for_device(device_type)

        venv_dir = "venv_license"

        commands = [
            f"python -m venv {venv_dir}",
            f"source {venv_dir}/bin/activate",
            f"pip install -r {license_requirements_file}",
            (
                f"{license_python_file} license -s {args.serial_number} -P {programmer} "
                f"-T {device_type} -a {factory_config_addr} -l {factory_config_len} "
                f"-I {args.lic_file}"
            ),
            "deactivate",
        ]

        subprocess.run(
            " && ".join(commands),
            shell=True,
            executable="/bin/bash",
            check=True,
        )
