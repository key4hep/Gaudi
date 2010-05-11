# File: GaudiPython/Parallel.py
# Author: Pere Mato (pere.mato@cern.ch)

""" GaudiPython.Parallel module.
    This module provides 'parallel' processing support for GaudiPyhton.
    It is adding some sugar on top of public domain packages such as
    the 'processing' or the 'pp' packages. The interface can be made
    independent of the underlying implementation package.
    Two main class are defined: Task and WorkManager
"""

__all__ = [ 'Task','WorkManager' ]
excluded_varnames = ['HOSTNAME', 'SSH_CLIENT', 'SSH_CONNECTION', 'DISPLAY']

import sys, os, time, copy

# == Eoin's adds for Parallel Mode ====
from processing import Process, Queue, Pool, currentProcess
from ROOT import TBufferFile, TBuffer
from Gaudi.Configuration import appendPostConfigAction, Configurable, INFO, ERROR, VERBOSE
from GaudiPython import AppMgr, gbl, setOwnership, SUCCESS, PyAlgorithm
# from ParallelStats import recordReaderStats, recordWorkerStats, recordWriterStats
# =====================================

def _prefunction( f, task, item) :
    return f((task,item))
def _ppfunction( args ) :
    #--- Unpack arguments
    task, item = args
    stat = Statistics()
    #--- Initialize the remote side (at least once)
    if not task.__class__._initializeDone :
        for k,v in task.environ.items() :
            if k not in excluded_varnames : os.environ[k] = v
        task.initializeRemote()
        task.__class__._initializeDone = True
    #--- Reset the task output
    task._resetOutput()
    #--- Call processing
    task.process(item)
    #--- Collect statistics
    stat.stop()
    return (copy.deepcopy(task.output), stat)

def _detect_ncpus():
    """Detects the number of effective CPUs in the system"""
    #for Linux, Unix and MacOS
    if hasattr(os, "sysconf"):
        if os.sysconf_names.has_key("SC_NPROCESSORS_ONLN"):
            #Linux and Unix
            ncpus = os.sysconf("SC_NPROCESSORS_ONLN")
            if isinstance(ncpus, int) and ncpus > 0:
                return ncpus
            else:
                #MacOS X
                return int(os.popen2("sysctl -n hw.ncpu")[1].read())
    #for Windows
    if os.environ.has_key("NUMBER_OF_PROCESSORS"):
        ncpus = int(os.environ["NUMBER_OF_PROCESSORS"]);
        if ncpus > 0:
            return ncpus
    #return the default value
    return 1

class Statistics(object):
    def __init__(self):
        self.name  = os.getenv('HOSTNAME')
        self.start = time.time()
        self.time  = 0.0
        self.njob  = 0
    def stop(self):
        self.time = time.time() - self.start

class Task(object) :
    """ Basic base class to encapsulate any processing that is going to be porcessed in parallel.
        User class much inherit from it and implement the methods initializeLocal,
        initializeRemote, process and finalize.   """
    _initializeDone = False
    def __new__ ( cls, *args, **kwargs ):
        task = object.__new__( cls )
        task.output = ()
        task.environ = {}
        for k,v in os.environ.items(): task.environ[k] = v
        task.cwd = os.getcwd()
        return task
    def initializeLocal(self):
        pass
    def initializeRemote(self):
        pass
    def process(self, item):
        pass
    def finalize(self) :
        pass
    def _mergeResults(self, result) :
        if type(result) is not type(self.output) :
            raise TypeError("output type is not same as obtained result")
        #--No iteratable---
        if not hasattr( result , '__iter__' ):
            if hasattr(self.output,'Add') : self.output.Add(result)
            elif hasattr(self.output,'__iadd__') : self.output += result
            elif hasattr(self.output,'__add__') : self.output = self.output + result
            else : raise TypeError('result cannot be added')
        #--Dictionary---
        elif type(result) is dict :
            if self.output.keys() <= result.keys(): minkeys = self.output.keys()
            else: minkeys = result.keys()
            for key in result.keys() :
                if key in self.output :
                    if hasattr(self.output[key],'Add') : self.output[key].Add(result[key])
                    elif hasattr(self.output[key],'__iadd__') : self.output[key] += result[key]
                    elif hasattr(self.output[key],'__add__') : self.output[key] = self.output[key] + result[key]
                    else : raise TypeError('result cannot be added')
                else :
                    self.output[key] = result[key]
        #--Anything else (list)
        else :
            for i in range( min( len(self.output) , len(result)) ):
                if hasattr(self.output[i],'Add') : self.output[i].Add(result[i])
                elif hasattr(self.output[i],'__iadd__') : self.output[i] += result[i]
                elif hasattr(self.output[i],'__add__') : self.output[i] = self.output[i] + result[i]
                else : raise TypeError('result cannot be added')
    def _resetOutput(self):
        output =  (type(self.output) is dict) and self.output.values() or self.output
        for o in output :
            if hasattr(o, 'Reset'): o.Reset()


class WorkManager(object) :
    """ Class to in charge of managing the tasks and distributing them to
        the workers. They can be local (using other cores) or remote
        using other nodes in the local cluster """

    def __init__( self, ncpus='autodetect', ppservers=None) :
        if ncpus == 'autodetect' : self.ncpus = _detect_ncpus()
        else :                     self.ncpus = ncpus
        if ppservers :
            import pp
            self.ppservers = ppservers
            self.sessions = [ SshSession(srv) for srv in ppservers ]
            self.server = pp.Server(ncpus=self.ncpus, ppservers=self.ppservers)
            self.mode = 'cluster'
        else :
            import processing
            self.pool = processing.Pool(self.ncpus)
            self.mode = 'multicore'
        self.stats = {}

    def __del__(self):
        if hasattr(self,'server') : self.server.destroy()

    def process(self, task, items, timeout=90000):
        if not isinstance(task,Task) :
            raise TypeError("task argument needs to be an 'Task' instance")
        # --- Call the Local initialialization
        task.initializeLocal()
        # --- Schedule all the jobs ....
        if self.mode == 'cluster' :
            jobs = [self.server.submit(_prefunction, (_ppfunction, task, item), (), ('GaudiPython.Parallel','time')) for item in items]
            for job in jobs :
                result, stat = job()
                task._mergeResults(result)
                self._mergeStatistics(stat)
            self._printStatistics()
            self.server.print_stats()
        elif self.mode == 'multicore' :
            start = time.time()
            jobs = self.pool.map_async(_ppfunction, zip([task for i in items] , items ))
            for result, stat in  jobs.get(timeout) :
                task._mergeResults(result)
                self._mergeStatistics(stat)
            end = time.time()
            self._printStatistics()
            print 'Time elapsed since server creation %f' %(end-start)
        # --- Call the Local Finalize
        task.finalize()
    def _printStatistics(self):
        njobs = 0
        for stat in self.stats.values():
            njobs += stat.njob
        print 'Job execution statistics:'
        print 'job count | % of all jobs | job time sum | time per job | job server'
        for name, stat  in self.stats.items():
            print '       %d |        %6.2f |     %8.3f |    %8.3f | %s' % (stat.njob, 100.*stat.njob/njobs, stat.time, stat.time/stat.njob, name)

    def _mergeStatistics(self, stat):
        if stat.name not in self.stats : self.stats[stat.name] = Statistics()
        s = self.stats[stat.name]
        s.time += stat.time
        s.njob += 1


class SshSession(object) :
    def __init__(self, hostname):
        import pyssh
        import pp
        self.host = hostname
        ppprefix =  os.path.dirname(os.path.dirname(pp.__file__))
        self.session = pyssh.Ssh(host=hostname)
        self.session.open()
        self.session.read_lazy()
        self.session.write('cd %s\n' % os.getcwd())
        self.session.read_lazy()
        self.session.write('setenv PYTHONPATH %s\n' % os.environ['PYTHONPATH'])
        self.session.read_lazy()
        self.session.write('setenv LD_LIBRARY_PATH %s\n' % os.environ['LD_LIBRARY_PATH'])
        self.session.read_lazy()
        self.session.write('setenv ROOTSYS %s\n' % os.environ['ROOTSYS'])
        self.session.read_lazy()
        self.session.write('%s %s/scripts-%s/ppserver.py \n'%(sys.executable, ppprefix, sys.version.split()[0] ))
        self.session.read_lazy()
        self.session.read_lazy()
        print 'started ppserver in ', hostname
    def __del__(self):
        self.session.close()
        print 'killed ppserver in ', self.host

# ==== Extra stuff for the Event-Parallel Model =======================

# ===========================================================================================
# Miscellaneous Tools
# ===========================================================================================

aida2root = gbl.Gaudi.Utils.Aida2ROOT.aida2root
aidatypes = ( gbl.AIDA.IHistogram,
              gbl.AIDA.IHistogram1D,
              gbl.AIDA.IHistogram2D,
              gbl.AIDA.IHistogram3D,
              gbl.AIDA.IProfile1D,
              gbl.AIDA.IProfile2D )
thtypes   = ( gbl.TH1D, gbl.TH2D, gbl.TH3D, gbl.TProfile, gbl.TProfile2D )
gppHead   = '[ GaudiPythonParallel ] '
line      = '-'*80

def setupSystem(nWorkers, config) :
    qLimit = 50
    inq         = Queue( )                                        # the Queue from Reader->(Workers)
    outQs       = [ Queue( qLimit ) for i in xrange(nWorkers) ]   # One outQueue for each Worker->Writer

    commonQueue = Queue( )                                        # Common Queue from setupSystem->(everyone) (for signals)
    qToParent   = Queue( )                                        # Common Queue from (everyone)->setupSystem (for signals)

    cStatQueue  = Queue( )                                        # A common queue, shared for (Workers)->Writer
    rHistQ      = Queue( )                                        # A special Queue from Reader->Writer (bypassing workers)

    # keys of the config dictionary
    ks = config.keys()
    ks.sort()

    # determine application
    apps = ['Gauss', 'Boole', 'Brunel', 'DaVinci']
    _app = None
    for app in apps :
        if app in ks : _app = app

    # determine output streams
    outs = [ 'Output', 'OutputFile', 'OutStream' ]
    outlist = []
    for k in ks :
        for o in outs :
            if hasattr( config[k], o ) :  outlist.append(k)

    itmlst = getOutputList( config )

    # create Reader/Worker/Writer classes
    w   =  Writer(        commonQueue, outQs,    cStatQueue, rHistQ, qToParent, nWorkers, config, qLimit, _app )
    wks = [Worker(i, inq, commonQueue, outQs[i], cStatQueue,         qToParent, nWorkers, config, qLimit, _app, itemlist=itmlst) for i in xrange(nWorkers)]
    r   =  Reader(   inq, commonQueue,                       rHistQ, qToParent, nWorkers, config, qLimit, _app )

    nProcesses = nWorkers+2   # workers + (reader + writer)
    nSenders   = nWorkers+1   # workers + reader, the writer doesn't send anything
    nc = 0

    # collect statistics dicts from each worker
    store = [] ; hFlagCt = 0
    while nc < nProcesses :
        item = qToParent.get()
        if item    == 'h' : print 'HFLAG RECEIVED BY SETUPSYSTEM' ; [commonQueue.put(None) for i in xrange(nSenders)]
        if not item : nc += 1 # ; print 'Parent : NC now: %i'%( nc )
    # print ' PARENT : all None flags received'
    wks = []
    for dictionary in store :
        if dictionary['name'] == 'Worker' : wks.append(dictionary)
        if dictionary['name'] == 'Reader' : reader = dictionary
        if dictionary['name'] == 'Writer' : writer = dictionary
    # print 'PARENT recording stats'
    # recordReaderStats( reader )
    # recordWorkerStats( wks )
    # recordWriterStats( writer )
    # print ' ==================================================== PARENT FINISHED, RETURNING'
    return True

def getOutputList( configuration ) :
    writers = configuration['ApplicationMgr'].OutStream
    print 'setupSystem: OutStreams identified : ', writers
    # check which tes items are for writing by looking at the writer algorithm settings
    # remove the trailing '#n' (sub-level notation for writer algorithms)
    outct = 0
    itmlst = []
    for w in writers :
        if hasattr(w,'Output') :
            for itm in w.ItemList + w.OptItemList :
                hsh = itm.find('#')
                itmlst.append(itm[:hsh])
    return itmlst

def dumpHistograms( hvt, node='Unspecified', omitList=[] ) :
    nlist = hvt.getHistoNames( )
    throw = []
    for om in omitList :
        for name in nlist :
            if name.startswith(om) : throw.append(name)
    [ nlist.remove(name) for name in throw ]
    del throw
    histDict = {}
    objects = 0 ; histos = 0
    if nlist :
        for n in nlist :
            o = hvt[ n ]
            if type(o) in aidatypes :
                o = aida2root(o)
                histos  += 1
            else :
                objects += 1
            histDict[ n ] = o
    else :
        print head+'WARNING : no histograms to recover?'
    # print line
    # print '%s : Histos collected'%(node)
    # print 'Objects : %i'%( objects    )
    # print 'Histos  : %i'%( histos     )
    # print 'Total   : %i'%( len(histDict.keys()) )
    # print line
    return histDict

def dumpTES( someClass ) :
    buf = TBufferFile(TBuffer.kWrite)
    someClass.ts.dumpBuffer(buf)
    return buf

def dumpTEStoMessage( self ) :
    buf = TMessage()
    self.ts.dumpBuffer(buf)
    return buf

def loadTES( someClass, tbuf ) :
    # Using TMessages/TSockets, we DON'T do SetBufferOffset/ReadMode.  But we are using TBufferFiles
    # so it must be done always
    # if someClass.__class__.__name__ == 'Worker'  : tbuf.SetBufferOffset() ; tbuf.SetReadMode()
    # if someClass.__class__.__name__ == 'Writer'  : tbuf.SetBufferOffset() ; tbuf.SetReadMode()
    tbuf.SetBufferOffset() ; tbuf.SetReadMode()
    root = gbl.DataObject()
    someClass.evt.setRoot('/Event', root)
    setOwnership(root, False)
    someClass.ts.loadBuffer(tbuf)

class CollectHistograms( PyAlgorithm ) :
    def __init__( self, node ) :
        self.node = node
        self.omit = node.omitHistos
        PyAlgorithm.__init__( self )
        return None
    def execute( self ) :
        return SUCCESS
    def finalize( self ) :
        header = 'CollectHistograms : '
        w = self.node
        nodeName = w.__class__.__name__
        # print '*'*80
        w.histDict = dumpHistograms( w.hvt, node=nodeName, omitList= self.omit )
        # print header+'%s %i : about to send %i histos on output queue'%( nodeName, w.id, len(w.histDict.keys()) )
        # w.cstatq.put( (w.id, w.histDict) )
        ks = w.histDict.keys()
        # send 100 at a time
        chunk = 100
        reps = len(ks)/chunk + 1
        for i in xrange(reps) :
            someKeys = ks[i*chunk : (i+1)*chunk]
            smalld = dict( [(key, w.histDict[key]) for key in someKeys] )
            w.cstatq.put( (w.id, smalld) )
        w.cstatq.put( None )
        # print header+'%s %i: Histos queued to writer'%( nodeName, w.id )
        for item in iter(w.commonQ.get, None) : print 'Worker %i : Got an item on the Common Queue?'%(w.id)
        # now clear the histogram store
        # print 'Clearing Histo store'
        w.hvt.clearStore()
        root = gbl.DataObject()
        w.hvt.setRoot('/stat', root)
        w.hvt.dump()
        # print '*'*80
        return SUCCESS

# ===========================================================================================
# The Reader
# ===========================================================================================

class Reader( ) :
    def __init__( self, inq, commonQueue, rstatq, qToParent, workers, config, qLimit, _app ) :
        self.inq       = inq
        self.c         = config
        self.commonQ   = commonQueue
        self.cstatq    = rstatq
        self.workers   = workers
        self.qToParent = qToParent
        self.qLimit    = qLimit
        self.id        = -1
        self._app      = _app

        # from ParallelStats import constructReaderDict
        # self.constructReaderDict = constructReaderDict

        r = Process( target=self.read )
        r.start()

    def readerConfig( self ) :
        ks = self.c.keys()
        if 'ApplicationMgr' in ks :
            self.c['ApplicationMgr'].OutStream = []
            if self._app == 'Gauss' : pass
            else : self.c['ApplicationMgr'].TopAlg  = []
        else :
            self.qToParent.put(None)
            print 'Reader : readerConfig : ApplicationMgr not available for configuration?'

        try    : self.c['HistogramPersistencySvc'].OutputFile = ''
        except : print 'Reader : No Histogram output to cancel!  Config continues...'

        if 'MessageSvc' in ks :
            self.c['MessageSvc'].Format      = '[Reader]% F%18W%S%7W%R%T %0W%M'
            self.c['MessageSvc'].OutputLevel = ERROR
        else :
            self.qToParent.put(None)
            print 'Reader : readerConfig : MessageSvc not available for configuration?'
        if self._app == 'Gauss' :
            gs = self.c['GaussSequencer']
            # Sequencer has two stages : generation and simulation, so let
            # the reader generate events one by one, and pass to workers for full Simulation step
            ed = [gs.Members[0]]
            gs.Members = ed

    def read( self ):
        currentProcess().setName('+Reader+')
        appendPostConfigAction( self.readerConfig() )

        # print '[ GaudiPython.Parallel ] Reader Started : Process %i'%( os.getpid() )

        self.ct       = 0

        # GaudiPython Tools
        self.a = AppMgr()
        self.evt = self.a.evtsvc()
        self.hvt = self.a.histsvc()
        self.ts = gbl.GaudiPython.TESSerializer(self.evt._idp)
        self.omitHistos = ['/stat/CaloPIDs']
        collectHistos = CollectHistograms( self )
        self.a.addAlgorithm( collectHistos )
        self.a.initialize()
        self.a.start()

        for i in xrange( self.c['ApplicationMgr'].EvtMax ) :
            self.a.run(1)
            self.ts = gbl.GaudiPython.TESSerializer(self.evt._idp)

            if self._app == 'Gauss' :
                if self.evt.getHistoNames() :
                    for i in self.evt.getHistoNames() : self.ts.addOptItem(i,1)
            else :
                if self.evt.getList() :
                    for i in self.evt.getList() : self.ts.addOptItem(i,1)
            buf = dumpTES( self )
            sent = False
            while sent == False :
                try :
                    self.inq.put( buf, block=True )
                    while self.inq._buffer : pass # try pause until the background thread is complete
                    sent = True ; self.ct += 1
                except :
                    self.errs += 1
            self.evt.clearStore()
        # signal the end
        for w in range(self.workers) : self.inq.put(None)


        # Termination
        self.a.stop()
        self.a.finalize()

        # print '='*80
        # print 'Reader Finished, AppMgr stopped, finalized.'
        # print line
        # print 'Events Sent : %i'%( self.ct )
        # print '='*80

        self.qToParent.put(None) # ; print 'READER has returned None Flag to setupSystem'

# ===========================================================================================
# The Worker
# ===========================================================================================

class Worker( ) :
    def __init__( self, wid, inq, cq, outq, cstatq, qToParent, nprocs, config, qLimit, _app, itemlist=None ) :

        # wid      : an integer (0...Nworkers-1) identifying the worker
        # in_q     : the common queue from reader to workers
        # out_q    : the queue from a worker to the writer (unique queue for each worker)
        # common_q : the common queue from workers to writer, along which 'id' will be sent on event completion
        self.id           = wid
        self.c            = config
        self.inq          = inq
        self.qLimit       = qLimit
        self.commonQ      = cq
        self.cstatq       = cstatq
        self.qToParent    = qToParent
        self.nprocs       = nprocs
        self.outList      = []
        self.tempCt       = 0
        self.putErrs      = 0
        self.completed    = 0
        self.KeepGoing    = True
        self.itmlst       = itemlist
        self._app      = _app

        self.outq         = outq

        # from ParallelStats import constructWorkerDict
        # self.constructWorkerDict = constructWorkerDict

        if self.itmlst : self.eventOutput = True
        else           : self.eventOutput = False

        w = Process( target=self.work )
        w.setDaemon(True)
        w.start()

    def workerConfig( self ) :
        ks = self.c.keys()
        if 'ApplicationMgr' in ks :
            self.c['ApplicationMgr'].OutStream = []
        else :
            self.qToParent.put(None)
            print 'Worker %i : workerConfig : ApplicationMgr not available for configuration?'%( self.id )
        if 'EventSelector' in ks :
            self.c['EventSelector'].Input      = []
        else :
            print 'Worker %i : workerConfig : EventSelector not available for configuration?'%( self.id )
        try    : self.c['HistogramPersistencySvc'].OutputFile = ''
        except : print 'Worker-%i: No Histogram output to cancel! Config continues...'%self.id
        formatHead = '[Worker %i]'%self.id
        if 'MessageSvc' in ks :
            self.c['MessageSvc'].Format = formatHead+'% F%18W%S%7W%R%T %0W%M'
            self.c['MessageSvc'].OutputLevel = INFO
        else :
            print 'Worker %i : workerConfig : MessageSvc not available for configuration?'%( self.id )

        if self.id :
            for k in ks :
                if hasattr( self.c[k], 'OutputLevel' ) : self.c[k].OutputLevel = ERROR

        if self._app == 'Gauss' :
            gs = self.c[ 'GaussSequencer' ]
            # Sequencer has two stages : generation and simulation, so let
            # the reader generate events one by one, and pass to workers for full Simulation step
            ed = [ gs.Members[1] ]
            gs.Members = ed

        if self._app == 'DaVinci' :
            if 'NTupleSvc' in self.c.keys() :
                self.c['NTupleSvc'].Output = ["FILE1 DATAFILE='Worker-%i-Hlt12-StatsTuple.root' TYP='ROOT' OPT='NEW'"%self.id]
                self.c['NTupleSvc'].OutputLevel = VERBOSE

    def workerExecuteWithOutput( self, tbuf ) :
        loadTES( self, tbuf )
        self.a._evtpro.executeEvent()
        buf = dumpTES( self )
        self.outq.put(buf)
        # allow the background thread to feed the Queue; not 100% guaranteed to finish before next line
        while self.outq._buffer : pass
        self.evt.clearStore()
        self.completed += 1
        return True

    def workerExecuteNoOutput( self, tbuf ) :
        loadTES( self, tbuf )
        self.a._evtpro.executeEvent()
        self.outq.put('dummy')
        self.evt.clearStore()
        self.completed += 1
        return True

    def work( self ):
        # print 'Worker %d: starting...%d, at %5.6f' % (self.id, os.getpid(), alive)
        cName = currentProcess().getName()
        currentProcess().setName('Worker '+cName)
        appendPostConfigAction( self.workerConfig() )

        self.ct = 0

        # take first event
        buf = self.inq.get()
        if buf is not None :
            # set up GaudiPython tools
            self.a   = AppMgr()
            self.evt = self.a.evtsvc()
            self.hvt = self.a.histsvc()
            self.nvt = self.a.ntuplesvc()
            first = True
            self.ts = gbl.GaudiPython.TESSerializer(self.evt._idp)
            self.omitHistos = ['/stat/CaloPIDs']
            collectHistos = CollectHistograms( self )
            self.a.addAlgorithm( collectHistos )
            self.a.initialize()
            self.a.start()

            # worker execution changes depending on output/no-output scenario
            if self.eventOutput :
                [ self.ts.addOptItem(itm,1) for itm in self.itmlst ]
                wFunction = self.workerExecuteWithOutput
            else :
                wFunction = self.workerExecuteNoOutput


            # iter over the Queue to workers, receiving serialized events. When done, workers place None on queue
            while self.KeepGoing :
                if first : pass
                else : buf = self.inq.get()
                if buf is not None :
                    sc = wFunction( buf )
                    if sc : pass
                    self.ct += 1
                    if first : first = False
                else :
                    self.KeepGoing = False

            # print '='*80
            # print 'Worker %i Finished'%( self.id )
            self.outq.put(None)
            # print line
            # print 'Events Recd : %i'%( self.ct )
            # print '='*80

            sc = self.finalize()
            self.qToParent.put(None)
            # print '[ GaudiPython Parallel ] : Worker has sent dict and None flag back to Parent'
        else : # if the buffer is None...
            self.outq.put(None)
            sc = self.finalize()
            self.qToParent.put(None)
            self.cstatq.put( (self.id, {}) )
            self.cstatq.put( None )
            for item in iter(self.commonQ.get, None) : print 'Worker %i : Got an item on the Common Queue?'%(self.id)

    def finalize( self ) :
        self.a.stop()
        self.a.finalize()
        self.Finished = True
        return True

# ===========================================================================================
# The Writer
# ===========================================================================================

class Writer( ) :
    def __init__( self, common_queue, out_qList, cstatq, rstatq, qToParent, workers, config, qLimit, _app ) :
        self.qList     = out_qList
        self.cq        = common_queue
        self.cstatq    = cstatq
        self.rstatq    = rstatq
        self.qToParent = qToParent
        self.qLimit    = qLimit
        self.workers   = workers
        self.output    = True
        self.flags     = 0
        self.c         = config
        self._app      = _app

        # from ParallelStats import constructWriterDict
        # self.constructWriterDict = constructWriterDict

        self.bookingDict = {}
        self.bookingDict['DataObject']        = self.bookDataObject
        self.bookingDict['NTuple::Directory'] = self.bookDataObject
        self.bookingDict['NTuple::File']      = self.bookDataObject
        self.bookingDict['TH1D']       = self.bookTH1D
        self.bookingDict['TH2D']       = self.bookTH2D
        self.bookingDict['TH3D']       = self.bookTH3D
        self.bookingDict['TProfile']   = self.bookTProfile
        self.bookingDict['TProfile2D'] = self.bookTProfile2D

        w = Process( target=self.write )
        w.start()

    def bookDataObject( self, n, o ):
        self.hvt.registerObject( n, o )

    def bookTH1D( self, n, o ) :
        obj = self.hvt._ihs.book(n, o.GetTitle(), o.GetXaxis().GetNbins(), o.GetXaxis().GetXmin(), o.GetXaxis().GetXmax())
        aida2root(obj).Add(o)

    def bookTH2D( self, n, o ) :
        obj = self.hvt._ihs.book(n, o.GetTitle(), o.GetXaxis().GetNbins(), o.GetXaxis().GetXmin(), o.GetXaxis().GetXmax(), o.GetYaxis().GetNbins(), o.GetYaxis().GetXmin(), o.GetYaxis().GetXmax())
        aida2root(obj).Add(o)

    def bookTH3D( self, n, o ) :
        obj = self.hvt._ihs.book(n, o.GetTitle(), o.GetXaxis().GetXbins(), o.GetXaxis().GetXmin(), o.GetXaxis().GetXmax(),
                                                  o.GetYaxis().GetXbins(), o.GetYaxis().GetXmin(), o.GetYaxis().GetXmax(),
                                                  o.GetZaxis().GetXbins(), o.GetZaxis().GetXmin(), o.GetZaxis().GetXmax() )
        aida2root(obj).Add(o)

    def bookTProfile( self, n, o ) :
        obj = self.hvt._ihs.bookProf(n, o.GetTitle(), o.GetXaxis().GetNbins(), o.GetXaxis().GetXmin(), o.GetXaxis().GetXmax(), o.GetOption())
        aida2root(obj).Add(o)

    def bookTProfile2D( self, n, o ) :
        obj = self.hvt._ihs.bookProf(n, o.GetTitle(), o.GetXaxis().GetNbins(), o.GetXaxis().GetXmin(), o.GetXaxis().GetXmax(), o.GetYaxis().GetNbins(), o.GetYaxis().GetXmin(), o.GetYaxis().GetXmax(), o.GetOption())
        aida2root(obj).Add(o)


    def writerConfig( self ) :
        ks = self.c.keys()
        if 'EventSelector' in ks :
            self.c['EventSelector'].Input      = []
        else :
            print 'Writer : writerConfig : EventSelector not available for configuration?'
        if 'ApplicationMgr' in ks :
            self.c['ApplicationMgr'].TopAlg    = []
        else :
            print 'Writer : writerConfig : ApplicationMgr not available for configuration?'
        if 'MessageSvc' in ks :
            self.c['MessageSvc'].Format        = '[Writer]% F%18W%S%7W%R%T %0W%M'
        else :
            print 'Writer : writerConfig : MessageSvc not available for configuration?'
        # sometimes the outstreams require that certain algs have completed
        # obviously, these algs aren't going to be run on our independent writer
        for k in self.c.keys() :
            if hasattr(self.c[k], 'RequireAlgs') : self.c[k].RequireAlgs = []

    def write( self ):
        currentProcess().setName('+Writer+')
        print 'WRITER : applying PostConfigAction'
        appendPostConfigAction( self.writerConfig() )
        print '[ GaudiPython.Parallel ] Writer started : Process %i'%( os.getpid() )

        self.a = AppMgr()
        self.a.initialize()
        self.a.start()
        self.evt = self.a.evtsvc()
        self.hvt = self.a.histsvc()
        self.ts = gbl.GaudiPython.TESSerializer(self.evt._idp)

        self.ct = 0
        status = [True]*self.workers
        recvd  = [0]*self.workers
        cqnc = 0  # none count from the common queue
        gotOK = True

        ind = 0 ; gotWhich = False ; ifblock = [] ; tryBlock = [] ; starttry = None ; outs = []

        waitForFlag = True ; whichQ = 0
        while sum(status) > 0 :
            whichQ = (whichQ+1)%self.workers
            outq = self.qList[whichQ]
            trying = True
            try :
                tbuf = outq.get(timeout=0.01,block=False)
                trying = False
            except :
                continue
            if tbuf :
                recvd[whichQ] += 1
                try :
                    loadTES( self, tbuf )
                    self.a._evtpro.executeEvent()   # fire the writing of TES to output file
                    self.evt.clearStore()           # and clear out the TES
                    self.ct += 1
                    if not self.ct%20 : print '[ GaudiPython.Parallel ] Writer Progress (n. Events) : %i'%self.ct
                except :
                    print '[ GaudiPython.Parallel ] Writer trying to load a ', type(tbuf), tbuf[:10] ,  '??... skipping to next'
            else :
                status[whichQ] = False   # that worker is finished...
                # print 'Writer received None from worker %i'%( whichQ )

        # print '[ GaudiPython.Parallel ] Writer Complete : %i Events received'%self.ct
        if self.output : self.finalize()
        else           : self.a.stop()   ; self.a.finalize()
        # self.constructWriterDict()
        # self.qToParent.put(self.d)
        self.qToParent.put(None)

    def composition( self ) :
        lst = self.hvt.getHistoNames()
        record = []
        objects = 0 ; histos = 0
        if lst :
            for n in lst :
                o = self.hvt[ n ]
                if type(o) in aidatypes :
                    histos  += 1
                    record.append( (n, o.entries()) )
                else :
                    objects += 1
        else :
            print 'Empty list!'
        # print line
        # print 'Size of Histo Store : %i'%( len(lst) )
        # print ' - Histos           : %i'%( histos   )
        # print ' - Objects          : %i'%( objects  )
        # print line

    def finalize( self ) :
        nc = 0
        self.HistoCollection = []
        for item in iter(self.rstatq.get, None) : self.HistoCollection.append( item )
        while nc < self.workers :
            tup = self.cstatq.get()
            if tup : self.HistoCollection.append( tup )   # tup is (worker-id, histoDict)
            else   : nc += 1
        self.HistoCollection.sort()
        # send signal to Parent that all Histos have been received
        self.qToParent.put('h')
        self.RebuildHistoStore()

        self.a.stop()
        self.a.finalize()
        # print '[ GaudiPython.Parallel ] Writer Complete.'

    def RebuildHistoStore( self ) :
        for tup in self.HistoCollection :
            workerID, histDict = tup
            added = 0 ; registered = 0; booked = 0
            for n in histDict.keys() :
                o = histDict[ n ]
                obj = self.hvt.retrieve( n )
                if obj :
                    aida2root(obj).Add(o)
                    added += 1
                else :
                    if o.__class__.__name__ in self.bookingDict.keys() :
                        self.bookingDict[o.__class__.__name__](n, o)
                    else :
                        print 'No booking method for: ', n, o, type(o), o.__class__.__name__
                    booked += 1
            # print '='*80
            # print 'Set of Histos complete'
            # print 'Added      (histos)  : %i'%( added      )
            # print 'Registered (objects) : %i'%( registered )
            # print 'Booked     (histos)  : %i'%( booked     )
            # print line
            # print 'Size of Histo Store  : %i'%( len( self.hvt.getHistoNames() ) )
            # print '='*80
        # print 'o'*80
        # print 'Writer : All Histogram sets added.'
        # self.composition( )
        # print 'o'*80

# == EOF ====================================================================================
