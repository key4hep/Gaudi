from Gaudi.Configuration import *
from Configurables import EvtCounter
from GaudiPython import AppMgr, gbl, setOwnership, PyAlgorithm, SUCCESS,FAILURE, InterfaceCast
from ROOT import TBufferFile, TBuffer
import multiprocessing
from multiprocessing import Process, Queue, JoinableQueue, Event
from multiprocessing import cpu_count, current_process
from multiprocessing.queues import Empty
from pTools import *
import time, sys, os
from ROOT import TParallelMergingFile
# This script contains the bases for the Gaudi MultiProcessing (GMP)
# classes

# There are three classes :
#   Reader
#   Worker
#   Writer

# Each class needs to perform communication with the others
# For this, we need a means of communication, which will be based on
# the python multiprocessing package
# This is provided in SPI pytools package
# cmt line : use pytools v1.1 LCG_Interfaces
# The PYTHONPATH env variable may need to be modified, as this might
# still point to 1.0_python2.5

# Each class will need Queues, and a defined method for using these
# queues.
# For example, as long as there is something in the Queue, both ends
# of the queue must be open
# Also, there needs to be proper Termination flags and criteria
# The System should be error proof.


# Constants -------------------------------------------------------------------
NAP = 0.001
MB  = 1024.0*1024.0
# waits to guard against hanging, in seconds
WAIT_INITIALISE   = 60*5
WAIT_FIRST_EVENT  = 60*3
WAIT_SINGLE_EVENT = 60*6
WAIT_FINALISE     = 60*2
STEP_INITIALISE   = 10
STEP_EVENT        = 2
STEP_FINALISE     = 10

# My switch for direct switching on/off Smaps Algorithm in GaudiPython AppMgr
SMAPS = False

# -----------------------------------------------------------------------------

# definitions
# ----------
# used to convert stored histos (in AIDA format) to ROOT format
aida2root = gbl.Gaudi.Utils.Aida2ROOT.aida2root

# used to check which type of histo we are dealing with
# i.e. if currentHisto in aidatypes : pass
aidatypes = ( gbl.AIDA.IHistogram,
              gbl.AIDA.IHistogram1D,
              gbl.AIDA.IHistogram2D,
              gbl.AIDA.IHistogram3D,
              gbl.AIDA.IProfile1D,
              gbl.AIDA.IProfile2D,
              gbl.AIDA.IBaseHistogram  )  # extra?

# similar to aidatypes
thtypes   = ( gbl.TH1D, gbl.TH2D, gbl.TH3D, gbl.TProfile, gbl.TProfile2D )

# Types of OutputStream in Gaudi
WRITERTYPES  = {  'EvtCollectionStream'    : "tuples",
                  'InputCopyStream'        : "events",
                  'OutputStream'           : "events",
                  'RecordStream'           : "records",
                  'RunRecordStream'        : "records",
                  'SequentialOutputStream' : "events",
                  'TagCollectionStream'    : "tuples"   }

# =============================================================================

class MiniWriter( object ) :
    '''
    A class to represent a writer in the GaudiPython configuration
    It can be non-trivial to access the name of the output file; it may be
    specified in the DataSvc, or just on the writer, may be a list, or string
    Also, there are three different types of writer (events, records, tuples)
    so this bootstrap class provides easy access to this info while configuring
    '''
    def __init__( self, writer, wType, config ) :
        self.w     = writer
        self.wType = wType
        # set parameters for directly accessing the correct
        #   part of the configuration, so that later, we can do
        #   config[ key ].Output = modified(output)
        self.key          = None
        self.output       = None
        self.ItemList     = None
        self.OptItemList  = None
        #
        self.wName = writer.getName()
        # Now process the Writer, find where the output is named
        self.woutput     = None
        self.datasvcName = None
        self.svcOutput   = None
        if hasattr( self.w, "Output" ) :
            self.woutput = self.w.Output
            self.getItemLists( config )
            self.set( self.wName, self.w.Output )
            return
        else :
            # if there is no output file, get it via the datasvc
            # (every writer always has a datasvc property)
            self.datasvcName = self.w.EvtDataSvc
            datasvc = config[ self.datasvcName ]
            if hasattr( datasvc, "Output" ) :
                self.getItemLists( config )
                self.set( self.datasvcName, datasvc.Output )
                return

    def getNewName( self, replaceThis, withThis, extra='' ) :
        # replace one pattern in the output name string
        #  with another, and return the Output name
        # It *might* be in a list, so check for this
        #
        # @param extra : might need to add ROOT flags
        #                i.e.: OPT='RECREATE', or such
        assert replaceThis.__class__.__name__ == 'str'
        assert    withThis.__class__.__name__ == 'str'
        old = self.output
        lst = False
        if old.__class__.__name__ == 'list' :
            old = self.output[0]
            lst = True
        new = old.replace( replaceThis, withThis )
        new += extra
        if lst :
            return [ new ]
        else :
            return new

    def getItemLists( self, config ) :
        # the item list
        if hasattr( self.w, "ItemList" ) :
            self.ItemList = self.w.ItemList
        else :
            datasvc = config[ self.w.EvtDataSvc ]
            if hasattr( datasvc, "ItemList" ) :
                self.ItemList = datasvc.ItemList
        # The option item list; possibly not a valid variable
        if hasattr( self.w, "OptItemList" ) :
            self.OptItemList = self.w.OptItemList
        else :
            datasvc = config[ self.w.EvtDataSvc ]
            if hasattr( datasvc, "OptItemList" ) :
                self.OptItemList = datasvc.OptItemList
        return

    def set( self, key, output ) :
        self.key         = key
        self.output      = output
        return

    def __repr__( self ) :
        s  = ""
        line = '-'*80
        s += (line+'\n')
        s += "Writer         : %s\n"%( self.wName  )
        s += "Writer Type    : %s\n"%( self.wType  )
        s += "Writer Output  : %s\n"%( self.output )
        s += "DataSvc        : %s\n"%( self.datasvcName )
        s += "DataSvc Output : %s\n"%( self.svcOutput   )
        s += '\n'
        s += "Key for config : %s\n"%( self.key    )
        s += "Output File    : %s\n"%( self.output )
        s += "ItemList       : %s\n"%( self.ItemList )
        s += "OptItemList    : %s\n"%( self.OptItemList )
        s += (line+'\n')
        return s

# =============================================================================

class CollectHistograms( PyAlgorithm ) :
    '''
    GaudiPython algorithm used to clean up histos on the Reader and Workers
    Only has a finalize method()
    This retrieves a dictionary of path:histo objects and sends it to the
    writer.  It then waits for a None flag : THIS IS IMPORTANT, as if
    the algorithm returns before ALL histos have been COMPLETELY RECEIVED
    at the writer end, there will be an error.
    '''
    def __init__( self, gmpcomponent ) :
        PyAlgorithm.__init__( self )
        self._gmpc = gmpcomponent
        self.log = self._gmpc.log
        return None
    def execute( self ) :
        return SUCCESS
    def finalize( self ) :
	self.log.info('CollectHistograms Finalise (%s)'%(self._gmpc.nodeType))
        self._gmpc.hDict = self._gmpc.dumpHistograms( )
        ks = self._gmpc.hDict.keys()
        self.log.info('%i Objects in Histogram Store'%(len(ks)))

        # crashes occurred due to Memory Error during the sending of hundreds
        # histos on slc5 machines, so instead, break into chunks
        # send 100 at a time
        chunk = 100
        reps = len(ks)/chunk + 1
        for i in xrange(reps) :
            someKeys = ks[i*chunk : (i+1)*chunk]
            smalld = dict( [(key, self._gmpc.hDict[key]) for key in someKeys] )
            self._gmpc.hq.put( (self._gmpc.nodeID, smalld) )
        # "finished" Notification
	self.log.debug('Signalling end of histos to Writer')
        self._gmpc.hq.put( 'HISTOS_SENT' )
        self.log.debug( 'Waiting on Sync Event' )
        self._gmpc.sEvent.wait()
        self.log.debug( 'Histo Sync Event set, clearing and returning' )
        self._gmpc.hvt.clearStore()
        root = gbl.DataObject()
        setOwnership(root, False)
        self._gmpc.hvt.setRoot( '/stat', root )
        return SUCCESS

# =============================================================================

class EventCommunicator( object ) :
    # This class is responsible for communicating Gaudi Events via Queues
    # Events are communicated as TBufferFiles, filled either by the
    # TESSerializer, or the GaudiSvc, "IPCSvc"
    def __init__( self, GMPComponent, qin, qout ) :
        self._gmpc = GMPComponent
        self.log   = self._gmpc.log
        # maximum capacity of Queues
        self.maxsize = 50
        # central variables : Queues
        self.qin  = qin
        self.qout = qout

        # flags
        self.allsent = False
        self.allrecv = False

        # statistics storage
        self.nSent    = 0    # counter : items sent
        self.nRecv    = 0    # counter : items received
        self.sizeSent = 0    # measure : size of events sent ( tbuf.Length() )
        self.sizeRecv = 0    # measure : size of events in   ( tbuf.Length() )
        self.qinTime  = 0    # time    : receiving from self.qin
        self.qoutTime = 0    # time    : sending on qout

    def send( self, item ) :
        # This class manages the sending of a TBufferFile Event to a Queue
        # The actual item to be sent is a tuple : ( evtNumber, TBufferFile )
        assert item.__class__.__name__ == 'tuple'
        startTransmission = time.time()
        self.qout.put( item )
        # allow the background thread to feed the Queue; not 100% guaranteed to
        # finish before next line
        while self.qout._buffer : time.sleep( NAP )
        self.qoutTime += time.time()-startTransmission
        self.sizeSent += item[1].Length()
        self.nSent += 1
        return SUCCESS

    def receive( self, timeout=None ) :
        # Receive items from self.qin
        startWait = time.time()
        try :
            itemIn = self.qin.get( timeout=timeout )
        except Empty :
            return None
        self.qinTime += time.time()-startWait
        self.nRecv += 1
        if itemIn.__class__.__name__ == 'tuple' :
            self.sizeRecv += itemIn[1].Length()
        else :
            self.nRecv -= 1
        try :
            self.qin.task_done()
        except :
            self._gmpc.log.warning('TASK_DONE called too often by : %s'\
                                    %(self._gmpc.nodeType))
        return itemIn

    def finalize( self ) :
        self.log.info('Finalize Event Communicator : %s %s'%(self._gmpc, self._gmpc.nodeType))
        # Reader sends one flag for each worker
        # Workers send one flag each
        # Writer sends nothing (it's at the end of the chain)
        if   self._gmpc.nodeType == 'Reader' : downstream = self._gmpc.nWorkers
        elif self._gmpc.nodeType == 'Writer' : downstream = 0
        elif self._gmpc.nodeType == 'Worker' : downstream = 1

        for i in xrange(downstream) :
            self.qout.put( 'FINISHED' )
        if self._gmpc.nodeType != 'Writer' :
            self.qout.join()
        # Now some reporting...
        self.statistics( )

    def statistics( self ) :
        self.log.name = '%s-%i Audit '%(self._gmpc.nodeType,self._gmpc.nodeID)
        self.log.info ( 'Items Sent     : %i'%(self.nSent) )
        self.log.info ( 'Items Received : %i'%(self.nRecv) )
        self.log.info ( 'Data  Sent     : %i'%(self.sizeSent) )
        self.log.info ( 'Data  Received : %i'%(self.sizeRecv) )
        self.log.info ( 'Q-out Time     : %5.2f'%(self.qoutTime) )
        self.log.info ( 'Q-in  Time     : %5.2f'%(self.qinTime ) )

# =============================================================================

class TESSerializer( object ) :
    def __init__( self, gaudiTESSerializer, evtDataSvc,
                        nodeType, nodeID, log ) :
        self.T   = gaudiTESSerializer
        self.evt = evtDataSvc
        self.buffersIn  = []
        self.buffersOut = []
        self.nIn     = 0
        self.nOut    = 0
        self.tDump   = 0.0
        self.tLoad   = 0.0
        # logging
        self.nodeType = nodeType
        self.nodeID   = nodeID
        self.log      = log
    def Load( self, tbuf ) :
        root = gbl.DataObject()
        setOwnership( root, False )
        self.evt.setRoot( '/Event', root )
        t = time.time()
        self.T.loadBuffer( tbuf )
        self.tLoad   += (time.time() - t)
        self.nIn     += 1
        self.buffersIn.append( tbuf.Length() )
    def Dump( self ) :
        t = time.time()
        tb = TBufferFile( TBuffer.kWrite )
        self.T.dumpBuffer(tb)
        self.tDump += ( time.time()-t )
        self.nOut  += 1
        self.buffersOut.append( tb.Length() )
        return tb
    def Report( self ) :
        evIn       = "Events Loaded    : %i"%( self.nIn  )
        evOut      = "Events Dumped    : %i"%( self.nOut )
        din = sum( self.buffersIn )
        dataIn     = "Data Loaded      : %i"%(din)
        dataInMb   = "Data Loaded (MB) : %5.2f Mb"%(din/MB)
        if self.nIn :
            avgIn      = "Avg Buf Loaded   : %5.2f Mb"\
                          %( din/(self.nIn*MB) )
            maxIn      = "Max Buf Loaded   : %5.2f Mb"\
                          %( max(self.buffersIn)/MB )
        else :
            avgIn      = "Avg Buf Loaded   : N/A"
            maxIn      = "Max Buf Loaded   : N/A"
        dout = sum( self.buffersOut )
        dataOut    = "Data Dumped      : %i"%(dout)
        dataOutMb  = "Data Dumped (MB) : %5.2f Mb"%(dout/MB)
        if self.nOut :
            avgOut     = "Avg Buf Dumped   : %5.2f Mb"\
                          %( din/(self.nOut*MB) )
            maxOut     = "Max Buf Dumped   : %5.2f Mb"\
                          %( max(self.buffersOut)/MB )
        else :
            avgOut     = "Avg Buf Dumped   : N/A"
            maxOut     = "Max Buf Dumped   : N/A"
        dumpTime   = "Total Dump Time  : %5.2f"%( self.tDump )
        loadTime   = "Total Load Time  : %5.2f"%( self.tLoad )

        lines =  evIn     ,\
                 evOut    ,\
                 dataIn   ,\
                 dataInMb ,\
                 avgIn    ,\
                 maxIn    ,\
                 dataOut  ,\
                 dataOutMb,\
                 avgOut   ,\
                 maxOut   ,\
                 dumpTime ,\
                 loadTime
        self.log.name = "%s-%i TESSerializer"%(self.nodeType, self.nodeID)
        for line in lines :
            self.log.info( line )
        self.log.name = "%s-%i"%(self.nodeType, self.nodeID)

# =============================================================================

class GMPComponent( object ) :
    # This class will be the template for Reader, Worker and Writer
    # containing all common components
    # nodeId will be a numerical identifier for the node
    # -1 for reader
    # -2 for writer
    # 0,...,nWorkers-1 for the Workers
    def __init__( self, nodeType, nodeID, queues, events, params, subworkers   ) :
        # declare a Gaudi MultiProcessing Node
        # the nodeType is going to be one of Reader, Worker, Writer
        # qPair is going to be a tuple of ( qin, qout )
        # for sending and receiving
        # if nodeType is "Writer", it will be a list of qPairs,
        # as there's one queue-in from each Worker
        #
        # params is a tuple of (nWorkers, config, log)

        self.nodeType = nodeType
        current_process().name = nodeType

        # Synchronisation Event() objects for keeping track of the system
        self.initEvent, eventLoopSyncer, self.finalEvent = events
        self.eventLoopSyncer, self.lastEvent = eventLoopSyncer   # unpack tuple

        # necessary for knowledge of the system
        self.nWorkers, self.sEvent, self.config, self.log = params
        self.subworkers = subworkers
        self.nodeID   = nodeID

        # describe the state of the node by the current Event Number
        self.currentEvent = None

        # Unpack the Queues : (events, histos, filerecords)
        self.queues = queues
        self.num = 0

        ks = self.config.keys()
        self.app = None
        list = ["Brunel", "DaVinci", "Boole", "Gauss"]
        for k in list:
           if k in ks: self.app = k

    def Start( self ) :
        # define the separate process
        qPair, histq, fq = self.queues

        # Set up the Queue Mechanisms ( Event Communicators )
        if self.nodeType == 'Reader' or self.nodeType == 'Worker' :
            # Reader or Worker Node
            qin, qout = qPair
            self.evcom = EventCommunicator( self, qin, qout )
        else :
            # Writer : many queues in, no queue out
            assert self.nodeType == 'Writer'
            self.evcoms = []
            qsin = qPair[0]
            for q in qsin :
                ec = EventCommunicator( self, q, None )
                self.evcoms.append( ec )
        # Histogram Queue
        self.hq = histq
        # FileRecords Queue
        self.fq = fq

        # Universal Counters (available to all nodes)
        # Use sensibly!!!
        self.nIn  = 0
        self.nOut = 0

        # Status Flag (possibly remove later)
        self.stat = SUCCESS

        # Set logger name
        self.log.name = '%s-%i'%(self.nodeType, self.nodeID)

        # Heuristic variables
        # time for init, run, final, firstEventTime, totalTime
        self.iTime       = 0.0
        self.rTime       = 0.0
        self.fTime       = 0.0
        self.firstEvTime = 0.0
        self.tTime       = 0.0

        self.proc = Process( target=self.Engine )
        # Fork and start the separate process
        self.proc.start()




    def Engine( self ) :
        # This will be the method carried out by the Node
        # Different for all
        pass

    def processConfiguration( self ) :
        # Different for all ; customize Configuration for multicore
        pass

    def SetupGaudiPython( self ) :
        # This method will initialize the GaudiPython Tools
        # such as the AppMgr and so on
        self.a   = AppMgr()
        if SMAPS :
            from AlgSmapShot import SmapShot
            smapsLog = self.nodeType+'-'+str(self.nodeID)+'.smp'
            ss = SmapShot( logname=smapsLog )
            self.a.addAlgorithm( ss )
        self.evt = self.a.evtsvc()
        self.hvt = self.a.histsvc()
        self.fsr = self.a.filerecordsvc()
        self.inc = self.a.service('IncidentSvc','IIncidentSvc')
        self.pers = self.a.service( 'EventPersistencySvc', 'IAddressCreator' )
        self.ts  = gbl.GaudiMP.TESSerializer( self.evt._idp, self.pers )
        self.TS  = TESSerializer( self.ts, self.evt,
                                  self.nodeType, self.nodeID, self.log )
        return SUCCESS

    def StartGaudiPython( self ) :
        self.a.initialize()
        self.a.start()
        return SUCCESS

    def LoadTES( self, tbufferfile ) :
        root = gbl.DataObject()
        setOwnership(root, False)
        self.evt.setRoot( '/Event', root )
        self.ts.loadBuffer(tbufferfile)

    def getEventNumber( self ) :
      if self.app != 'Gauss':
        # Using getList or getHistoNames can result in the EventSelector
        # re-initialising connection to RootDBase, which costs a lot of
        # time... try to build a set of Header paths??

        # First Attempt : Unpacked Event Data
        lst = [ '/Event/Gen/Header',
                '/Event/Rec/Header' ]
        for l in lst :
            path = l
            try :
                n = self.evt[path].evtNumber()

                return n
            except :
                # No evt number at this path
                continue

        # second attepmt : try DAQ/RawEvent data
        # The Evt Number is in bank type 16, bank 0, data pt 4
        try :
            n = self.evt['/Event/DAQ/RawEvent'].banks(16)[0].data()[4]

            return n
        except :
            pass

        # Default Action
        if self.nIn > 0 or self.nOut > 0 :
            pass
        else :
            self.log.warning('Could not determine Event Number')
        return -1
      else:
           if self.nodeID == -1:
             self.num = self.num + 1

             return self.num

    def IdentifyWriters( self ) :
        #
        # Identify Writers in the Configuration
        #
        d = {}
        keys = [ "events", "records", "tuples", "histos" ]
        for k in keys :
            d[k] = []

        # Identify Writers and Classify
        wkeys = WRITERTYPES.keys()
        for v in self.config.values() :
            if v.__class__.__name__ in wkeys :
                writerType = WRITERTYPES[ v.__class__.__name__ ]
                d[writerType].append( MiniWriter(v, writerType, self.config) )
                if self.nodeID == 0 :
                    self.log.info('Writer Found : %s'%(v.name()))

        # Now Check for the Histogram Service
        if 'HistogramPersistencySvc' in  self.config.keys() :
            hfile =self.config['HistogramPersistencySvc'].getProp('OutputFile')
            d[ "histos" ].append( hfile )
        return d

    def dumpHistograms( self ) :
        '''
        Method used by the GaudiPython algorithm CollectHistos
        to obtain a dictionary of form { path : object }
        representing the Histogram Store
        '''
        nlist = self.hvt.getHistoNames( )
        histDict = {}
        objects = 0 ; histos = 0
        if nlist :
            for n in nlist :
                o = self.hvt[ n ]
                if type(o) in aidatypes :
                    o = aida2root(o)
                    histos  += 1
                else :
                    objects += 1
                histDict[ n ] = o
        else :
            print 'WARNING : no histograms to recover?'
        return histDict

    def Initialize( self ) :
        start = time.time()
        self.processConfiguration( )
        self.SetupGaudiPython( )
        self.initEvent.set()
        self.StartGaudiPython( )

        if self.app == 'Gauss':

            tool = self.a.tool( "ToolSvc.EvtCounter" )
            self.cntr = InterfaceCast( gbl.IEventCounter )( tool.getInterface() )
        else:
            self.cntr = None

        self.iTime = time.time() - start

    def Finalize( self ) :
        start = time.time()
        self.a.stop()
        self.a.finalize()
        self.log.info( '%s-%i Finalized'%(self.nodeType, self.nodeID) )
        self.finalEvent.set()
        self.fTime = time.time() - start

    def Report( self ) :
        self.log.name = "%s-%i Audit"%(self.nodeType, self.nodeID)
        allTime  = "Alive Time     : %5.2f"%(self.tTime)
        initTime = "Init Time      : %5.2f"%(self.iTime)
        frstTime = "1st Event Time : %5.2f"%(self.firstEvTime)
        runTime  = "Run Time       : %5.2f"%(self.rTime)
        finTime  = "Finalise Time  : %5.2f"%(self.fTime)
        tup = ( allTime, initTime, frstTime, runTime, finTime )
        for t in tup :
            self.log.info( t )
        self.log.name = "%s-%i"%(self.nodeType, self.nodeID)
        # and report from the TESSerializer
        self.TS.Report()

# =============================================================================

class Reader( GMPComponent )  :
    def __init__( self, queues, events, params, subworkers  ) :
        GMPComponent.__init__(self, 'Reader', -1, queues, events, params, subworkers  )

    def processConfiguration( self ) :
        # Reader :
        #   No algorithms
        #   No output
        #   No histos
        self.config[ 'ApplicationMgr' ].TopAlg    = []
        self.config[ 'ApplicationMgr' ].OutStream = []
        if "HistogramPersistencySvc" in self.config.keys() :
            self.config[ 'HistogramPersistencySvc' ].OutputFile = ''
        self.config['MessageSvc'].Format    = '[Reader]% F%18W%S%7W%R%T %0W%M'
        self.evtMax = self.config[ 'ApplicationMgr' ].EvtMax

    def DumpEvent( self ) :
        tb = TBufferFile( TBuffer.kWrite )
        # print '----Reader dumping Buffer!!!'
        self.ts.dumpBuffer( tb )
        # print '\tBuffer Dumped, size : %i'%( tb.Length() )
        return tb

    def DoFirstEvent( self ) :
        # Do First Event ------------------------------------------------------
        # Check Termination Criteria
        startFirst = time.time()
        self.log.info('Reader : First Event')
        if self.nOut == self.evtMax :
            self.log.info('evtMax( %i ) reached'%(self.evtMax))
            self.lastEvent.set()
            return SUCCESS
        else :
            # Continue to read, dump and send event
            self.a.run(1)
            if not bool(self.evt['/Event']) :
                self.log.warning('No More Events! (So Far : %i)'%(self.nOut))
                self.lastEvent.set()
                return SUCCESS
            else :
                # Popluate TESSerializer list and send Event
                if self.app == "Gauss":
                   lst = self.evt.getHistoNames()
                else:
                  try :
                    lst = self.evt.getList()
                    if self.app == "DaVinci":
                      daqnode = self.evt.retrieveObject( '/Event/DAQ' ).registry()
                      setOwnership( daqnode, False )
                      self.evt.getList( daqnode, lst, daqnode.address().par() )
                  except :
                    self.log.critical('Reader could not acquire TES List!')
                    self.lastEvent.set()
                    return FAILURE
                self.log.info('Reader : TES List : %i items'%(len(lst)))
                for l in lst :
                    self.ts.addItem(l)
                self.currentEvent = self.getEventNumber( )
                tb = self.TS.Dump( )
                self.log.info('First Event Sent')
                self.evcom.send( (self.currentEvent, tb) )
                self.nOut += 1
                self.eventLoopSyncer.set()
                self.evt.clearStore( )
                self.firstEvTime = time.time()-startFirst
                return SUCCESS

    def Engine( self ) :
        # rename process
        import os
        import ctypes
        libc = ctypes.CDLL('libc.so.6')
        name = str(self.nodeType) + str(self.nodeID) + '\0'
        libc.prctl(15,name,0,0,0)


        startEngine = time.time()
        self.log.name = 'Reader'
        self.log.info('Reader Process starting')

        self.Initialize()

        # add the Histogram Collection Algorithm
        self.a.addAlgorithm( CollectHistograms(self) )

        self.log.info('Reader Beginning Distribution')
        sc = self.DoFirstEvent( )
        if sc.isSuccess() :
            self.log.info('Reader First Event OK')
        else :
            self.log.critical('Reader Failed on First Event')
            self.stat = FAILURE

        # Do All Others -------------------------------------------------------
        while True :
            # Check Termination Criteria
            if self.nOut == self.evtMax :
                self.log.info('evtMax( %i ) reached'%(self.evtMax))
                break
            # Check Health
            if not self.stat.isSuccess() :
                self.log.critical( 'Reader is Damaged!' )
                break
            # Continue to read, dump and send event
            t = time.time()
            self.a.run(1)
            self.rTime += (time.time()-t)
            if not bool(self.evt['/Event']) :
                self.log.warning('No More Events! (So Far : %i)'%(self.nOut))
                break
            self.currentEvent = self.getEventNumber( )
            tb = self.TS.Dump( )
            self.evcom.send( (self.currentEvent, tb) )
            # clean up
            self.nOut += 1
            self.eventLoopSyncer.set()
            self.evt.clearStore( )
        self.log.info('Setting <Last> Event')
        self.lastEvent.set()

        # Finalize
        self.log.info( 'Reader : Event Distribution complete.' )
        self.evcom.finalize()
        self.Finalize()
        self.tTime = time.time() - startEngine
        self.Report()

# =============================================================================
class Subworker(GMPComponent):
    def __init__( self, workerID, queues, events, params, subworkers  ) :
        GMPComponent.__init__(self,'Worker', workerID, queues, events, params, subworkers )
        # Identify the writer streams
        self.writerDict = self.IdentifyWriters( )
        # Identify the accept/veto checks for each event
        self.acceptAlgs, self.requireAlgs, self.vetoAlgs = self.getCheckAlgs()
        self.log.info("Subworker-%i Created OK"%(self.nodeID))
        self.eventOutput = True

    def Engine( self ) :
        # rename process
        import os
        import ctypes
        libc = ctypes.CDLL('libc.so.6')
        name = str(self.nodeType) + str(self.nodeID) + '\0'
        libc.prctl(15,name,0,0,0)

        self.initEvent.set()
        startEngine = time.time()
        msg = self.a.service('MessageSvc')
        msg.Format = '[' + self.log.name + '] % F%18W%S%7W%R%T %0W%M'

        self.log.name = "Worker-%i"%(self.nodeID)
        self.log.info("Subworker %i starting Engine"%(self.nodeID))
        self.filerecordsAgent = FileRecordsAgent(self)

        # populate the TESSerializer itemlist
        self.log.info('EVT WRITERS ON WORKER : %i'\
                       %( len(self.writerDict['events'])))

        nEventWriters = len( self.writerDict[ "events" ] )
        self.a.addAlgorithm( CollectHistograms(self) )

        # Begin processing
        Go = True
        while Go :
            packet = self.evcom.receive( )
            if packet : pass
            else      : continue
            if packet == 'FINISHED' : break
            evtNumber, tbin = packet    # unpack
            if self.cntr != None:

                self.cntr.setEventCounter( evtNumber )

            self.nIn += 1
            self.TS.Load( tbin )

            t = time.time()
            sc = self.a.executeEvent()
            if self.nIn == 1 :
                self.firstEvTime = time.time()-t
            else :
                self.rTime += (time.time()-t)
            if sc.isSuccess() :
                pass
            else :
	        self.log.name = "Worker-%i"%(self.nodeID)
                self.log.warning('Did not Execute Event')
                self.evt.clearStore()
                continue
            if self.isEventPassed() :
                pass
            else :
                self.log.name = "Worker-%i"%(self.nodeID)
                self.log.warning( 'Event did not pass : %i'%(evtNumber) )
                self.evt.clearStore()
                continue
            if self.eventOutput :
                # It may be the case of generating Event Tags; hence
                #   no event output
                self.currentEvent = self.getEventNumber( )
                tb = self.TS.Dump( )
                self.evcom.send( (self.currentEvent, tb) )
                self.nOut += 1
            self.inc.fireIncident(gbl.Incident('Subworker','EndEvent'))
            self.eventLoopSyncer.set()
            self.evt.clearStore( )
        self.log.name = "Worker-%i"%(self.nodeID)
        self.log.info('Setting <Last> Event %s' %(self.nodeID))
        self.lastEvent.set()

        self.evcom.finalize()
        # Now send the FileRecords and stop/finalize the appMgr
        self.filerecordsAgent.SendFileRecords()
        self.tTime = time.time()-startEngine
        self.Finalize()
        self.Report()
        #self.finalEvent.set()

    def SetServices(self,a, evt, hvt, fsr, inc, pers, ts , cntr):
        self.a = a
        self.evt = evt
        self.hvt = hvt
        self.fsr = fsr
        #self.inc = inc
        self.inc = self.a.service('IncidentSvc','IIncidentSvc')
        self.pers = pers
        self.ts  = ts
        self.cntr = cntr
        self.TS  = TESSerializer( self.ts, self.evt,
                                  self.nodeType, self.nodeID, self.log )


    def getCheckAlgs( self ) :
        '''
        For some output writers, a check is performed to see if the event has
        executed certain algorithms.
        These reside in the AcceptAlgs property for those writers
        '''
        acc = []
        req = []
        vet = []
        for m in self.writerDict[ "events" ] :
            if hasattr(m.w, 'AcceptAlgs')  : acc += m.w.AcceptAlgs
            if hasattr(m.w, 'RequireAlgs') : req += m.w.RequireAlgs
            if hasattr(m.w, 'VetoAlgs')    : vet += m.w.VetoAlgs
        return (acc, req, vet)


    def checkExecutedPassed( self, algName ) :
        if  self.a.algorithm( algName )._ialg.isExecuted()\
        and self.a.algorithm( algName )._ialg.filterPassed() :
            return True
        else :
            return False

    def isEventPassed( self ) :
        '''
        Check the algorithm status for an event.
        Depending on output writer settings, the event
          may be declined based on various criteria.
        This is a transcript of the check that occurs in GaudiSvc::OutputStream
        '''
        passed = False

        self.log.debug('self.acceptAlgs is %s'%(str(self.acceptAlgs)))
        if self.acceptAlgs :
            for name in self.acceptAlgs :
                if self.checkExecutedPassed( name ) :
                    passed = True
                    break
        else :
            passed = True

        self.log.debug('self.requireAlgs is %s'%(str(self.requireAlgs)))
        for name in self.requireAlgs :
            if self.checkExecutedPassed( name ) :
                pass
            else :
                self.log.info('Evt declined (requireAlgs) : %s'%(name) )
                passed = False

        self.log.debug('self.vetoAlgs is %s'%(str(self.vetoAlgs)))
        for name in self.vetoAlgs :
            if self.checkExecutedPassed( name ) :
                pass
            else :
                self.log.info( 'Evt declined : (vetoAlgs) : %s'%(name) )
                passed = False
        return passed

# =============================================================================
class Worker( GMPComponent ) :
    def __init__( self, workerID, queues, events, params , subworkers ) :
        GMPComponent.__init__(self,'Worker', workerID, queues, events, params, subworkers )
        # Identify the writer streams
        self.writerDict = self.IdentifyWriters( )
        # Identify the accept/veto checks for each event
        self.acceptAlgs, self.requireAlgs, self.vetoAlgs = self.getCheckAlgs()
        self.log.name = "Worker-%i"%(self.nodeID)
        self.log.info("Worker-%i Created OK"%(self.nodeID))
        self.eventOutput = True

    def processConfiguration( self ) :

        # Worker :
        #   No input
        #   No output
        #   No Histos
        self.config[ 'EventSelector'  ].Input     = []
        self.config[ 'ApplicationMgr' ].OutStream = []
        if "HistogramPersistencySvc" in self.config.keys() :
            self.config[ 'HistogramPersistencySvc' ].OutputFile = ''
        formatHead = '[Worker-%i] '%(self.nodeID)
        self.config['MessageSvc'].Format = formatHead+'% F%18W%S%7W%R%T %0W%M'

        for key, lst in self.writerDict.iteritems() :
            self.log.info( 'Writer Type : %s\t : %i'%(key, len(lst)) )

        for m in self.writerDict[ "tuples" ] :
            # rename Tuple output file with an appendix
            # based on worker id, for merging later
            newName = m.getNewName( '.', '.w%i.'%(self.nodeID) )
            self.config[ m.key ].Output = newName

        # Suppress INFO Output for all but Worker-0
        #if self.nodeID == 0 :
        #    pass
        #else                :
        #    self.config[ 'MessageSvc' ].OutputLevel = ERROR

        if self.app == "Gauss":
          try:
            if "ToolSvc.EvtCounter" not in self.config:
                from Configurables import EvtCounter
                counter = EvtCounter()
            else:
                counter = self.config["ToolSvc.EvtCounter"]
            counter.UseIncident = False
          except:
            # ignore errors when trying to change the configuration of the EvtCounter
            self.log.warning('Cannot configure EvtCounter')

    def Engine( self ) :

        # rename process
        import os
        import ctypes
        libc = ctypes.CDLL('libc.so.6')
        name = str(self.nodeType) + str(self.nodeID) + '\0'
        libc.prctl(15,name,0,0,0)

        startEngine = time.time()
        self.log.info("Worker %i starting Engine"%(self.nodeID))
        self.Initialize()
        self.filerecordsAgent = FileRecordsAgent(self)

        # populate the TESSerializer itemlist
        self.log.info('EVT WRITERS ON WORKER : %i'\
                       %( len(self.writerDict['events'])))

        nEventWriters = len( self.writerDict[ "events" ] )
        if nEventWriters :
            itemList = set()
            optItemList = set()
            for m in self.writerDict[ "events" ] :
                for item in m.ItemList :
                    hsh = item.find( '#' )
                    if hsh != -1:
                      item = item[:hsh]
                    itemList.add( item )
                for item in m.OptItemList :
                    hsh = item.find( '#' )
                    if hsh != -1:
                      item = item[:hsh]
                    optItemList.add( item )
            # If an item is mandatory and optional, keep it only in the optional list
            itemList -= optItemList
            for item in sorted( itemList ):
                self.log.info( ' adding ItemList Item to ts : %s' % ( item ) )
                self.ts.addItem( item )
            for item in sorted( optItemList ):
                self.log.info( ' adding Optional Item to ts : %s' % ( item ) )
                self.ts.addOptItem( item )
        else :
            self.log.info( 'There is no Event Output for this app' )
            self.eventOutput = False

        # Begin processing
        Go = True
        while Go :
            packet = self.evcom.receive( )
            if packet : pass
            else      : continue
            if packet == 'FINISHED' : break
            evtNumber, tbin = packet    # unpack
            if self.cntr != None:
                self.cntr.setEventCounter( evtNumber )

            # subworkers are forked before the first event is processed
            # reader-thread for ConDB must be closed and reopened in each subworker
            # this is done by disconnect()
            if self.nIn == 0:

                self.log.info("Fork new subworkers and disconnect from CondDB")
                condDB = self.a.service('CondDBCnvSvc', gbl.ICondDBReader)
                condDB.disconnect()

                # Fork subworkers and share services
                for k in self.subworkers:
                   k.SetServices(self.a, self.evt, self.hvt, self.fsr, self.inc, self.pers, self.ts, self.cntr)
                   k.Start()
                   self.a.addAlgorithm( CollectHistograms(self) )
            self.nIn += 1
            self.TS.Load( tbin )

            t = time.time()
            sc = self.a.executeEvent()
            if self.nIn == 1 :
                self.firstEvTime = time.time()-t
            else :
                self.rTime += (time.time()-t)
            if sc.isSuccess() :
                pass
            else :
                self.log.warning('Did not Execute Event')
                self.evt.clearStore()
                continue
            if self.isEventPassed() :
                pass
            else :
                self.log.warning( 'Event did not pass : %i'%(evtNumber) )
                self.evt.clearStore()
                continue
            if self.eventOutput :
                # It may be the case of generating Event Tags; hence
                #   no event output
                self.currentEvent = self.getEventNumber( )
                tb = self.TS.Dump( )
                self.evcom.send( (self.currentEvent, tb) )
                self.nOut += 1
            self.inc.fireIncident(gbl.Incident('Worker','EndEvent'))
            self.eventLoopSyncer.set()
            self.evt.clearStore( )
        self.log.info('Setting <Last> Event')
        self.lastEvent.set()

        self.evcom.finalize()
        self.log.info( 'Worker-%i Finished Processing Events'%(self.nodeID) )
        # Now send the FileRecords and stop/finalize the appMgr
        self.filerecordsAgent.SendFileRecords()
        self.Finalize()
        self.tTime = time.time()-startEngine
        self.Report()

        for k in self.subworkers:
            self.log.info('Join subworkers')
            k.proc.join()

    def getCheckAlgs( self ) :
        '''
        For some output writers, a check is performed to see if the event has
        executed certain algorithms.
        These reside in the AcceptAlgs property for those writers
        '''
        acc = []
        req = []
        vet = []
        for m in self.writerDict[ "events" ] :
            if hasattr(m.w, 'AcceptAlgs')  : acc += m.w.AcceptAlgs
            if hasattr(m.w, 'RequireAlgs') : req += m.w.RequireAlgs
            if hasattr(m.w, 'VetoAlgs')    : vet += m.w.VetoAlgs
        return (acc, req, vet)


    def checkExecutedPassed( self, algName ) :
        if  self.a.algorithm( algName )._ialg.isExecuted()\
        and self.a.algorithm( algName )._ialg.filterPassed() :
            return True
        else :
            return False

    def isEventPassed( self ) :
        '''
        Check the algorithm status for an event.
        Depending on output writer settings, the event
          may be declined based on various criteria.
        This is a transcript of the check that occurs in GaudiSvc::OutputStream
        '''
        passed = False

        self.log.debug('self.acceptAlgs is %s'%(str(self.acceptAlgs)))
        if self.acceptAlgs :
            for name in self.acceptAlgs :
                if self.checkExecutedPassed( name ) :
                    passed = True
                    break
        else :
            passed = True

        self.log.debug('self.requireAlgs is %s'%(str(self.requireAlgs)))
        for name in self.requireAlgs :
            if self.checkExecutedPassed( name ) :
                pass
            else :
                self.log.info('Evt declined (requireAlgs) : %s'%(name) )
                passed = False

        self.log.debug('self.vetoAlgs is %s'%(str(self.vetoAlgs)))
        for name in self.vetoAlgs :
            if self.checkExecutedPassed( name ) :
                pass
            else :
                self.log.info( 'Evt declined : (vetoAlgs) : %s'%(name) )
                passed = False
        return passed

# =============================================================================

class Writer( GMPComponent ) :
    def __init__( self, queues, events, params, subworkers  ) :
        GMPComponent.__init__(self,'Writer', -2, queues, events, params, subworkers )
        # Identify the writer streams
        self.writerDict = self.IdentifyWriters( )
        # This keeps track of workers as they finish
        self.status = [False]*self.nWorkers
        self.log.name = "Writer--2"

    def processConfiguration( self ) :
        # Writer :
        #   No input
        #   No Algs
        self.config[ 'ApplicationMgr' ].TopAlg = []
        self.config[ 'EventSelector'  ].Input  = []

        self.config['MessageSvc'].Format = '[Writer] % F%18W%S%7W%R%T %0W%M'

        # Now process the output writers
        for key, lst in self.writerDict.iteritems() :
            self.log.info( 'Writer Type : %s\t : %i'%(key, len(lst)) )

        # Modify the name of the output file to reflect that it came
        #  from a parallel processing
        #
        # Event Writers
        for m in self.writerDict[ "events" ] :
            self.log.debug( 'Processing Event Writer : %s'%(m) )
            newName = m.getNewName( '.', '.p%i.'%self.nWorkers )
            self.config[ m.key ].Output = newName

        # Now, if there are no event writers, the FileRecords file
        #   will fail to open, as it only opens an UPDATE version
        #   of the existing Event Output File
        # So, if there are no event writers, edit the string of the
        #   FileRecord Writer

        # FileRecords Writers
        for m in self.writerDict[ "records" ] :
            self.log.debug( 'Processing FileRecords Writer: %s'%(m) )
            newName = m.getNewName( '.', '.p%i.'%self.nWorkers,
                                       extra=" OPT='RECREATE'" )
            self.config[ m.key ].Output = newName

        # same for histos
        hs = "HistogramPersistencySvc"
        n = None
        if hs in self.config.keys() :
            n = self.config[ hs ].OutputFile
        if n :
            newName=self.config[hs].OutputFile.replace('.',\
                                                       '.p%i.'%(self.nWorkers))
            self.config[ hs ].OutputFile = newName

    def Engine( self ) :
        # rename process
        import os
        import ctypes
        libc = ctypes.CDLL('libc.so.6')
        name = str(self.nodeType) + str(self.nodeID) + '\0'
        libc.prctl(15,name,0,0,0)

        startEngine = time.time()
        self.Initialize()
        self.histoAgent = HistoAgent( self )
        self.filerecordsAgent = FileRecordsAgent( self )

        # Begin processing
        Go      = True
        current = -1
        stopCriteria = self.nWorkers
        while Go :
            current = (current+1)%self.nWorkers
            packet = self.evcoms[current].receive( timeout=0.01 )
            if packet == None :
                continue
            if packet == 'FINISHED' :
                self.log.info('Writer got FINISHED flag : Worker %i'%(current))

                self.status[current] = True
                if all(self.status) :
                    self.log.info('FINISHED recd from all workers, break loop')
                    break
                continue
            # otherwise, continue as normal
            self.nIn += 1    # update central count (maybe needed by FSR store)
            evtNumber, tbin = packet    # unpack
            self.TS.Load( tbin )
            t  = time.time()
            self.a.executeEvent()
            self.rTime += ( time.time()-t )
            self.currentEvent = self.getEventNumber( )
            self.evt.clearStore( )
            self.eventLoopSyncer.set()
        self.log.name = "Writer--2"
        self.log.info('Setting <Last> Event')
        self.lastEvent.set()

        # finalisation steps
        [ e.finalize() for e in self.evcoms ]
        # Now do Histograms
        sc = self.histoAgent.Receive()
        sc = self.histoAgent.RebuildHistoStore()
        if sc.isSuccess() : self.log.info( 'Histo Store rebuilt ok' )
        else              : self.log.warning( 'Histo Store Error in Rebuild' )

        # Now do FileRecords
        sc = self.filerecordsAgent.Receive()
        self.filerecordsAgent.Rebuild()
        self.Finalize()
        #self.rTime = time.time()-startEngine
        self.Report()

# =============================================================================



# =============================================================================

class Coord( object ) :
    def __init__( self, nWorkers, config, log ) :

        self.log = log
        self.config = config
        # set up Logging
        self.log.name = 'GaudiPython-Parallel-Logger'
        self.log.info( 'GaudiPython Parallel Process Co-ordinator beginning' )

        if nWorkers == -1 :
            # The user has requested all available cpus in the machine
            self.nWorkers = cpu_count()
        else :
            self.nWorkers = nWorkers


        self.qs = self.SetupQueues( )    # a dictionary of queues (for Events)
        self.hq = JoinableQueue( )       # for Histogram data
        self.fq = JoinableQueue( )       # for FileRecords data

        # Make a Syncer for Initalise, Run, and Finalise
        self.sInit = Syncer( self.nWorkers, self.log,
                             limit=WAIT_INITIALISE,
                             step=STEP_INITIALISE    )
        self.sRun  = Syncer( self.nWorkers, self.log,
                             manyEvents=True,
                             limit=WAIT_SINGLE_EVENT,
                             step=STEP_EVENT,
                             firstEvent=WAIT_FIRST_EVENT )
        self.sFin  = Syncer( self.nWorkers, self.log,
                             limit=WAIT_FINALISE,
                             step=STEP_FINALISE )
        # and one final one for Histogram Transfer
        self.histSyncEvent = Event()

        # params are common to al subprocesses
        params = (self.nWorkers, self.histSyncEvent, self.config, self.log)

        self.subworkers = []
        # Declare SubProcesses!
        for i in range(1, self.nWorkers ) :
            sub = Subworker( i, self.getQueues(i), self.getSyncEvents(i), params, self.subworkers )
            self.subworkers.append( sub )
        self.reader= Reader(self.getQueues(-1), self.getSyncEvents(-1), params, self.subworkers)
        self.workers = []
        wk = Worker( 0, self.getQueues(0), self.getSyncEvents(0), params, self.subworkers )
        self.writer= Writer(self.getQueues(-2), self.getSyncEvents(-2), params, self.subworkers)

        self.system = []
        self.system.append(self.writer)
        self.system.append(wk)
        self.system.append(self.reader)

    def getSyncEvents( self, nodeID ) :
        init = self.sInit.d[nodeID].event
        run  = ( self.sRun.d[nodeID].event, self.sRun.d[nodeID].lastEvent )
        fin  = self.sFin.d[nodeID].event
        return ( init, run, fin )

    def getQueues( self, nodeID ) :
        eventQ = self.qs[ nodeID ]
        histQ  = self.hq
        fsrQ   = self.fq
        return ( eventQ, histQ, fsrQ )

    def Go( self ) :

        # Initialise
        self.log.name = 'GaudiPython-Parallel-Logger'
        self.log.info( 'INITIALISING SYSTEM' )

        # Start reader, writer and main worker
        for p in self.system :
            p.Start()

        sc = self.sInit.syncAll(step="Initialise")
        if sc == SUCCESS: pass
        else  : self.Terminate() ; return FAILURE

        # Run
        self.log.name = 'GaudiPython-Parallel-Logger'
        self.log.info( 'RUNNING SYSTEM' )
        sc = self.sRun.syncAll(step="Run")
        if sc == SUCCESS: pass
        else  : self.Terminate() ; return FAILURE

        # Finalise
        self.log.name = 'GaudiPython-Parallel-Logger'
        self.log.info( 'FINALISING SYSTEM' )
        sc = self.sFin.syncAll(step="Finalise")
        if sc == SUCCESS: pass
        else  : self.Terminate() ; return FAILURE

        # if we've got this far, finally report SUCCESS
        self.log.info( "Cleanly join all Processes" )
        self.Stop()
        self.log.info( "Report Total Success to Main.py" )
        return SUCCESS

    def Terminate( self ) :
        # Brutally kill sub-processes
        children = multiprocessing.active_children()
        for i in children:
            i.terminate()

        #self.writer.proc.terminate()
        #[ w.proc.terminate() for w in self.workers]
        #self.reader.proc.terminate()

    def Stop( self ) :
        # procs should be joined in reverse order to launch
        self.system.reverse()
        for s in self.system :
            s.proc.join()
        return SUCCESS

    def SetupQueues( self ) :
        # This method will set up the network of Queues needed
        # N Queues = nWorkers + 1
        # Each Worker has a Queue in, and a Queue out
        # Reader has Queue out only
        # Writer has nWorkers Queues in

        # one queue from Reader-Workers
        rwk = JoinableQueue()
        # one queue from each worker to writer
        workersWriter = [ JoinableQueue() for i in xrange(self.nWorkers) ]
        d = {}
        d[-1] = (None, rwk)              # Reader
        d[-2] = (workersWriter, None)    # Writer
        for i in xrange(self.nWorkers) : d[i] =  (rwk, workersWriter[i])
        return d

# ============================= EOF ===========================================
