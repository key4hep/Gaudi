#!/usr/bin/env gaudirun.py
###############################################################
# Job options file
#==============================================================
from Gaudi.Configuration import *
from Configurables import MyDataAlgorithm, DataCreator

from Configurables import GaudiExamplesCommonConf
GaudiExamplesCommonConf()

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

ApplicationMgr( TopAlg = [ MyDataAlgorithm() ],
                ExtSvc = [ dondem ],
                EvtMax = 10,
                EvtSel = 'NONE' )

EventDataSvc().EnableFaultHandler = True

