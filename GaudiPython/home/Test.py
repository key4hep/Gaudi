import math
import sys
#----------Patch to force loading .so with RTDL_GLOBAL
if sys.platform == 'linux2' : sys.setdlopenflags(0x100|0x2)

import gaudimodule as gaudi
g = gaudi.AppMgr()
g.JobOptionsType = 'NONE'
g.EvtSel  = "NONE"

g.config()
print '**************GaudiPython Job has been configured*************'

g.DLLs   =   ['GaudiAlg', 'GaudiIntrospection']
g.ExtSvc =   ['IntrospectionSvc']

g.service('IntrospectionSvc').Dictionaries = ['TestDict']

g.DLLs   +=  ['Test']

seq1 = g.algorithm('seq1')
seq1.members = ['HelloWorld', 'WriteAlg' ]

g.topAlg +=  ['Sequencer/seq1']

g.initialize()
print '**************GaudiPython Job has been initialized*************'
g.run(2)
print '**************GaudiPython Job has been run ********************'


evt    = gaudi.DataSvc(g.service('EventDataSvc'))
header = evt['/Event']
print '***The event header is: ', header.values()
tracks = evt['/Event/MyTracks']
print '***I got ', len(tracks), ' tracks'

his    = gaudi.HistoSvc(g.service('HistogramDataSvc'))
h1 = his.book('h1', 'histogram title 1D', 10, 0, 10)
print '***Created 1D histogram'
print h1
h2 = his.book('h2', 'histogram title 1D', 10, 0, 10, 20, 0, 10)
print '***Created 2D histogram'
print h2

print '**************Histograms Tested ********************'


#---------------User Algorithm----------------------------------------------
class PhysAnalAlg(gaudi.PyAlgorithm):
  def initialize(self):
    self.evt = gaudi.DataSvc(g.service('EventDataSvc'))
    self.his = gaudi.HistoSvc(g.service('HistogramDataSvc'))
    print 'Initializing User Analysis...'
    self.h1 = self.his.book('myhisto1', 'Histogram 1D for tests', 20, 0., 40.) 
    self.h2 = self.his.book('myhisto2', 'Histogram 2D for test2', 20, 0., 40., 20, 0., 10.) 
    print '....User Analysis Initialized'
    return 1
  def execute(self):
    tks = self.evt.object('MyTracks')
    print 'MyTracks collection contains '  + `len(tks)` + ' Tracks'
    for t in tks :
      self.h1.fill( math.sqrt(t.px*t.px + t.py*t.py + t.pz*t.pz), 1)
      self.h2.fill( t.px, t.py )
    return 1 
  def finalize(self):
    print 'Finalizing User Analysis...'
    print self.h1
    print self.h1.entries()
    print self.h2
    print '....User Analysis Finalized'
    return 1 

print '**************Testing Python Algorithms ********************'
phyanalalg = PhysAnalAlg('PythonAlgorithm')
g.topAlg = g.topAlg + [ 'PythonAlgorithm' ]
g.run(100)
g.exit()









