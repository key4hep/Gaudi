#--------------------------------------------------
#Note that it is assumed the LHCb environment setup
#--------------------------------------------------
import gaudimodule,os
g = gaudimodule.AppMgr()

g.loaddict('EventDict')
#g.readOptions(os.environ['EVENTSYSROOT']+'/options/PoolDicts.opts')

sel = g.evtsel()
sel.open('rfio:/castor/cern.ch/lhcb/prod/00000080_00000001_5.oodst')
#sel.open('rfio:/castor/cern.ch/user/c/cattanem/Brunel/v23r0/Pool412400.dst',typ='POOL_ROOT')

evt = g.evtsvc()
his = g.histsvc()

h1 = his.book('h1','# of MCParticles', 40, 0, 5000)

class DumpAlg(gaudimodule.PyAlgorithm):
  def execute(self):
    evh  = evt['Header']
    mcps = evt['MC/Particles']
    print 'event # = ',evh.evtNum()
    h1.fill(mcps.size())

g.addAlgorithm(DumpAlg())
g.run(5)
print h1.contents()

