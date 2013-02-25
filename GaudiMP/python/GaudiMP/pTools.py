from GaudiPython import gbl, SUCCESS, FAILURE
from multiprocessing import Event
import pickle, time

# Eoin Smith
# 3 Aug 2010

#
# This script contains the ancillary classes and functions used in the
#  GaudiPython Parallel model
#
# Classes :
# - HistoAgent : In charge of extracting Histograms from their Transient Store
#                 on a reader/worker, communicating them to the writer, and on
#                 the writer receiving them and rebuilding a single store
#
# - FileRecordsAgent : Similar to HistoAgent, but for FileRecords Data
#
# - LumiFSR : FSR data from different workers needs to be carefully merged to
#              replicate the serial version; this class aids in that task by
#              representing an LHCb LumiFSR object as a python class
#
# - PackedCaloHypos : Pythonization of an LHCb class, used for inspecting some
#                      differences in serial and parallel output
#
# - Syncer : This class is responsible for syncing processes for a specified
#             section of execution.  For example, one Syncer object might be
#             syncing Initialisation, one for Running, one for Finalisation.
#             Syncer uses multiprocessing.Event() objects as flags which are
#             visible across the N processes sharing them.
#             IMPORTANT : The Syncer objects in the GaudiPython Parallel model
#              ensure that there is no hanging; they in effect, allow a timeout
#              for Initialisation, Run, Finalise on all processes
#
# - SyncMini : A class wrapper for a multiprocessing.Event() object
#
# Methods :
# - getEventNumber(evt) : pass a valid instance of the GaudiPython TES
#                          ( AppMgr().evtsvc() ) to this to get the current
#                          Event Number as an integer (even from RawEvents!)
#


# used to convert stored histos (in AIDA format) to ROOT format
aida2root = gbl.Gaudi.Utils.Aida2ROOT.aida2root

# =========================== Classes =========================================

class HistoAgent( ) :
    def __init__( self, gmpComponent ) :
        self._gmpc = gmpComponent
        self.hvt = self._gmpc.hvt
        self.histos = []
        self.qin = self._gmpc.hq
        self.log = self._gmpc.log

        # There are many methods for booking Histogram Objects to Histo store
        # here they are collected in a dictionary, with key = a relevant name
        self.bookingDict = {}
        self.bookingDict['DataObject']        = self.bookDataObject
        self.bookingDict['NTuple::Directory'] = self.bookDataObject
        self.bookingDict['NTuple::File']      = self.bookDataObject
        self.bookingDict['TH1D']       = self.bookTH1D
        self.bookingDict['TH2D']       = self.bookTH2D
        self.bookingDict['TH3D']       = self.bookTH3D
        self.bookingDict['TProfile']   = self.bookTProfile
        self.bookingDict['TProfile2D'] = self.bookTProfile2D

    def register( self, tup ) :
        # add a tuple of (worker-id, histoDict) to self.histos
        assert tup.__class__.__name__ == 'tuple'
        self.histos.append( tup )

    def Receive( self ) :
        hstatus = self._gmpc.nWorkers+1    # +1 for the Reader!
        while True :
            tup = self.qin.get()
            if tup == 'HISTOS_SENT' :
                self.log.debug('received HISTOS_SENT message')
                hstatus -= 1
                if not hstatus : break
            else   :
              self.register( tup )
        self._gmpc.sEvent.set()
        self.log.info('Writer received all histo bundles and set sync event')
        return SUCCESS


    def RebuildHistoStore( self ) :
        '''
        Rebuild the Histogram Store from the histos received by Receive()
        If we have a histo which is not in the store,
        book and fill it according to self.bookingDict
        If we have a histo with a matching histo in the store,
        add the two histos, remembering that aida2root must be used on
        the Stored histo for compatibility.
        '''
        errors = 0
        for tup in self.histos :
            workerID, histDict = tup
            added = 0 ; registered = 0; booked = 0

            for n in histDict.keys() :
                o = histDict[ n ]
                obj = self.hvt.retrieve( n )

                if obj :
                    try    :
                        aida2root(obj).Add(o)
                    except :
                        self.log.warning('FAILED TO ADD : %s'%(str(obj)))
                        errors += 1
                    added += 1
                else :

                    if o.__class__.__name__ in self.bookingDict.keys() :
                        try    :
                            self.bookingDict[o.__class__.__name__](n, o)
                        except :
                            self.log.warning('FAILED TO REGISTER : %s\tto%s'\
                                             %(o.__class__.__name__, n))
                            errors += 1
                    else :
                        self.log.warning( 'No booking method for: %s\t%s\t%s'\
                                          %(n,type(o),o.__class__.__name__) )
                        errors += 1
                    booked += 1
        hs = self.hvt.getHistoNames()
        self.log.info( 'Histo Store Rebuilt : ' )
        self.log.info( '  Contains %i objects.'%(len(hs)) )
        self.log.info( '  Errors in Rebuilding : %i'%(errors) )
        return SUCCESS


    def bookDataObject( self, n, o ):
        '''
        Register a DataObject to the Histo Store
        '''
        self._gmpc.hvt.registerObject( n, o )

    def bookTH1D( self, n, o ) :
        '''
        Register a ROOT 1D THisto to the Histo Store
        '''
        obj = self.hvt._ihs.book( n, o.GetTitle(),\
                                     o.GetXaxis().GetNbins(),\
                                     o.GetXaxis().GetXmin(),\
                                     o.GetXaxis().GetXmax() )
        aida2root(obj).Add(o)

    def bookTH2D( self, n, o ) :
        '''
        Register a ROOT 2D THisto to the Histo Store
        '''
        obj = self.hvt._ihs.book( n, o.GetTitle(),\
                                     o.GetXaxis().GetNbins(),\
                                     o.GetXaxis().GetXmin(),\
                                     o.GetXaxis().GetXmax(),\
                                     o.GetYaxis().GetNbins(),\
                                     o.GetYaxis().GetXmin(),\
                                     o.GetYaxis().GetXmax() )
        aida2root(obj).Add(o)

    def bookTH3D( self, n, o ) :
        '''
        Register a ROOT 3D THisto to the Histo Store
        '''
        obj = self.hvt._ihs.book( n, o.GetTitle(),\
                                     o.GetXaxis().GetXbins(),\
                                     o.GetXaxis().GetXmin(),\
                                     o.GetXaxis().GetXmax(),\
                                     o.GetYaxis().GetXbins(),\
                                     o.GetYaxis().GetXmin(),\
                                     o.GetYaxis().GetXmax(),\
                                     o.GetZaxis().GetXbins(),\
                                     o.GetZaxis().GetXmin(),\
                                     o.GetZaxis().GetXmax() )
        aida2root(obj).Add(o)

    def bookTProfile( self, n, o ) :
        '''
        Register a ROOT TProfile to the Histo Store
        '''
        obj = self.hvt._ihs.bookProf( n, o.GetTitle(),\
                                         o.GetXaxis().GetNbins(),\
                                         o.GetXaxis().GetXmin(),\
                                         o.GetXaxis().GetXmax(),\
                                         o.GetOption() )
        aida2root(obj).Add(o)

    def bookTProfile2D( self, n, o ) :
        '''
        Register a ROOT TProfile2D to the Histo Store
        '''
        obj = self.hvt._ihs.bookProf( n, o.GetTitle(),\
                                         o.GetXaxis().GetNbins(),\
                                         o.GetXaxis().GetXmin(),\
                                         o.GetXaxis().GetXmax(),\
                                         o.GetYaxis().GetNbins(),\
                                         o.GetYaxis().GetXmin(),\
                                         o.GetYaxis().GetXmax()  )
        aida2root(obj).Add(o)

# =============================================================================

class FileRecordsAgent( ) :
    def __init__( self, gmpComponent ) :
        self._gmpc = gmpComponent
        self.fsr = self._gmpc.fsr
        self.q   = self._gmpc.fq
        self.log = self._gmpc.log
        self.objectsIn  = []   # used for collecting FSR store objects
        self.objectsOut = []

    def localCmp( self, tupA, tupB ) :
        # sort tuples by a particular element
        # for the sort() method
        ind  = 0
        valA = tupA[ind]
        valB = tupB[ind]
        if   valA<valB : return -1
        elif valA>valB : return  1
        else           : return  0


    def SendFileRecords( self ) :
        # send the FileRecords data as part of finalisation

        # Take Care of FileRecords!
        # There are two main things to consider here
        # 1) The DataObjects in the FileRecords Transient Store
        # 2) The fact that they are Keyed Containers, containing other objects
        #
        # The Lead Worker, nodeID=0, sends everything in the FSR store, as
        #   a completeness guarantee,
        #
        # send in form ( nodeID, path, object)
        self.log.info('Sending FileRecords...')
        lst      = self.fsr.getHistoNames()

        # Check Validity
        if not lst :
            self.log.info('No FileRecords Data to send to Writer.')
            self.q.put( 'END_FSR' )
            return SUCCESS

        # no need to send the root node
        if '/FileRecords' in lst : lst.remove('/FileRecords')

        for l in lst :
            o = self.fsr.retrieveObject( l )
            if hasattr(o, "configureDirectAccess") :
                o.configureDirectAccess()
            # lead worker sends everything, as completeness guarantee
            if self._gmpc.nodeID == 0 :
                self.objectsOut.append( (0, l, pickle.dumps(o)) )
            else :
                # only add the Event Counter
                # and non-Empty Keyed Containers (ignore empty ones)
                if l == '/FileRecords/EventCountFSR' :
                    tup = (self._gmpc.nodeID, l, pickle.dumps(o))
                    self.objectsOut.append( tup )
                else :
                    # It's a Keyed Container
                    assert "KeyedContainer" in o.__class__.__name__
                    nObjects = o.numberOfObjects()
                    if nObjects :
                        self.log.debug("Keyed Container %s with %i objects"\
                                       %(l, nObjects))
                        tup = (self._gmpc.nodeID, l, pickle.dumps(o))
                        self.objectsOut.append( tup )
        self.log.debug('Done with FSR store, just to send to Writer.')

        if self.objectsOut :
            self.log.debug('%i FSR objects to Writer'%(len(self.objectsOut)))
            for ob in self.objectsOut :
                self.log.debug('\t%s'%(ob[0]))
            self.q.put( self.objectsOut )
        else :
            self.log.info('Valid FSR Store, but no data to send to Writer')
        self.log.info('SendFSR complete')
        self.q.put( 'END_FSR' )
        return SUCCESS

    def Receive( self ) :
        # Receive contents of all Workers FileRecords Transient Stores
        self.log.info('Receiving FSR store data...')
        nc = self._gmpc.nWorkers
        while nc > 0 :
            objects = self.q.get( )
            if objects == 'END_FSR' :
                nc -= 1
                continue
                if nc==0 :
                    break
            # but if it's regular objects...
            for o in objects :
                self.objectsIn.append(o)
        # Now sort it by which worker it came from
        # an object is : (nodeID, path, pickledObject)
        self.objectsIn.sort(cmp=self.localCmp)
        self.log.info('All FSR data received')
        return SUCCESS

    def Rebuild( self ) :
        # objects is a list of (path, serializedObject) tuples
        for sourceNode, path, serialob in self.objectsIn :
            self.log.debug('Working with %s'%(path))
            ob = pickle.loads(serialob)
            if hasattr( ob, 'update' ) :
                ob.update()
            if hasattr( ob, 'numberOfObjects' ) :
                nCont = ob.numberOfObjects()
                self.log.debug( '\t %s has containedObjects : %i'%(type(ob).__name__,  nCont) )
            if sourceNode == 0 :
                self.log.debug('Registering Object to : %s'%(path))
                self.fsr.registerObject( path, ob )
            else :
                self.log.debug('Merging Object to : %s'%(path))
                self.MergeFSRobject( sourceNode, path, ob )
        # As RecordStream has been split into Worker and Writer parts, the
        # count for output is wrong... fix that here, as every event received
        # by the writer is written (validation testing occurs on the worker)

        self.log.info('FSR Store Rebuilt.  Correcting EventCountFSR')
        if bool( self.fsr._idp ) :   # There might not be an FSR stream (Gauss)
            ecount  = '/FileRecords/EventCountFSR'
            if self.fsr[ecount] :
                self.fsr[ecount].setOutput( self._gmpc.nIn )
                self.log.info( 'Event Counter Output set : %s : %i'\
                               %(ecount, self.fsr[ecount].output()) )
            # Do some reporting
            self.log.debug('FSR store reconstructed!')
            lst = self.fsr.getHistoNames()
            if lst :
                for l in lst :
                    ob = self.fsr.retrieveObject(l)
                    if hasattr( ob, 'configureDirectAccess' ) :
                        ob.configureDirectAccess()
                    if hasattr( ob, 'containedObjects' ) :
                        # if ob.numberOfObjects() :
                        self.log.debug('\t%s (cont. objects : %i)'\
                                       %(l, ob.numberOfObjects()))
                    else :
                        self.log.debug('\t%s'%(l))
        self.log.info('FSR Store fully rebuilt.')
        return SUCCESS

    def MergeFSRobject( self, sourceNode, path, ob ) :
        # Merge Non-Empty Keyed Container from Worker>0
        if path == '/FileRecords/TimeSpanFSR' :
            # TimeSpanFSR is a straightforward case
            self.ProcessTimeSpanFSR( path, ob )
        elif path == '/FileRecords/EventCountFSR' :
            # Event Counter is also easy
            self.ProcessEventCountFSR( path, ob )
        # now other cases may not be so easy...
        elif "KeyedContainer" in ob.__class__.__name__ :
            # Keyed Container of LumiFSRs : extract and re-register
            # self.ProcessLumiFSR( path, ob )
            if "LumiFSR" in ob.__class__.__name__ :
                self.MergeLumiFSR( path, ob )
            else:
                self.log.info("Skipping Merge of Keyed Container %s for %s"\
                              %(ob.__class__.__name__,path))

    def ProcessTimeSpanFSR( self, path, ob ) :
        ob2 = self.fsr.retrieveObject( path )
        if ob.containedObjects().size() :
            sz = ob.containedObjects().size()
            cob = ob2.containedObjects()[0]
            min = cob.earliest()
            max = cob.latest()
            for j in xrange( sz ) :
                cob = ob.containedObjects()[j]
                self.log.debug( 'Adding TimeSpanFSR' )
                if cob.earliest() < min:
                  min = cob.earliest()
                if cob.latest() > max:
                  max = cob.latest()
                # this is annoying: it has to be rebuilt, without a key & added
                continue
            tsfsr = gbl.LHCb.TimeSpanFSR()
            tsfsr.setEarliest( min )
            tsfsr.setLatest( max )
            self.fsr[path].clear()
            self.fsr[path].add( tsfsr )

    def ProcessEventCountFSR( self, path, ob ) :
        self.log.debug('Event Count Input Addition')
        self.fsr[path].setInput( self.fsr[path].input()+ob.input() )

    def MergeLumiFSR( self, path, keyedC ) :
        from ROOT import string
        # Fetch the first lumi
        keyedContainer = self.fsr.retrieveObject(path)
        # The LumiFSR KeyedContainers only have one object
        assert keyedContainer.numberOfObjects() == 1
        l = keyedContainer.containedObject(0)
        baseLumi = LumiFSR( l )
        # Now deal with the argument Non-empty Keyed Container of LumiFSRs
        nCont = keyedC.numberOfObjects()
        for i in xrange(nCont) :
            obj = keyedC.containedObject(i)
            nextLumi = LumiFSR( obj )
            baseLumi.merge( nextLumi )
        # Now Rebuild and ReRegister
        newLumi = gbl.LHCb.LumiFSR()
        for r in baseLumi.runs :
            newLumi.addRunNumber( r )
        for f in baseLumi.files :
            newLumi.addFileID( string(f) )
        for k in baseLumi.keys :
            increment, integral = baseLumi.info[k]
            newLumi.addInfo(k, increment, integral)
        # clear existing Keyed Container
        self.fsr[path].clear()
        # Add newly merged lumiFSR
        self.fsr[path].add(newLumi)
        return SUCCESS

# =============================================================================

class LumiFSR( ) :
    def __init__(self, lumi) :
        # lumi looks like :
        # {  runs : 69857 69858
        #    files : root:/castor/cer.../069857_0000000006.raw
        #    info (key/incr/integral) : 0 8 0 / 1 8 259 / 2 8 76 ... }

        # class variables
        self.runs  = []
        self.files = []
        self.info  = {}
        self.keys  = []

        # get run numbers
        for r in lumi.runNumbers() :
            self.runs.append(r)
        # get file ids
        for f in lumi.fileIDs() :
            self.files.append(f)
        # Now the tricky bit, the info is not accessible via Python
        # except as a string
        s = str(lumi)
        sa = s.split("info (key/incr/integral) : ")[-1]
        sa = sa.split('/')[:-1]
        for rec in sa :
            k,i,t = rec.split()
            k = int(k)
            i = int(i)
            t = int(t)
            self.info[k] = (i,t)
        self.keys = self.info.keys()
    def merge( self, otherLumi ) :
        assert otherLumi.__class__.__name__ == "LumiFSR"
        # add any extra runs
        for r in otherLumi.runs :
            if r in self.runs :
                pass
            else :
                self.runs.append( r )
        self.runs.sort()
        # add any extra fileIDs
        for f in otherLumi.files :
            if f in self.files :
                pass
            else :
                self.files.append( f )
        self.files.sort()
        # Now add any extra records
        for k in otherLumi.keys :
            increment, integral = otherLumi.info[k]
            if k in self.keys :
                myIncrement, myIntegral = self.info[k]
                self.info[k] = ( myIncrement+increment, myIntegral+integral )
            else :
                self.info[k] = ( increment, integral )
        # don't forget to update keys
        self.keys = self.info.keys()
    def __repr__( self ) :
        s  = "LumiFSR Python class\n"
        s += "\tRuns : \n"
        for r in self.runs :
            s += "\t\t%i\n"%(r)
        s +=  "\tFiles : \n"
        for f in self.files :
            s += "\t\t%s\n"%(f)
        s += "\tInfo : \n"
        for k in self.keys :
            increment, integral = self.info[k]
            s += "\t\t%i\t%i\t%i\n"%(k,increment,integral)
        return s

# =============================================================================

class PackedCaloHypo() :
    def __init__(self, o) :
      cl = 'LHCb::PackedCaloHypo'
      assert o.__class__.__name__ == cl
      self.centX = o.centX
      self.centY = o.centY
      self.cerr  = (o.cerr00,o.cerr10,o.cerr11)
      self.cov   = (o.cov00,o.cov10,o.cov11,o.cov20,o.cov21,o.cov22)
      self.firstCluster = o.firstCluster
      self.firstDigit   = o.firstDigit
      self.firstHypo    = o.firstHypo
      self.hypothesis   = o.hypothesis
      self.key          = o.key
      self.lastCluster  = o.lastCluster
      self.lastDigit    = o.lastDigit
      self.lastHypo     = o.lastHypo
      self.lh           = o.lh
      self.pos          = (o.posE, o.posX, o.posY)
      self.z            = o.z
    def __repr__( self ) :
        s  = "PackedCaloHypo : \n"
        s += "\tcentX        : %s\n"%( str(self.centX) )
        s += "\tcentY        : %s\n"%( str(self.centY) )
        s += "\tcerr         : %s\n"%( str(self.cerr ) )
        s += "\tcov          : %s\n"%( str(self.cov  ) )
        s += "\tfirstCluster : %s\n"%( str(self.firstCluster) )
        s += "\tfirstDigit   : %s\n"%( str(self.firstDigit) )
        s += "\tfirstHypo    : %s\n"%( str(self.firstHypo) )
        s += "\thypothesis   : %s\n"%( str(self.hypothesis) )
        s += "\tkey          : %s\n"%( str(self.key) )
        s += "\tlastCluster  : %s\n"%( str(self.lastCluster) )
        s += "\tlastDigit    : %s\n"%( str(self.lastDigit) )
        s += "\tlastHypo     : %s\n"%( str(self.lastHypo) )
        s += "\tlh           : %s\n"%( str(self.lh   ) )
        s += "\tpos          : %s\n"%( str(self.pos  ) )
        s += "\tz            : %s\n"%( str(self.z  ) )
        s += "---------------------------------------\n"
        return s

# =============================================================================

class SyncMini( object ) :
    def __init__( self, event, lastEvent=None ) :
        self.event  = event
        self.t      = 0.0
        self.lastEvent = None
        if lastEvent :
            self.lastEvent = lastEvent
    def check( self ) :
        return self.event.is_set()
    def checkLast( self ) :
        return self.lastEvent.is_set()
    def reset( self ) :
        self.event.clear()
        self.t = time.time()
    def getTime( self ) :
        return self.t
    def set( self ) :
        self.event.set()
    def __repr__( self ) :
        s  = "---------- SyncMini --------------\n"
        s += "    Status : %s\n"%(self.event.is_set())
        s += "         t : %5.2f\n"%(self.t)
        if self.lastEvent :
            s += "Last Event : %s\n"%(self.lastEvent.is_set())
        s += "----------------------------------\n"
        return s

# =============================================================================

class Syncer( object ) :
    def __init__( self, nWorkers, log, manyEvents=False,
                  limit=None, step=None, firstEvent=None ) :
        # Class to help synchronise the sub-processes
        self.limit  = limit
        self.step   = step
        self.d = {}
        self.manyEvents = manyEvents

        for i in xrange(-2, nWorkers) :
            self.d[ i ] = SyncMini( Event(), lastEvent=Event() )
            if self.manyEvents :
                self.limitFirst = firstEvent

        self.keys       = self.d.keys()
        self.nWorkers   = nWorkers
        self.log        = log

    def syncAll( self, step="Not specified" ) :
        # is it this method, or is it the rolling version needed?
        # if so, drop through...

        if self.manyEvents :
            sc = self.syncAllRolling( )
            return sc

        # Regular version ----------------------------
        for i in xrange( 0, self.limit, self.step ) :
            if self.checkAll( ) :
                self.log.info('%s : All procs done @ %i s'%(step,i))
                break
            else :
                time.sleep(self.step)

        # Now the time limit is up... check the status one final time
        if self.checkAll() :
            self.log.info("All processes : %s ok."%(step))
            return SUCCESS
        else :
            self.log.critical('Some process is hanging on : %s'%(step))
            for k in self.keys :
                hangString= "%s : Proc/Stat : %i/%s"%(step,k,self.d[k].check())
                self.log.critical( hangString )
            return FAILURE

    def syncAllRolling( self ) :
        # Keep track of the progress of Event processing
        # Each process syncs after each event, so keep clearing
        #  the sync Event, and re-checking
        # Note the time between True checks too, if the time
        #  between events exceeds singleEvent, this is considered a hang

        # set the initial time
        begin   = time.time()
        firstEv = {}
        timers  = {}
        for k in self.keys :
            self.d[k].reset()
            firstEv[k] = False
            timers[k]  = 0.0

        active = self.keys
        while True :
            # check the status of each sync object
            for k in active :
                sMini = self.d[k]

                if sMini.check() or sMini.checkLast():
                    if sMini.checkLast() and sMini.check() :
                        # if last Event set,then event loop finished
                        active.remove( k )
                        alive = time.time()-begin
                        self.log.info( "Audit : Node %i alive for %5.2f"\
                                       %(k,alive) )
                    else :
                        sMini.reset()
                else :
                    # the event still has not been checked, how long is that?
                    # is it the first Event?
                    wait = time.time()-sMini.getTime()
                    cond = wait > self.limit
                    if not firstEv[k] :
                        cond       = wait > self.limitFirst
                        firstEv[k] = True
                    if cond :
                        # It is hanging!
                        self.log.critical('Single event wait : %5.2f'%(wait))
                        self.processHang()
                        return FAILURE

            # Termination Criteria : if all procs have been removed, we're done
            if self.checkLastEvents() :
                self.log.info('TC met for event loop')
                break
            else :
                # sleep, loop again
                time.sleep(self.step)

        self.log.info("All processes Completed all Events ok")
        return SUCCESS

    def processHang( self ) :
        self.log.critical('Some proc is hanging during Event processing!')
        for k in self.keys :
            self.log.critical( "Proc/Stat : %i / %s"%(k,self.d[k].check()) )
        return

    def checkAll( self ) :
        # Check the status of each Sync object
        # return True or False
        currentStatus = [ mini.check() for mini in self.d.values() ]
        return all( currentStatus )

    def checkLastEvents( self ) :
       # check if all of the lastEvents are set to true in self.d[k][1]
       stat    = [ sMini.checkLast() for sMini in self.d.values() ]
       return all(stat)

# =========================== Methods =========================================

def getEventNumber( evt ) :
    # The class-independent version of the Event Number Retrieval method
    #
    n = None
    # First Attempt : Unpacked Event Data
    lst = [ '/Event/Gen/Header',
            '/Event/Rec/Header' ]
    for l in lst :
        try :
            n = evt[l].evtNumber()
            return n
        except :
            # No evt number at this path
            continue

    # second attepmt : try DAQ/RawEvent data
    # The Evt Number is in bank type 16, bank 0, data pt 4
    try :
        n = evt['/Event/DAQ/RawEvent'].banks(16)[0].data()[4]
        return n
    except :
        pass

    # Default Action
    return n

# ================================= EOF =======================================


