#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiConfig2 import Configurables as C
from GaudiConfig2 import useGlobalInstances

useGlobalInstances(True)

C.AuditorSvc("AuditorSvc", Auditors=["ChronoAuditor"])

# algorithms
algorithms = [
    C.Gaudi.TestSuite.Counter.GaudiHistoAlgorithm("SimpleCounterHistos", OutputLevel=2),
]

app = C.ApplicationMgr(
    "ApplicationMgr",
    TopAlg=["SimpleCounterHistos"],
    EvtMax=50000,
    EvtSel="NONE",
)
try:
    app.TopAlg.append("MessageSvc")
    assert False, "this is not expected"
except TypeError:
    pass  # this is expected

app.TopAlg[0].OutputLevel = 2

app.MessageSvcType = "MessageSvc"
app.MessageSvcType.OutputLevel = 3
app.ExtSvc = ["Gaudi::Monitoring::MessageSvcSink"]

try:
    app.JobOptionsSvcType = "MessageSvc"
    assert False, "this is not expected"
except TypeError:
    pass  # this is expected

app.HistogramPersistency = "ROOT"
C.RootHistCnv.PersSvc("RootHistSvc", OutputFile="histo-c2g.root")

C.HistogramSvc(
    "HistogramDataSvc",
    OutputLevel=2,
)
