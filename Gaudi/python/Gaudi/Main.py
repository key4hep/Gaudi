import sys, os
from time import time
from Gaudi import Configuration
import logging

log = logging.getLogger(__name__)

class gaudimain(object) :
    def __init__(self) :
        from Configurables import ApplicationMgr
        appMgr = ApplicationMgr()
        if "GAUDIAPPNAME" in os.environ:
            appMgr.AppName = str(os.environ["GAUDIAPPNAME"])
        if "GAUDIAPPVERSION" in os.environ:
            appMgr.AppVersion = str(os.environ["GAUDIAPPVERSION"])

    def generatePyOutput(self, all = False):
        from pprint import pformat
        conf_dict = Configuration.configurationDict(all)
        return pformat(conf_dict)

    def generateOptsOutput(self, all = False):
        from pprint import pformat
        conf_dict = Configuration.configurationDict(all)
        out = []
        names = conf_dict.keys()
        names.sort()
        for n in names:
            props = conf_dict[n].keys()
            props.sort()
            for p in props:
                out.append('%s.%s = %s;' % (n,p, repr(conf_dict[n][p])))
        return "\n".join(out)

    def _writepickle(self, filename) :
        #--- Lets take the first file input file as the name of the pickle file
        import pickle
        output = open(filename, 'wb')
        # Dump only the the configurables that make sense to dump (not User ones)
        from GaudiKernel.Proxy.Configurable import getNeededConfigurables
        to_dump = {}
        for n in getNeededConfigurables():
            to_dump[n] = Configuration.allConfigurables[n]
        pickle.dump(to_dump, output, -1)
        output.close()

    def printconfig(self, old_format = False, all = False) :
        msg = 'Dumping all configurables and properties'
        if not all:
            msg += ' (different from default)'
        log.info(msg)
        conf_dict = Configuration.configurationDict(all)
        if old_format:
            print self.generateOptsOutput(all)
        else:
            print self.generatePyOutput(all)

    def writeconfig(self, filename, all = False):
        write = { ".pkl" : lambda filename, all: self._writepickle(filename),
                  ".py"  : lambda filename, all: open(filename,"w").write(self.generatePyOutput(all) + "\n"),
                  ".opts": lambda filename, all: open(filename,"w").write(self.generateOptsOutput(all) + "\n"),
                }
        from os.path import splitext
        ext = splitext(filename)[1]
        if ext in write:
            write[ext](filename, all)
        else:
            log.error("Unknown file type '%s'. Must be any of %r.", ext, write.keys())
            sys.exit(1)

    def printsequence(self):
        def printAlgo( algName, appMgr, prefix = ' ') :
            print prefix + algName
            alg = appMgr.algorithm( algName.split( "/" )[ -1 ] )
            prop = alg.properties()
            if prop.has_key( "Members" ) :
                subs = prop[ "Members" ].value()
                for i in subs : printAlgo( i.strip( '"' ), appMgr, prefix + "     " )
            elif prop.has_key( "DetectorList" ) :
                subs = prop[ "DetectorList" ].value()
                for i in subs : printAlgo( algName.split( "/" )[ -1 ] + i.strip( '"' ) + "Seq", appMgr, prefix + "     ")
        import GaudiPython
        self.g = GaudiPython.AppMgr()
        mp = self.g.properties()
        print "\n ****************************** Algorithm Sequence **************************** \n"
        for i in mp["TopAlg"].value(): printAlgo( i, self.g )
        print "\n ****************************************************************************** \n"

    ## Instantiate and run the application.
    #  Depending on the number of CPUs (ncpus) specified, it start
    def run(self, ncpus = None):
        if not ncpus:
            # Standard sequential mode
            result = self.runSerial()
        else:
            # Doing the checking here because it is not done in runParallel
            ncpus = int(ncpus)
            assert(ncpus > 0)
            # Parallel mode
            result = self.runParallel(ncpus)
        return result

    def runSerial(self) :
        #--- Instantiate the ApplicationMgr------------------------------
        import GaudiPython
        log.debug('-'*80)
        log.debug('%s: running in serial mode', __name__)
        log.debug('-'*80)
        sysStart = time()
        self.g = GaudiPython.AppMgr()
        success = self.g.run(self.g.EvtMax).isSuccess()
        success = self.g.exit().isSuccess() and success
        if not success:
            return 1
        sysTime = time()-sysStart
        log.debug('-'*80)
        log.debug('%s: serial system finished, time taken: %5.4fs', __name__, sysTime)
        log.debug('-'*80)
        return 0

    def runParallel(self, ncpus) :
        from Gaudi.Configuration import Configurable
        import GaudiPython.Parallel as gpp
        c = Configurable.allConfigurables
        log.info('-'*80)
        log.info('%s: running in parallel mode', __name__)
        log.info('-'*80)
        sysStart = time()
        sc = gpp.setupSystem(ncpus, c)
        if not sc:
            return 1
        sysTime = time()-sysStart
        log.info('-'*80)
        log.info('%s: parallel system finished, time taken: %5.4fs', __name__, sysTime)
        log.info('-'*80)
        return 0
