from ROOT import TFile
import sets
import sys

histos = ['TH1D', 'TH2D', 'TProfile']
ser = 'SERIAL'
par = 'PARALL'

# =================================================================================================
# Method   : rec( o, path=None, lst=None )
# 
# @param o    : a ROOT object
# @param path : a string like a transient store path; ie '/stat/CaloPIDs/ECALPIDE'
# @param lst  : a list to hold (path, object) tuples
# 
# function    : recursively pull apart a ROOT file, making a list of (path, TObject) tuples
#               This is done by GetListOfKeys method, which lets one work down through directories
#               until you hit the Histo at the end of the path.  The list of tuples is returned
# 
def rec( o, path=None, lst=None ) :
  if not path : path = '/stat' ; lst = []
  else        : path = path + '/' + o.GetName()
  lst.append( (path,o) )
  if 'GetListOfKeys' in dir(o) : 
    keys = o.GetListOfKeys()
    for k in keys :
      name =  k.GetName()
      rec( o.Get(name), path, lst ) 
  else :
    pass
  return lst
# =================================================================================================

# =================================================================================================
# Method   : composition( t )
# 
# @param t : a tuple of ( type, d ) where type is either 'SERIAL' or 'PARALL'
#            and d is a dictionary of ROOT objects, with each key = ROOT path 
# 
# function : deduce the composition, (objects/histos) counts
#
def composition( t ) :
  typ, d = t 
  hists = 0 ; objs = 0
  for k in d.keys() : 
    if d[k].__class__.__name__ in histos : hists += 1
    else                                 : objs  += 1
  return objs, hists
# =================================================================================================
  
# =================================================================================================
# Method        : comparePaths( t1, t2 )
# 
# @param t1, t2 : a tuple of ( type, d ) where type is either 'SERIAL' or 'PARALL'
#                 and d is a dictionary of ROOT objects, with each key = ROOT path 
# 
# function      : compare the paths between the two histo files.  If the files are identical, they
#                 should have the same set of paths.  The Parallel file should definitely have the
#                 same paths as the Serial.  Perhaps the Serial file will have some more paths due
#                 to extra histos added as part of Application Sequencer finalisation
#                 Arguments t1 and t2 are checked and the parallel/serial auto-detected
#                 Uses sets module for intersections/unions, etc.
# 
def comparePaths( t1, t2 ) : 
  if   t1[0]  == ser : ds = t1[1] ; dp = t2[1]
  elif t2[0]  == ser : ds = t2[1] ; dp = t1[1]
  else : print 'Neither tuple is Serial Root file reference?' ; return
  
  dsks = ds.keys() ; dpks = dp.keys()
  dsks.sort()      ; dpks.sort()
  
  sset = sets.Set( dsks )
  pset = sets.Set( dpks )
  os, hs = composition( (ser, ds) )
  op, hp = composition( (par, dp) )
  print '\n' + '='*80
  print 'Comparison of Paths : Serial vs Parallel ROOT files'
  print '-'*80
  print 'Number of paths in Serial file : %i (objects, histos) = ( %i, %i )'%( len(dsks), os, hs )
  print 'Number of paths in Parall file : %i (objects, histos) = ( %i, %i )'%( len(dpks), op, hp )
  matching = sset.intersection(pset)
  matchingHistos = 0
  for n in matching :
    if ds[n].__class__.__name__ in histos : matchingHistos += 1    
  print '\nMatching paths                 : %i'%( len(matching) )
  uSer = sset - pset
  # work out histos unique to parallel file
  uniqueSerialHistos = 0
  for n in uSer :
    if ds[n].__class__.__name__ in histos : uniqueSerialHistos += 1
  print 'Paths unique to Serial file : %i ( %i Histos )'%( len(uSer), uniqueSerialHistos )
  if uSer : 
    for n in uSer : print '\t%s : \t%s'%( ds[n], n )
  uPar = pset - sset
  uniqueParallHistos = 0  
  for n in uPar :
    if dp[n].__class__.__name__ in histos : uniqueParallHistos += 1  
  print 'Paths unique to Parall file : %i ( %i Histos )'%( len(uPar), uniqueParallHistos )
  if uPar : 
    for n in uPar : print '\t%s : \t%s'%( dp[n], n )
  print 'Matching Histos to test : %i'%( matchingHistos )
  print '='*80 + '\n'
  return ( ((os,hs),(op,hp)), (uSer, uniqueSerialHistos), (uPar, uniqueParallHistos), matchingHistos )
# =================================================================================================  

# =================================================================================================
# Method        : compareHistos( t1, t2 )
# 
# @param t1, t2 : a tuple of ( type, d ) where type is either 'SERIAL' or 'PARALL'
#                 and d is a dictionary of ROOT objects, with each key = ROOT path 
# 
# function      : compare the histograms in Serial/Parallel ROOT files.  First, go through each 
#                 dict to collect the histos (ignore TDirectory objects, etc).  Then the histos
#                 in the parallel file (experimental) are compared to their equivalents in the 
#                 serial file (definitely correct) using 3 methods.
#                 1) The entries are checked, they should be equal
#                 2) If entries are equal, check the Integral(); should be equal
#                 3) If integrals are equal, check the KolmogorovTest() ; should be 1
#                 Arguments t1 and t2 are checked and the parallel/serial auto-detected
# 
def compareHistos(t1, t2, state) :
  
  ( ((serialObjects,serialHistos),(parallObjects, parallHistos)), (uniqueSerPaths,uniqueSerHistos), (uniqueParPaths,uniqueParHistos), mh ) = state
 
  # deduce which one is parallel, which serial
  if   t1[0]  == ser : ds = t1[1] ; dp = t2[1]
  elif t2[0]  == ser : ds = t2[1] ; dp = t1[1]
  else : print 'Neither tuple is Serial Root file reference?' ; return

  # histocount, objectcount for parallel/serial
  hcp = 0 ; pHistos = []
  hcs = 0 ; sHistos = []
  
  omit = ['/stat/Brunel/MemoryTool/Virtual mem, all entries',
          '/stat/Brunel/MemoryTool/Virtual mem, downscaled']
  omit = []
  
  # find the histos in the serial file
  for k in ds.keys() : 
    if k not in omit : 
      if ds[k].__class__.__name__ in histos : hcs += 1 ; sHistos.append( k )
  # same for parallel
  for k in dp.keys() : 
    if k not in omit : 
      if dp[k].__class__.__name__ in histos : hcp += 1 ; pHistos.append( k )
      

  cEntries   = 0 ; xEntries   = 0 ; diffEntries   = []
  cIntegrals = 0 ; xIntegrals = 0 ; diffIntegrals = []
  passedKol  = 0 ; failedKol  = 0 ; diffKols      = [] ; zeroIntegrals = 0
  kTested    = 0
  notfound   = 0 ; integralMatch = 0 ; otherTest = 0 ; zeroIntegralMatch = 0
  for h in sHistos :
    if h in pHistos : 
      # matching histos to check
      cEntries += 1
      sh = ds[h] ; ph = dp[h]
      # first check entries
      if sh.GetEntries() != ph.GetEntries() : diffEntries.append(h) ; xEntries += 1 ; continue
      # check for (non-zero sum of bin error) && (non-zero integrals) for K-Test
      sBinError = 0.0 ; pBinError = 0.0
      for i in xrange(sh.GetNbinsX()) : sBinError += sh.GetBinError(i)
      for i in xrange(ph.GetNbinsX()) : pBinError += ph.GetBinError(i)
      sint = sh.Integral() ; pint = ph.Integral()
      if (bool(sint) and bool(pint)) and ( sBinError>0 and pBinError>0 ) : 
        kTested += 1
        kTest = sh.KolmogorovTest(ph)
        if int(kTest) : passedKol += 1 
        else          : failedKol += 1 ; diffKols.append(h) # ; print 'KTest result : ', kTest        
      else : 
        # try the integral test?
        otherTest += 1
        if all((sint, pint)) and (sint==pint) : 
          integralMatch += 1
        elif (sint==pint) :
          zeroIntegralMatch += 1
        else : 
            diffIntegrals.append( h )
            xIntegrals += 1
    else :
      notfound += 1 ; print 'not found? ', h

  # report on Failed Entry-Checks      
  print '\n\n'+'-'*80
  print 'Summary of histos with different Entries'
  print '-'*80
  if diffEntries :
    diffEntries.sort()  
    for e in diffEntries : print '\t\t\t%s:\t%i != %i'%( e, int(ds[e].GetEntries()), int(dp[e].GetEntries()) )  
  print '-'*80
  
  # report on Failed Kolmogorov Tests    
  print '\n\n'+'-'*60
  print 'Summary of histos which failed Kolmogorov Test'
  print '-'*60
  if diffKols :
    diffKols.sort()  
    for e in diffKols : 
      result = ds[e].KolmogorovTest(dp[e])
      print '%s\t\t%s :\tK-Test Result :\t %5.16f'%( type(ds[e]), e, result )  
  print '-'*60
  
  # report on Failed Integral Checks
  print '\n\n'+'-'*60
  print 'Summary of histos which failed Integral Check'
  print '-'*60
  if diffIntegrals :
    diffIntegrals.sort()
    for e in diffIntegrals : 
      diff = dp[e].Integral()-ds[e].Integral()
      pc   = (diff*100)/ds[e].Integral()
      print '%s\t\t%s:\t Diff = %5.6f\tPercent Diff to Serial : %5.6f '%( type(ds[e]), e, diff, pc  )
  print '-'*60 + '\n' 
  print '='*80 + '\n'
  
  print '\n' + '='*80
  print 'Comparison : Serial/Parallel ROOT Histo files'
  print '\n\t\tSerial\tParall'
  print '\tObjects : %i\t%i\t\t( p-s = %i )'%( serialObjects, parallObjects, parallObjects-serialObjects )  
  print '\tHistos  : %i\t%i\t\t( p-s = %i )'%( serialHistos,  parallHistos,  parallHistos-serialHistos  )
  print '\t          __________'
  print '\tTotal   : %i\t%i\n'%( serialHistos+serialObjects,  parallHistos+parallObjects  )  
  print 'Objects/Histos unique to Serial File : %i / %i'%( len(uniqueSerPaths)-uniqueSerHistos, uniqueSerHistos )
  print 'Objects/Histos unique to Parall File : %i / %i'%( len(uniqueParPaths)-uniqueParHistos, uniqueParHistos )
  print '\nMatching Histograms valid for Comparison : %i'%( mh )
  print '\nOmissions : '
  for entry in omit : print '\t%s'%( entry )   
  print '\nHistograms for Comparison (after Omissions) : %i'%( mh-len(omit) )  
  print '\n\tHISTOGRAM TESTS : '
  print '\t\tKOLMOGOROV TEST      : %i'%( kTested )
  print '\t\tINTEGRAL TEST        : %i'%( otherTest )
  print '\t\tENTRIES TEST         : %i'%( xEntries )
  print '\t\t                       ____'
  print '\t\tTested               : %i'%( cEntries )  

  print '\n\tDISCREPANCIES : '
  print '\t\tK-Test      : %i'%( failedKol  )
  print '\t\tIntegrals   : %i'%( xIntegrals )    
  print '\t\tEntries     : %i'%( xEntries   )  
  print '\n'+'='*80  

# =================================================================================================

if __name__ == '__main__' : 
  sys.argv.pop(0)   # get rid of script name
  if len(sys.argv) == 2 :
    pFile = sys.argv[0]
    sFile = sys.argv[1]
  else :
    print '*'*80
    print 'Wrong count of arguments? > python compareRootHistos.py someParallelFile.root someSerialFile.root'
    print '*'*80
    sys.exit(0)
  tfs = TFile( sFile, 'REC' ) ; print 'opening Serial File : %s'%( sFile )
  tfp = TFile( pFile, 'REC' ) ; print 'opening Parall File : %s'%( pFile )
  
  # get structure of TFiles in a list of (path, object) tuples
  lser = rec(tfs) ; lpar = rec(tfp)
  # make a dictionary of lser and lpar.  keys=paths
  dserial = dict( [(n, o) for n, o in lser] )
  dparall = dict( [(n, o) for n, o in lpar] )
  # make a tuple of (type, dict) where type is either 'serial' or 'parallel'
  ts = ( ser, dserial )  ; tp = ( par, dparall )
  
  # check objs/histos in each file
#   composition( ts ) ; composition( tp )
  
  # compare paths from each file
  state = comparePaths( ts, tp )
  
  # compare histos from each file
  compareHistos( ts, tp, state )
  
#   # finished with TFiles
#   tfs.Close()     ; tfp.Close()  
# 

