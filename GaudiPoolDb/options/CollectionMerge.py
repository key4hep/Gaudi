import os, sys, gaudimodule

class CollectionMerge:
  def __init__(self):
    self.merger_input  = []
    self.merger_output = []
    self.input         = []
    self.output        = []
    self.appMgr = gaudimodule.AppMgr()
    self.appMgr.EvtMax = 1
    self.appMgr.EvtSel = 'NONE'
    self.appMgr.Dlls   += ['GaudiPoolDb']
    self.appMgr.ExtSvc += ['PoolDbCacheSvc','TagCollectionSvc']
    self.appMgr.service('PoolRootEvtCnvSvc').DbType     = 'POOL_ROOT'
    self.appMgr.service('PoolRootKeyEvtCnvSvc').DbType  = 'POOL_ROOTKEY'
    self.appMgr.service('PoolRootTreeEvtCnvSvc').DbType = 'POOL_ROOTTREE'
    self.appMgr.service('PoolDbCacheSvc').Dlls          = ['lcg_RootStorageSvc','lcg_XMLCatalog','SealSTLDict']
    self.appMgr.service('PoolDbCacheSvc').OutputLevel   = 4
    self.appMgr.topAlg += ['CollectionCloneAlg']
    self.collSvc = self.appMgr.service('TagCollectionSvc')
    self.merger=self.appMgr.algorithm('CollectionCloneAlg')
    self.merger.EvtTupleSvc = 'TagCollectionSvc'

  def finalize(self):
    self.appMgr.finalize()
    self.appMgr.exit()

  def defineInput(self,tuple,file,typ='POOL_ROOTTREE'):
    num_input = len(self.input)
    log_name = 'INPUT_'+str(num_input)
    tup_name = log_name+'/'+tuple
    self.merger_input.append(tup_name)
    self.input.append(log_name + " DATAFILE='"+file+"' OPT='READ' TYP='"+typ+"'")

  def defineOutput(self,tuple,file,selector=None,criteria=None,opt='RECREATE',typ='POOL_ROOTTREE'):
    self.output = ['OUTPUT' + " DATAFILE='"+file+"' OPT='"+opt+"' TYP='"+typ+"'"]
    self.merger_output = "DATA='OUTPUT/"+tuple+"'"
    if ( criteria is not None ):
      self.merger_output  = self.merger_output  + " SEL='"+criteria+"'"
    if ( selector is not None ):
      self.merger_output  = self.merger_output  + " FUN='"+selector+"'"
    print self.merger_output

  def execute(self, do_finalize=1):
    self.collSvc.Input = self.input
    self.collSvc.Output = self.output
    self.merger.Input = self.merger_input
    self.merger.Output = self.merger_output
    self.appMgr.initialize()
    self.appMgr.algorithm('CollectionCloneAlg').execute()
    if ( do_finalize ):
      self.finalize()
    
if __name__ == '__main__':
  o = CollectionMerge()
  o.defineInput('Dir1/Dir2/Dir3/Collection','PFN:MyEvtCollection0.root')
  o.defineInput('Dir1/Dir2/Dir3/Collection','PFN:MyEvtCollection1.root')
  o.defineInput('Dir1/Dir2/Dir3/Collection','PFN:MyEvtCollection2.root')
  o.defineInput('Dir1/Dir2/Dir3/Collection','PFN:MyEvtCollection3.root')
  o.defineOutput(tuple='Merged',criteria='Ntrack>80',file='PFN:NewCollection.root',opt='RECREATE')
  o.execute()
  sys.exit()
