from Gaudi.Configuration import *

importOptions("test_purge2.opts")

AuditorSvc().Auditors += [ "TimingAuditor/TIMER" ] 
