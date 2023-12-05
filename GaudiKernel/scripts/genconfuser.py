#!/usr/bin/env python3
#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
"""
Generate _confDb.py files for ConfigurableUser classes.
"""

import logging
import os
import sys
import time
from glob import glob
from pprint import pformat

import GaudiKernel.ConfigurableDb
from GaudiKernel.ConfigurableDb import cfgDb

logging.VERBOSE = (logging.INFO + logging.DEBUG) // 2
logging.addLevelName(logging.VERBOSE, "VERBOSE")
logging.verbose = lambda msg, *args, **kwargs: logging.log(
    logging.VERBOSE, msg, *args, **kwargs
)


def _inheritsfrom(derived, basenames):
    """
    Check if any of the class names in 'basenames' is anywhere in the base
    classes of the class 'derived'.
    If 'derived' _is_ one in 'basenames', returns False.

    'basenames' can be a string or an iterable (of strings).
    """
    if isinstance(basenames, str):
        basenames = (basenames,)
    for b in derived.__bases__:
        if b.__name__ in basenames:
            return True
        else:
            if _inheritsfrom(b, basenames):
                return True
    return False


def loadConfigurableDb(build_dir=None, project_name=None):
    """
    Equivalent to GaudiKernel.ConfigurableDb.loadConfigurableDb(), but does a
    deep search and executes the '*.confdb' files instead of importing them.
    """
    log = GaudiKernel.ConfigurableDb.log
    from os.path import join as path_join

    # look for the confdb files in all the reasonable places
    #  - CMake builds
    confDbFiles = []
    for path in sys.path:
        confDbFiles += [
            f for f in glob(path_join(path, "*", "*.confdb")) if os.path.isfile(f)
        ]
    #  - new-style CMake builds (look for all .confdb in the build tree)
    if build_dir:
        for root, _, files in os.walk(build_dir):
            confDbFiles += [
                os.path.join(root, f) for f in files if f.endswith(".confdb")
            ]
    #  - used projects and local merged file
    pathlist = os.getenv("LD_LIBRARY_PATH", "").split(os.pathsep)
    for path in filter(os.path.isdir, pathlist):
        confDbFiles += [
            f
            for f in [
                path_join(path, f) for f in os.listdir(path) if f.endswith(".confdb")
            ]
        ]
    #  - get the list of ignored files
    ignored_files = set(os.environ.get("CONFIGURABLE_DB_IGNORE", "").split(","))
    #  - load the confdb files
    for confDb in set(confDbFiles):
        if confDb in ignored_files or (
            project_name
            and os.path.basename(confDb) == ("{}.confdb".format(project_name))
        ):
            # skip ignored files and the project's own confdb
            log.debug("\t-ignoring [%s]", confDb)
            # make sure we count also <project_name>.confdb for GaudiKernel
            ignored_files.add(confDb)
            continue
        log.debug("\t-loading [%s]...", confDb)
        try:
            cfgDb._loadModule(confDb)
        except Exception as err:
            # It may happen that the file is found but not completely
            # written, usually during parallel builds, but we do not care.
            log.warning("Could not load file [%s] !", confDb)
            log.warning("Reason: %s", err)
    # top up with the regular merged confDb (for the used projects)
    # (make sure GaudiKernel gets the right exclusions)
    os.environ["CONFIGURABLE_DB_IGNORE"] = ",".join(ignored_files)
    GaudiKernel.ConfigurableDb.loadConfigurableDb()


def getConfigurableUsers(modulename, root, mayNotExist=False):
    """
    Find in the module 'modulename' all the classes that derive from ConfigurableUser.
    Return the list of the names.
    The flag mayNotExist is used to choose the level of the logging message in case
    the requested module does not exist.
    """
    # remember the old system path
    oldpath = list(sys.path)
    # we need to hack the sys.path to add the first part of the module name after root
    moduleelements = modulename.split(".")
    if len(moduleelements) > 1:
        moddir = os.sep.join([root] + moduleelements[:-1])
    else:
        moddir = root
    # this is the name of the submodule to import
    shortmodname = moduleelements[-1]
    # check if the module file actually exists
    if not os.path.exists(os.path.join(moddir, shortmodname) + ".py"):
        msg = "Module %s does not exist" % modulename
        if mayNotExist:
            logging.verbose(msg)
        else:
            logging.error(msg)
        # no file -> do not try to import
        return []
    # prepend moddir to the path
    sys.path.insert(0, moddir)
    logging.verbose("sys.path prepended with %r", sys.path[0])

    logging.info("Looking for ConfigurableUser in %r", modulename)
    g, l = {}, {}
    try:
        logging.verbose("importing %s", shortmodname)
        exec("import %s as mod" % shortmodname, g, l)
    finally:
        # restore old sys.path
        logging.verbose("restoring old sys.path")
        sys.path = oldpath
    mod = l["mod"]
    if "__all__" in dir(mod) and mod.__all__:
        all = mod.__all__
    else:
        all = [n for n in dir(mod) if not n.startswith("_")]
    result = []
    for name in all:
        cfg = cfgDb.get(name)
        if cfg and cfg["module"] != modulename:
            # This name comes from another module
            logging.verbose("Object %r already found in module %r", name, cfg["module"])
            continue
        t = getattr(mod, name)
        if isinstance(t, type) and _inheritsfrom(
            t, ("ConfigurableUser", "SuperAlgorithm")
        ):
            result.append(name)
    logging.verbose("Found %r", result)
    return result


def main():
    from optparse import OptionParser

    parser = OptionParser(
        prog=os.path.basename(sys.argv[0]),
        usage="%prog [options] <PackageName> [<Module1> ...]",
    )
    parser.add_option(
        "-o",
        "--output",
        action="store",
        type="string",
        help="output file for confDb data [default = '../genConfDir/<PackageName>_user_confDb.py'].",
    )
    parser.add_option(
        "-r",
        "--root",
        action="store",
        type="string",
        help="root directory of the python modules [default = '../python'].",
    )
    parser.add_option(
        "-v", "--verbose", action="store_true", help="print some debugging information"
    )
    parser.add_option(
        "--debug", action="store_true", help="print more debugging information"
    )
    parser.add_option(
        "--build-dir",
        action="store",
        help="build directory where to look for .confdb files (search all subdirectories)",
    )
    parser.add_option(
        "--project-name",
        action="store",
        help="name of the current project (used to exclude spurious versions of the .confdb file for the current project)",
    )
    parser.set_defaults(root=os.path.join("..", "python"))

    opts, args = parser.parse_args()

    if opts.debug:
        log_level = logging.DEBUG
    elif opts.verbose:
        log_level = logging.VERBOSE
    else:
        log_level = logging.INFO if os.environ.get("VERBOSE") else logging.WARNING
    logging.basicConfig(
        format="%(levelname)s: %(message)s", stream=sys.stdout, level=log_level
    )

    if len(args) < 1:
        parser.error("PackageName is required")

    package_name = args.pop(0)

    usingConvention = False
    if not args:
        # use the conventional module name <package>.Configuration
        args = [package_name + ".Configuration"]
        usingConvention = True

    genConfDir = os.path.join("..", os.environ.get("CMTCONFIG", ""), "genConfDir")
    if not os.path.exists(genConfDir):
        genConfDir = os.path.join("..", "genConfDir")

    if not opts.output:
        outputfile = os.path.join(genConfDir, package_name + "_user.confdb")
    else:
        outputfile = opts.output

    # We can disable the error on missing configurables only if we can import Gaudi.Configurables
    # It must be done at this point because it may conflict with logging.basicConfig
    try:
        import Gaudi.Configurables

        Gaudi.Configurables.ignoreMissingConfigurables = True
    except ImportError:
        pass
    # load configurables database to avoid fake duplicates
    loadConfigurableDb(opts.build_dir, opts.project_name)
    # ensure that local configurables are in the database
    try:
        # Add the local python directories to the python path to be able to import the local
        # configurables
        sys.path.insert(0, genConfDir)
        sys.path.insert(0, os.path.join("..", "python"))
        localConfDb = os.path.join(genConfDir, package_name, package_name + ".confdb")
        if os.path.exists(localConfDb):
            cfgDb._loadModule(localConfDb)
            # Extend the search path of the package module to find the configurables
            package_module = __import__(package_name)
            package_module.__path__.insert(0, os.path.join(genConfDir, package_name))
    except Exception:
        pass  # ignore failures (not important)

    # Collecting ConfigurableUser specializations
    cus = {}
    for mod in args:
        lst = None
        try:
            lst = getConfigurableUsers(mod, root=opts.root, mayNotExist=usingConvention)
        except ImportError:
            import traceback

            logging.error(
                "Cannot import module %r:\n%s", mod, traceback.format_exc().rstrip()
            )  # I remove the trailing '\n'
            return 2
        if lst:
            cus[mod] = lst
            # Add the configurables to the database as fake entries to avoid duplicates
            for m in lst:
                cfgDb.add(configurable=m, package="None", module="None", lib="None")
        elif not usingConvention:
            logging.warning(
                "Specified module %r does not contain ConfigurableUser specializations",
                mod,
            )

    if cus:
        logging.info("ConfigurableUser found:\n%s", pformat(cus))
        # header
        output = """##  -*- ascii -*-
# db file automatically generated by %s on: %s
""" % (
            parser.prog,
            time.asctime(),
        )

        for mod in cus:
            for cu in cus[mod]:
                output += "%s %s %s\n" % (mod, "None", cu)

        # trailer
        output += "## %s\n" % package_name
    elif usingConvention:
        logging.info("No ConfigurableUser found")
        output = (
            "# db file automatically generated by %s on: %s\n"
            "# No ConfigurableUser specialization in %s\n"
        ) % (parser.prog, time.asctime(), package_name)
    else:
        logging.error("No ConfigurableUser specialization found")
        return 1

    # create the destination directory if not there
    output_dir = os.path.dirname(outputfile)
    try:
        logging.info("Creating directory %r", output_dir)
        os.makedirs(output_dir, 0o755)
    except OSError as err:
        import errno

        if err.errno == errno.EEXIST:
            # somebody already - perhaps concurrently - created that dir.
            pass
        else:
            raise

    # write output to file
    logging.verbose("Writing confDb data to %r", outputfile)
    open(outputfile, "w").write(output)
    return 0


if __name__ == "__main__":
    retcode = main()
    sys.exit(retcode)
