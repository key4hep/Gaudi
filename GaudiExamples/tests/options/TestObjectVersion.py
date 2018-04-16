from Gaudi.Configuration import ApplicationMgr
from Configurables import Gaudi__Examples__TestObjectVersion__CreateObject as CO
from Configurables import Gaudi__Examples__TestObjectVersion__UseObject as UO
ApplicationMgr(TopAlg=[CO(), UO()], EvtSel="NONE", EvtMax=1)
