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
C.RootHistCnv.PersSvc('RootHistSvc', OutputFile='histo.root')

C.HistogramSvc(
    'HistogramDataSvc',
    OutputLevel=2,
    Input=["InFile DATAFILE='../data/input.hbook' TYP='HBOOK'"])
