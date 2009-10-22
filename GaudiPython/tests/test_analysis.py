import gaudimodule as gaudi
import math
g = gaudi.AppMgr()
g.JobOptionsType = 'NONE'
g.EvtSel  = "NONE"
g.config()
print '**************GaudiPython Job has been configured*************'

g.DLLs   =   ['GaudiAlg']
g.DLLs   +=  ['GPyTest']
g.loaddict('test_GPyTestDict')

seq1 = g.algorithm('seq1')
seq1.members = ['HelloWorld', 'WriteAlg' ]
g.topAlg +=  ['Sequencer/seq1']

g.initialize()
print '**************GaudiPython Job has been initialized*************'
g.run(2)
print '**************GaudiPython Job has been run ********************'
evt    = g.datasvc('EventDataSvc')

header = evt['/Event']
print '***The event number is: ', header.event()
tracks = evt['/Event/MyTracks']
print '***I got ', tracks.size(), ' tracks'

his    = g.histsvc('HistogramDataSvc')
h1 = his.book('h1', 'histogram title 1D', 10, 0, 10)
print '***Created 1D histogram'
print h1

h2 = his.book('h2', 'histogram title 2D', 10, 0, 10, 20, 0, 10)
print '***Created 2D histogram'
print h2

print '**************Histograms Tested ********************'

#---------------User Algorithm----------------------------------------------
class PhysAnalAlg(gaudi.PyAlgorithm):
  def initialize(self):
    self.evt = g.evtsvc()
    self.his = g.histsvc()
    print 'Initializing User Analysis...'
    self.h1 = self.his.book('myhisto1', 'Histogram 1D for tests', 20, 0., 40.) 
    self.h2 = self.his.book('myhisto2', 'Histogram 2D for tests', 20, 0., 40., 20, 0., 10.) 
    print '....User Analysis Initialized'
    return 1
  def execute(self):
    tks = self.evt['MyTracks']
    print 'MyTracks collection contains '  + `tks.size()` + ' Tracks'
    for t in tks :
      self.h1.fill( math.sqrt(t.px()*t.px() + t.py()*t.py() + t.pz()*t.pz()), 1)
      self.h2.fill( t.px(), t.py() )
    return 1 
  def finalize(self):
    print 'Finalizing User Analysis...'
    print self.h1
    print self.h1.contents()
    print self.h2
    print '....User Analysis Finalized'
    return 1
  def __repr__(self):
    return 'here we are'



from GaudiAlgs import HistoAlgo

class MyHistoAlg(HistoAlgo) :
  def execute( self ) :
    #--waiting to fix a problem in PyROOT
    self.plot( 15 , 'ha-ha-ha' , 0 , 100 ) 
    return 1

  

print '**************Testing Python Algorithms ********************'
phyanalalg = PhysAnalAlg('PythonAlgorithm')
myAlg = MyHistoAlg('MyAlg')
g.setAlgorithms( ['Sequencer/seq1', phyanalalg, myAlg ] )

g.run(10)

hh=myAlg.histoSvc('MyAlg/1')
print hh

#g.exit()
