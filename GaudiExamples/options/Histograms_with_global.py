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
from GaudiConfig2 import Configurables as C, useGlobalInstances
useGlobalInstances(True)

C.AuditorSvc('AuditorSvc', Auditors=['ChronoAuditor'])

# algorithms
algorithms = [
    C.GaudiHistoAlgorithm('SimpleHistos', HistoPrint=True, OutputLevel=3),
    C.Gaudi.Examples.R7.GaudiHistoAlgorithm('SimpleR7Histos', OutputLevel=2),
    C.Gaudi.Examples.Boost.GaudiHistoAlgorithm(
        'SimpleBoostHistos', OutputLevel=2),
]

app = C.ApplicationMgr(
    'ApplicationMgr',
    TopAlg=['SimpleHistos', 'SimpleR7Histos', 'SimpleBoostHistos'],
    EvtMax=50000,
    EvtSel='NONE',
)
try:
    app.TopAlg.append('MessageSvc')
    assert False, "this is not expected"
except TypeError:
    pass  # this is expected

app.TopAlg[0].OutputLevel = 2

app.MessageSvcType = 'MessageSvc'
app.MessageSvcType.OutputLevel = 3

try:
    app.JobOptionsSvcType = 'MessageSvc'
    assert False, "this is not expected"
except TypeError:
    pass  # this is expected

app.HistogramPersistency = 'ROOT'
C.RootHistCnv.PersSvc('RootHistSvc', OutputFile='histo-c2g.root')

C.HistogramSvc(
    'HistogramDataSvc',
    OutputLevel=2,
    Input=["InFile DATAFILE='../data/input.hbook' TYP='HBOOK'"])
