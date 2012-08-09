
from Gaudi.Configuration import *
from GaudiPython import AppMgr, gbl
from ROOT import TFile, TBufferFile, TBuffer
from multiprocessing import Process, Queue
from Configurables import LHCbApp
import sys

#
# loadFile.py
# -----------
# Open a dst file for inspection
#

def checkKeys( name ) :
  # Check the TTree keys in each file
  fname = name[4:] # TFile doesn't need the "PFN:" prefix
  tf = TFile( fname, 'REC' )





importOptions( '$STDOPTS/LHCbApplication.opts' )
#importOptions( '$GAUDIPOOLDBROOT/options/GaudiPoolDbRoot.opts' )
importOptions( '$GAUDICNVROOT/options/Setup.opts' )


OutputStream( "DstWriter" ).Output = ''
HistogramPersistencySvc().OutputFile = ''
MessageSvc( OutputLevel = ERROR )
EventSelector().PrintFreq = 100

ApplicationMgr( OutputLevel = ERROR,
                AppName = 'File Check - Serial vs Parallel' )
                # TopAlg = ['UnpackMCParticle', 'UnpackMCVertex'] )

PAR = 'PARALLEL'
SER = 'SERIAL'

def CompareTrees( pname, sname ) :
  pf = TFile( pname, 'REC' )
  sf = TFile( sname, 'REC' )
  event = '_Event'
  pfks = pf.GetListOfKeys()
  sfks = sf.GetListOfKeys()
  pfkeys = list( [pfk.GetName() for pfk in pfks] ) ; pfkeys.sort()
  sfkeys = list( [sfk.GetName() for sfk in sfks] ) ; sfkeys.sort()
  pMeta = [] ; pEvent = [] ; pOther = []
  for k in pfkeys :
    if   k.startswith( event ) : pEvent.append( k )
    elif k.startswith( '##' )  : pMeta.append( k )
    else : pOther.append( k )
  sMeta = [] ; sEvent = [] ; sOther = []
  for k in sfkeys :
    if   k.startswith( event ) : sEvent.append( k )
    elif k.startswith( '##' )  : sMeta.append( k )
    else : sOther.append( k )

  if pMeta == sMeta : pass
  else             : print 'Meta Data differs'

  if pEvent == sEvent : pass
  else               : print 'Event data differs'

  if pOther != sOther :
    pset = set( pOther )
    sset = set( sOther )
    pExtra = pset - sset
    sExtra = sset - pset
    if pExtra : print 'Extra Data in parallel file : ', pExtra
    if sExtra : print 'Extra Data in serial   file : ', sExtra
    if sExtra or pExtra : print 'Files will have different sizes'
  pf.Close()
  sf.Close()

def switchDict( d ) :
  # switch a dictionary around ;  make the values the keys, and vice versa
  # only works if all values are unique
  nkeys = len( d.keys() )
  vals = d.values()
  nvals = len( vals )
  for v in vals :
    if vals.count( v ) > 1 :
      print 'Dictionary cannot be switched, values not unique'
      return None
  print 'Dict has keys/values : %i/%i' % ( nkeys, nvals )
  pairs = d.items() # returns (key, val) tuples in a list
  newd = {}
  for k, entry in pairs : newd[entry] = k
  return newd


def printDict( d, name = 'unspecified' ) :
  # Print out a dictionary in the form
  #
  # Dictionary Name :
  #   key     value
  #   key     value
  #   ...
  #
  print '-' * 80
  print 'Dictionary %s : ' % ( name )
  for k in iter( d.keys() ) :
    print '\t', k, '\t', d[k]
  print '-' * 80


def Reader( readerType, filename, qacross, qToEngine ) :
  #
  # Process for reading a file
  # One process for reading Serial File, another for Parallel File
  #
  # First the order of events is determined, (parallel != serial, usually)
  #
  # Then the events are run *in order* using AppMgr().runSelectedEvents(pfn, evtNumber)
  # on both Serial-Reader and Parallel-Reader processes.
  #
  # The string repr of everything in the TES is placed in a dictionary and
  # sent to the comparison Process, which compares the two dictionaries
  #
  a = AppMgr()
  sel = a.evtsel()
  evt = a.evtsvc()

  header = '/Event/Rec/Header'
  sel.open( filename )
  ct = 0
  order = {}
  fname = filename[4:] # runSelectedEvents doesn't need the "PFN:" prefix

  # determine the ordering
  while True :
    a.run( 1 )
    if evt[header] :
      eNumber = int( evt[header].evtNumber() )
      order[eNumber] = ct
      ct += 1
    else : break

  if readerType == SER :
    # send the ordering details to the parallel-reader
    order = switchDict( order )
    qacross.put( order )
    qacross.put( None )
    # changeName
    serOrder = order
  elif readerType == PAR :
    # receive the serial ordering from queue,  and send ordering to SerialReader
    for serOrder in iter( qacross.get, None ) : pass
    lsks = len( serOrder.keys() )
    lpks = len( order.keys() )
    print 'Events in Files (serial/parallel) : %i / %i' % ( lsks, lpks )

  # now run files in the order specified by the serial ordering
  # and send them one by one to the comparison engine
  for i in iter( serOrder.keys() ) :
    if readerType == PAR  : i = order[serOrder[i]]

    a.runSelectedEvents( fname, i )
    lst = evt.getList()

    lst.sort()
    ascii = dict( [ ( l, ( evt[l].__class__.__name__, evt[l].__repr__() ) ) for l in lst ] )
    qToEngine.put( ascii )
  qToEngine.put( None )
  print '%s Reader Finished' % ( readerType )

def ComparisonEngine( pQueue, sQueue ) :
   # The Comparison Engine runs on a seperate forked process and receives
   # events in pairs, one each from Serial FileReader and Parallel FileReader
   #
   # The events arrive in Dictionary Format, d[path]=(className, string_repr)
   # and are compared using the compareEvents method
   #
   # Results are stored in an array of bools (PerfectMatch=True, Diff=False)
   #
   results = []
   while True :
     pitem = pQueue.get()
     sitem = sQueue.get()
     if pitem == sitem == None : print 'Termination Signals received ok' ; break
     elif pitem == None      : print 'pitem != sitem : ', pitem, sitem ; break
     elif sitem == None      : print 'pitem != sitem : ', pitem, sitem ; break
     results.append( compareEvents( pitem, sitem ) )
   print '=' * 80
   print 'Comparison Engine Finished'
   print '-' * 80
   print 'Total Events Checked : %i' % ( len( results ) )
   print 'Perfect Matches      : %i' % ( sum( results ) )
   print 'Errors               : %i' % ( len( results ) - sum( results ) )
   print '=' * 80

def checkForAddressDifference( a, b ) :
  # the __repr__() method for Event Data Objects will return a generic
  # string "DataObject at 0xADDRESS" for non-Pythonised objects
  # If these objects have the same path, they are equal, but this
  # cannot be tested with "==" in Python, as the memory address will
  # be different for the two different DataObjects, so this method
  # will check if the difference is in the address
  #
  # args : a, b two string representations
  ref = 'DataObject at 0x'
  if a[:16] == b[:16] == ref : return True
  else : return False


def compareEvents( s, p ) :
  # events in form of dictionary, with form
  # d[ path ] = tuple( className, string_repr )

  # check 1 : number of keys (paths)
  sks = s.keys() ; pks = p.keys()
  sks.sort() ; pks.sort()
  if len( sks ) == len( pks ) : pass
  else                    :
    # There may be extra keys in the parallel file
    # example: DstWriter may ask for /Event/Prev/MC/Header#1
    #          but in TESSerializer, *all* DataObjects will be sent
    #          including /Event/Prev and /Event/Prev/MC

    # check for extra keys in the parallel file which are just containing DataObjects
    # if found, remove them

    extras = list( set( pks ) - set( sks ) )
    for e in extras :
      if p[e][0] == 'DataObject' : pks.remove( e )
      else : print 'Extra Other thing found!', e, p[e][0] ; return False

  # check 2 : same paths?
  if sks == pks : pass
  else          : return False

  # check 3 : check the content
  l = len( sks )
  diffs = []
  for i in xrange( l ) :
    key = sks[i]
    # compare class name
    if s[key][0] == p[key][0] : pass
    else : diffs.append( key )
    # compare string representation
    if s[key][1] == p[key][1] : pass
    elif checkForAddressDifference( p[key][1], s[key][1] ) : pass
    else : diffs.append( key )

  # finish
  if diffs : return False
  else     : return True

def CheckFileRecords( par, ser ):

  print "Checking File Records"

  parFSR = GetFSRdicts( par )
  serFSR = GetFSRdicts( ser )



  diff1 = set( parFSR["TimeSpanFSR"].iteritems() ) - set( serFSR["TimeSpanFSR"].iteritems() )
  diff2 = set( parFSR["EventCountFSR"].iteritems() ) - set( serFSR["EventCountFSR"].iteritems() )

  print "\nDifferent entries in TimeSpanFSR:  \t" + str( len( diff1 ) ) + "\nDifferent entries in EventCountFSR:\t" + str( len( diff2 ) )

  for k in ["LumiFSRBeamCrossing", "LumiFSRBeam2", "LumiFSRNoBeam"]:
    diff3 = set( parFSR[k]["key"] ) - set( serFSR[k]["key"] )
    diff4 = set( parFSR[k]["incr"] ) - set( serFSR[k]["incr"] )
    diff5 = set( parFSR[k]["integral"] ) - set( serFSR[k]["integral"] )
    print "Different entries in " + str( k ) + ": \tkey: " + str( len( diff3 ) ) + " increment: " + str( len( diff4 ) ) + " integral: " + str( len( diff5 ) )


def LumiFSR( lumi ):

   runs = []
   files = []
   info = {}
   keys = []

   for r in lumi.runNumbers() :
     runs.append( r )

   for f in lumi.fileIDs() :
     files.append( f )
   s = str( lumi )
   sa = s.split( "info (key/incr/integral) : " )[-1]
   sa = sa.split( '/' )[:-1]

   key = []
   incr = []
   integral = []
   for rec in sa :
     k, i, t = rec.split()
     key.append( int( k ) )
     incr.append( int( i ) )
     integral.append( int( t ) )

   return ( runs, files, key, incr, integral )


def GetFSRdict( filename, queue ):


  FSR = { "TimeSpanFSR" :  {'earliest': 0, 'latest': 0},
          "LumiFSRBeamCrossing" :  {'key': 0, 'incr': 0, 'integral':0},
          "LumiFSRBeam1" :  {'key': 0, 'incr': 0, 'integral':0},
          "LumiFSRBeam2" :  {'key': 0, 'incr': 0, 'integral':0},
          "LumiFSRNoBeam" :  {'key': 0, 'incr': 0, 'integral':0},
           "EventCountFSR" :  {'input': 0, 'output': 0, 'statusFlag': 0}}

  options  = "from LumiAlgs.LumiFsrReaderConf import LumiFsrReaderConf as LumiFsrReader; LumiFsrReader().OutputLevel =  INFO; LumiFsrReader().inputFiles = ['%s'] ;" %filename
  options += "LumiFsrReader().Persistency='ROOT'; LumiFsrReader().EvtMax = 1; from Configurables import LHCbApp; LHCbApp().Persistency='ROOT';  from Configurables import CondDB, DDDBConf;"
  options += " CondDB().UseLatestTags=['%s']; DDDBConf(DataType='%s');"%( 2011, 2011 )
  exec options
  app = AppMgr()
  app.run( 1 )
  fsr = app.filerecordsvc()

  lst = fsr.getHistoNames()

  if lst :
    for l in lst :

      ob = fsr.retrieveObject( l )


      if "LumiFSR" in l:

        assert ob.numberOfObjects() == 1
        k = ob.containedObject( 0 )
        runs, files, keys, increment, integral = LumiFSR( k )

        FSR[l[l.rfind( '/' ) + 1:]]['runs'] = runs
        FSR[l[l.rfind( '/' ) + 1:]]['files'] = files
        FSR[l[l.rfind( '/' ) + 1:]]['key'] = keys
        FSR[l[l.rfind( '/' ) + 1:]]['incr'] = increment
        FSR[l[l.rfind( '/' ) + 1:]]['integral'] = integral



      if "TimeSpanFSR" in l:

        FSR["TimeSpanFSR"]['earliest'] = ob.containedObject( 0 ).earliest()
        FSR["TimeSpanFSR"]['latest'] = ob.containedObject( 0 ).latest()

      if "EventCountFSR" in l:

        FSR["EventCountFSR"]['input'] = ob.input()
        FSR["EventCountFSR"]['output'] = ob.output()
        FSR["EventCountFSR"]['statusFlag'] = ob.statusFlag()

  app.stop()
  app.finalize()

  queue.put( FSR )

def CompareFSR( pout, sout ):

  parFSR = pout.get()
  serFSR = sout.get()

  print "Comparing File Records"

  diff1 = set( parFSR["TimeSpanFSR"].iteritems() ) - set( serFSR["TimeSpanFSR"].iteritems() )
  diff2 = set( parFSR["EventCountFSR"].iteritems() ) - set( serFSR["EventCountFSR"].iteritems() )

  print "\nDifferent entries in TimeSpanFSR:  \t" + str( len( diff1 ) ) + "\nDifferent entries in EventCountFSR:\t" + str( len( diff2 ) )

  for k in ["LumiFSRBeamCrossing", "LumiFSRBeam2", "LumiFSRNoBeam"]:
    diff3 = set( parFSR[k]['key'] ) - set( serFSR[k]['key'] )
    diff4 = set( parFSR[k]['incr'] ) - set( serFSR[k]['incr'] )
    diff5 = set( parFSR[k]['integral'] ) - set( serFSR[k]["integral"] )
    print "Different entries in " + str( k ) + ": \tkey: " + str( len( diff3 ) ) + " increment: " + str( len( diff4 ) ) + " integral: " + str( len( diff5 ) )


  print "\nParallel: \n" + str( parFSR )
  print "\nSerial: \n" + str( serFSR )

if __name__ == '__main__' :

  args = sys.argv
  args.pop( 0 ) # get rid of script name
  if len( args ) != 2 :
    print 'Please supply two arguments : > python loadFile <parallelFile> <serialFile>'
    sys.exit( 0 )
  else :
    par = 'PFN:' + args[0]
    ser = 'PFN:' + args[1]
    print 'Parallel File to be analysed : %s' % ( par )
    print 'Serial   File to be analysed : %s' % ( ser )


  pname = par[4:] # TFile doesn't need the "PFN:" prefix
  sname = ser[4:]

  qacross = Queue()
  pout = Queue()
  sout = Queue()

  par = Process( target = Reader, args = ( PAR, par, qacross, pout ) )
  ser = Process( target = Reader, args = ( SER, ser, qacross, sout ) )
  com = Process( target = ComparisonEngine, args = ( pout, sout ) )

  #com.start() ; par.start() ; ser.start()
  #ser.join()  ; par.join()  ; com.join()

  #CompareTrees( pname, sname )

  print "Check File Records"

  ser = sys.argv[0]
  par = sys.argv[1]

  pout = Queue()
  sout = Queue()

  sp = Process( target = GetFSRdict, args = ( ser, sout ) )
  pp = Process( target = GetFSRdict, args = ( par, pout ) )
  cp = Process( target = CompareFSR, args = ( pout, sout ) )

  sp.start(); pp.start(); cp.start()
  sp.join();pp.join(); cp.join()

