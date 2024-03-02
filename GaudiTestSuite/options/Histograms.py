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
from Configurables import Gaudi__Monitoring__MessageSvcSink as MessageSvcSink
from Gaudi.Configuration import *

AuditorSvc().Auditors = ["ChronoAuditor"]
MessageSvc().OutputLevel = INFO
RootHistSvc("RootHistSvc").OutputFile = "histo.root"

from Configurables import Gaudi__Histograming__Sink__Root as RootHistoSink
from Configurables import (
    Gaudi__TestSuite__Counter__GaudiHistoAlgorithm as CounterHistoAlg,
)
from Configurables import (
    Gaudi__TestSuite__Counter__GaudiRootHistoAlgorithm as RootCounterHistoAlg,
)

algs = [
    CounterHistoAlg("SimpleCounterHistos", OutputLevel=DEBUG),
    RootCounterHistoAlg("SimpleRootCounterHistos", OutputLevel=DEBUG),
]

app = ApplicationMgr(
    EvtMax=50000,
    EvtSel="NONE",
    HistogramPersistency="ROOT",
    TopAlg=algs,
    ExtSvc=[MessageSvcSink(), RootHistoSink()],
)
