#!/usr/bin/env python

"""
The root level configuration package. Detects the current platform, out of
Windows, macOS, and Linux, and calls the appropriate configurator file.
"""

import platform

import config_linux
import config_mac
import config_windows


if __name__ == "__main__":
    system = platform.system()
    if system == "Linux":
        config_linux.run()
    elif system == "Darwin":
        config_mac.run()
    elif system == "Windows":
        config_windows.run()
    else:
        print("Platform '{}' is not supported.".format(system))
