#!/usr/bin/env python
"""
Generate _confDb.py files for ConfigurableUser classes.
"""

import os
import sys
import time
import logging
import GaudiKernel.ConfigurableDb

from pprint import pformat
from glob import glob
from GaudiKernel.ConfigurableDb import cfgDb

logging.VERBOSE = (logging.INFO + logging.DEBUG) / 2
logging.addLevelName(logging.VERBOSE, "VERBOSE")
logging.verbose = lambda msg, *args, **kwargs: \
    apply(logging.log, (logging.VERBOSE, msg) + args, kwargs)

def _inheritsfrom(derived, basename):
    """
    Check if the class name 'basename' is anywhere in the base classes of the
    class 'derived'.
    If 'derived' _is_ 'basename', returns False.
    """
    for b in derived.__bases__:
        if b.__name__ == basename:
            return True
        else:
            if _inheritsfrom(b, basename):
                return True
    return False

def loadConfigurableDb():
    '''
    Equivalent to GaudiKernel.ConfigurableDb.loadConfigurableDb(), but does a
    deep search and executes the '*_confDb.py' files instead of importing them.
    '''
    # find the '*_confDb.py' files that are not merged ones
    for p in sys.path:
        for f in [f for f in glob(os.path.join(p, '*', '*_confDb.py'))
                  if 'merged' not in f and os.path.isfile(f)]:
            logging.verbose('Loading %s', f)
            try:
                execfile(f, {}, {})
            except:
                # It may happen that the file is found but not completely
                # written, usually during parallel builds, but we do not care.
                pass
    # top up with the regular merged confDb (for the used projects)
    GaudiKernel.ConfigurableDb.loadConfigurableDb()

def getConfigurableUsers(modulename, root, mayNotExist = False):
    """
    Find in the module 'modulename' all the classes that derive from ConfigurableUser.
    Return the list of the names.
    The flag mayNotExist is used to choose the level of the logging message in case
    the requested module does not exist.
    """
    # remember the old system path
    oldpath = list(sys.path)
    # we need to hack the sys.path to add the first part of the module name after root
    moduleelements = modulename.split('.')
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
        exec "import %s as mod" % shortmodname in g, l
    finally:
        # restore old sys.path
        logging.verbose("restoring old sys.path")
        sys.path = oldpath
    mod = l["mod"]
    if "__all__" in dir(mod) and mod.__all__:
        all = mod.__all__
    else:
        all = [ n for n in dir(mod) if not n.startswith("_")]
    result = []
    for name in all:
        cfg = cfgDb.get(name)
        if cfg and cfg["module"] != modulename:
            # This name comes from another module
            logging.verbose("Object %r already found in module %r", name, cfg["module"])
            continue
        t = getattr(mod, name)
        if isinstance(t, type) and  _inheritsfrom(t, "ConfigurableUser"):
            result.append(name)
    logging.verbose("Found %r", result)
    return result

def main():
    from optparse import OptionParser
    parser = OptionParser(prog = os.path.basename(sys.argv[0]),
                          usage = "%prog [options] <PackageName> [<Module1> ...]")
    parser.add_option("-o", "--output", action="store", type="string",
                      help="output file for confDb data [default = '../genConf/<PackageName>_user_confDb.py'].")
    parser.add_option("-r", "--root", action="store", type="string",
                      help="root directory of the python modules [default = '../python'].")
    parser.add_option("-v", "--verbose", action="store_true",
                      help="print some debugging information")
    parser.add_option("--debug", action="store_true",
                      help="print more debugging information")
    parser.add_option("--lockerpath", action="store",
                      metavar = "DIRNAME",
                      help="directory where to find the module 'locker'")
    parser.set_defaults(root = os.path.join("..","python"))

    opts, args = parser.parse_args()

    if opts.debug:
        log_level = logging.DEBUG
    elif opts.verbose:
        log_level = logging.VERBOSE
    else:
        log_level = logging.INFO
    logging.basicConfig(format = "%(levelname)s: %(message)s",
                        stream = sys.stdout,
                        level = log_level)

    if len(args) < 1:
        parser.error("PackageName is required")

    package_name = args.pop(0)

    usingConvention = False
    if not args:
        # use the conventional module name <package>.Configuration
        args = [package_name + ".Configuration"]
        usingConvention = True

    genConfDir = os.path.join("..", os.environ.get("CMTCONFIG", ""), "genConf")
    if not os.path.exists(genConfDir):
        genConfDir = os.path.join("..", "genConf")

    if not opts.output:
        outputfile = os.path.join(genConfDir, package_name + '_user_confDb.py')
    else:
        outputfile = opts.output


    # The locking ensures that nobody tries to modify the python.zip file while
    # we read it.
    dbLock = None
    if "GAUDI_BUILD_LOCK" in os.environ:
        if opts.lockerpath:
            sys.path.append(opts.lockerpath)
        # Get the LockFile class from the locker module in GaudiPolicy or use a fake
        # factory.
        try:
            from locker import LockFile
        except ImportError:
            def LockFile(*args, **kwargs):
                return None
        # obtain the lock
        dbLock = LockFile(os.environ["GAUDI_BUILD_LOCK"], temporary =  True)

    # We can disable the error on missing configurables only if we can import Gaudi.Configurables
    # It must be done at this point because it may conflict with logging.basicConfig
    try:
        import Gaudi.Configurables
        Gaudi.Configurables.ignoreMissingConfigurables = True
    except:
        pass
    # load configurables database to avoid fake duplicates
    loadConfigurableDb()
    # ensure that local configurables are in the database
    try:
        # Add the local python directories to the python path to be able to import the local
        # configurables
        sys.path.insert(0, genConfDir)
        sys.path.insert(0, os.path.join("..", "python"))
        localConfDb = os.path.join(genConfDir, package_name, package_name + '_confDb.py')
        if os.path.exists(localConfDb):
            execfile(localConfDb, {}, {})
            # Extend the search path of the package module to find the configurables
            package_module = __import__(package_name)
            package_module.__path__.insert(0, os.path.join(genConfDir, package_name))
    except:
        pass # ignore failures (not important)
    del dbLock # Now we can let the others operate on the install area python directory

    # Collecting ConfigurableUser specializations
    cus = {}
    for mod in args:
        lst = None
        try:
            lst = getConfigurableUsers(mod, root = opts.root, mayNotExist = usingConvention)
        except ImportError:
            import traceback
            logging.error("Cannot import module %r:\n%s", mod,
                          traceback.format_exc().rstrip()) # I remove the trailing '\n'
            return 2
        if lst:
            cus[mod] = lst
            # Add the configurables to the database as fake entries to avoid duplicates
            for m in lst:
                cfgDb.add(configurable = m,
                          package = 'None',
                          module  = 'None',
                          lib     = 'None')
        elif not usingConvention:
            logging.warning("Specified module %r does not contain ConfigurableUser specializations", mod)

    if cus:
        logging.info("ConfigurableUser found:\n%s", pformat(cus))
        # header
        output = """##  -*- python -*-
# db file automatically generated by %s on: %s
## insulates outside world against anything bad that could happen
## also prevents global scope pollution
def _fillCfgDb():
    from GaudiKernel.Proxy.ConfigurableDb import CfgDb

    # get a handle on the repository of Configurables
    cfgDb = CfgDb()

    # populate the repository with informations on Configurables
""" % (parser.prog, time.asctime())

        for mod in cus:
            for cu in cus[mod]:
                output += """
    cfgDb.add( configurable = '%s',
               package = '%s',
               module  = '%s',
               lib     = 'None' )""" % (cu, package_name, mod)

        # trailer
        output += """

    return #_fillCfgDb

# fill cfgDb at module import...
try:
    _fillCfgDb()
    #house cleaning...
    del _fillCfgDb
except Exception,err:
    print "Py:ConfigurableDb   ERROR Problem with [%%s] content!" %% __name__
    print "Py:ConfigurableDb   ERROR",err
    print "Py:ConfigurableDb   ERROR   ==> culprit is package [%s] !"
""" % package_name
    elif usingConvention:
        logging.info("No ConfigurableUser found")
        output = ("# db file automatically generated by %s on: %s\n"
                  "# No ConfigurableUser specialization in %s\n") % (parser.prog, time.asctime(), package_name)
    else:
        logging.error("No ConfigurableUser specialization found")
        return 1

    # create the destination directory if not there
    output_dir = os.path.dirname(outputfile)
    if not os.path.exists(output_dir):
        logging.info("Creating directory %r", output_dir)
        os.makedirs(output_dir, 0755)

    # write output to file
    logging.verbose("Writing confDb data to %r", outputfile)
    open(outputfile, "w").write(output)
    return 0

if __name__ == '__main__':
    retcode = main()
    sys.exit(retcode)
