from GaudiConfig2 import Configurables as C, useGlobalInstances
useGlobalInstances(True)

C.AuditorSvc('AuditorSvc', Auditors=['ChronoAuditor'])

# algorithms
algorithms = [
    C.GaudiHistoAlgorithm('SimpleHistos', HistoPrint=True, OutputLevel=3),
    C.Gaudi.Examples.HistoProps('Histos2', Histo2=(-5, 5, 200)),
]

app = C.ApplicationMgr(
    'ApplicationMgr',
    TopAlg=['SimpleHistos', 'Histos2'],
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
    Predefined1DHistos={"/stat/Histos2/2": ("TEST2", -100, 200)},
    OutputLevel=2)
