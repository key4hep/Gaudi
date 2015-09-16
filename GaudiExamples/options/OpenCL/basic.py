from Gaudi.Configuration import DEBUG
from Configurables import ApplicationMgr
from Configurables import Gaudi__Examples__OpenCLAlg as OpenCLAlg

alg = OpenCLAlg('OpenCLAlg', OutputLevel=DEBUG)

ApplicationMgr(TopAlg=[alg], EvtSel='NONE', EvtMax=5)
