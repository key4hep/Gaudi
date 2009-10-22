#!/usr/bin/env gaudirun.py 
###############################################################
# Job options file
#==============================================================
from Gaudi.Configuration import *
from Configurables import MyDataAlgorithm, DataCreator

importOptions('Common.opts')
MessageSvc().OutputLevel = INFO

mdigi  = DataCreator ( 'MuonDigits', Data = 'Rec/Muon/Digits')
mfoo   = DataCreator ( 'MuonFoos'  , Data = 'Rec/Muon/Foos'  )
mold   = DataCreator ( 'MuonOld'   , Data = 'Rec/Muon/Old'   )
dondem = DataOnDemandSvc(
    UsePreceedingPath = True,
    NodeMap = { 'Rec': 'DataObject', 
                'Rec/Muon'  : 'DataObject',
                'Rec/Foo'   : 'DataObject',
                },
    AlgMap  = { mdigi.Data  : mdigi,
                mfoo.Data   : mfoo  
                },
    # obsolete property:
    Algorithms = [ "DATA='%s' TYPE='%s'"%
                   (mold.Data, mold.getFullName())
                   ],
    # obsolete property:
    Nodes = [ "DATA='Rec/Obsolete' TYPE='DataObject'"],
    Dump = False    
    )
pcache = PoolDbCacheSvc( Dlls = ['GaudiKernelDict'] )

ApplicationMgr( TopAlg = [ MyDataAlgorithm() ],
                ExtSvc = [ pcache, dondem ],
                EvtMax = 10,
                EvtSel = 'NONE' )
                
PoolDbCacheSvc().Dlls = ['GaudiKernelDict']
EventDataSvc().EnableFaultHandler = True

