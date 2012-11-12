import sys, os
from time import time
from Gaudi import Configuration
import logging

log = logging.getLogger(__name__)

class gaudimain(object) :
    # main loop implementation, None stands for the default
    mainLoop = None

    def __init__(self) :
        from Configurables import ApplicationMgr
        appMgr = ApplicationMgr()
        if "GAUDIAPPNAME" in os.environ:
            appMgr.AppName = str(os.environ["GAUDIAPPNAME"])
        if "GAUDIAPPVERSION" in os.environ:
            appMgr.AppVersion = str(os.environ["GAUDIAPPVERSION"])
        self.log = logging.getLogger(__name__)
        self.printsequence = False

    def setupParallelLogging( self ) :
        # ---------------------------------------------------
        # set up Logging
        # ----------------
        # from multiprocessing import enableLogging, getLogger
        import multiprocessing
        # preliminaries for handlers/output files, etc.
        from time import ctime
        datetime = ctime()
        datetime = datetime.replace(' ', '_')
        outfile = open( 'gaudirun-%s.log'%(datetime), 'w' )
        # two handlers, one for a log file, one for terminal
        streamhandler = logging.StreamHandler(strm=outfile)
        console       = logging.StreamHandler()
        # create formatter : the params in parentheses are variable names available via logging
        formatter = logging.Formatter( "%(asctime)s - %(name)s - %(levelname)s - %(message)s" )
        # add formatter to Handler
        streamhandler.setFormatter(formatter)
        console.setFormatter(formatter)
        # now, configure the logger
        # enableLogging( level=0 )
        # self.log = getLogger()
        self.log = multiprocessing.log_to_stderr()
        self.log.setLevel( logging.INFO )
        self.log.name = 'Gaudi/Main.py Logger'
        self.log.handlers = []
        # add handlers to logger : one for output to a file, one for console output
        self.log.addHandler(streamhandler)
        self.log.addHandler(console)
        self.log.removeHandler(console)
        # set level!!
        self.log.setLevel = logging.INFO
        # ---------------------------------------------------

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

    def _printsequence(self):
        if not self.printsequence:
            # No printing requested
            return

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
            # Otherwise, run with the specified number of cpus
            result = self.runParallel(ncpus)
        return result


    def runSerial(self) :
        #--- Instantiate the ApplicationMgr------------------------------
        import GaudiPython
        self.log.debug('-'*80)
        self.log.debug('%s: running in serial mode', __name__)
        self.log.debug('-'*80)
        sysStart = time()
        self.g = GaudiPython.AppMgr()
        self._printsequence()
        runner = self.mainLoop or (lambda app, nevt: app.run(nevt))
        try:
            statuscode = runner(self.g, self.g.EvtMax)
        except SystemError:
            # It may not be 100% correct, but usually it means a segfault in C++
            self.g.ReturnCode = 128 + 11
            statuscode = False
        except:
            # for other exceptions, just set a generic error code
            self.g.ReturnCode = 1
            statuscode = False
        if hasattr(statuscode, "isSuccess"):
            success = statuscode.isSuccess()
        else:
            success = statuscode
        success = self.g.exit().isSuccess() and success
        if not success and self.g.ReturnCode == 0:
            # ensure that the return code is correctly set
            self.g.ReturnCode = 1
        sysTime = time()-sysStart
        self.log.debug('-'*80)
        self.log.debug('%s: serial system finished, time taken: %5.4fs', __name__, sysTime)
        self.log.debug('-'*80)
        return self.g.ReturnCode

    def runParallel(self, ncpus) :
        if self.mainLoop:
            self.log.fatal("Cannot use custom main loop in multi-process mode, check your options")
            return 1
        self.setupParallelLogging( )
        from Gaudi.Configuration import Configurable
        import GaudiMP.GMPBase as gpp
        c = Configurable.allConfigurables
        self.log.info('-'*80)
        self.log.info('%s: Parallel Mode : %i '%(__name__, ncpus))
        from commands import getstatusoutput as gso
        metadataCommands = [ 'uname -a',
                             'echo $CMTCONFIG',
                             'echo $GAUDIAPPNAME',
                             'echo $GAUDIAPPVERSION']
        for comm in metadataCommands :
            s, o = gso( comm )
            if s :
                o = "Undetermined"
            string = '%s: %30s : %s '%(__name__, comm, o)
            self.log.info( string )
        try :
            events = str(c['ApplicationMgr'].EvtMax)
        except :
            events = "Undetermined"
        self.log.info('%s: Events Specified : %s '%(__name__,events))
        self.log.info('-'*80)
        # Parall = gpp.Coordinator(ncpus, shared, c, self.log)
        Parall = gpp.Coord( ncpus, c, self.log )
        sysStart = time()
        sc = Parall.Go()
        self.log.info('MAIN.PY : received %s from Coordinator'%(sc))
        if sc.isFailure() :
            return 1
        sysTime = time()-sysStart
        self.log.name = 'Gaudi/Main.py Logger'
        self.log.info('-'*80)
        self.log.info('%s: parallel system finished, time taken: %5.4fs', __name__, sysTime)
        self.log.info('-'*80)
        return 0
