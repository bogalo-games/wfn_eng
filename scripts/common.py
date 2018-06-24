#!/usr/bin/env python

import os.path
import os


def root():
    """
    Wrapper around os.getcwd() in case we change project structure later.
    """

    return os.getcwd()


def lib_root():
    """
    Returns the root of the lib directory. Assumes that this is being run from
    the root wfn_eng directory.
    """

    return os.path.join(root(), 'lib')


def is_right_root():
    """
    Checks that the cwd of this script is the right working directory.
    """

    split_path = os.path.split(root())
    return split_path[len(split_path) - 1] == "wfn_eng"


def env_file_name():
    """
    Provides the absolute path to the configuration file.
    """

    return os.path.join(root(), "config.env")


def find_file(dir, file_name):
    """
    Given a root directory (usually cwd), search through the directory for the
    first occurrence of a file with the given name.
    """

    try:
        files = os.scandir(dir)
    except Exception:
        # TODO: Actually... handle the error
        return

    for file in files:
        if file.name == file_name:
            return os.path.join(dir, file.name)
        elif file.is_dir():
            f = find_file(
                os.path.join(dir, file.name),
                file_name
            )

            if isinstance(f, str):
                return f

    return None


def find_required_files(dir, required_files):
    """
    Given a set of required files (and their short names), return a map of
    their short name to their absolute location.
    """

    found_files = {}
    for k in required_files:
        found_files[k] = find_file(
            dir,
            required_files[k]
        )
    return found_files


def write_env_file(file_name, dict):
    """
    Given a simple dictionary (1 layer deep), representing a set of environment
    variables paried with their values, fill an environment file that can be
    used by wfn_eng.
    """

    f = open(file_name, "w")
    for k in dict:
        f.write("{}={}\n".format(k, dict[k]))
