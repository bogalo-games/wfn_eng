#!/usr/bin/env python

"""
Configuration file for macOS platform.
"""


import common


"""
A set of required files for this configration.
"""
required_files = {
    "VULKAN_LIB": "libvulkan.1.dylib",
    "MOLTENVK_ICD": "MoltenVK_icd.json",
    "EXPLICIT_LAYER": "explicit_layer.d",
}


def run():
    common.write_env_file(
        common.env_file_name(),
        common.find_required_files(
            common.lib_root(),
            required_files
        )
    )


if __name__ == "__main__":
    run()
