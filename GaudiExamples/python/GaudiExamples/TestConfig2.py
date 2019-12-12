from GaudiConfig2 import Configurables as C, Configurable, mergeConfigs


def setUpAlgorithms():
    algorithms = [
        C.GaudiHistoAlgorithm('SimpleHistos', HistoPrint=True, OutputLevel=3),
        C.Gaudi.Examples.HistoProps('Histos2', Histo2=(-5, 5, 200)),
    ]
    app = C.ApplicationMgr(TopAlg=algorithms)
    return algorithms + [app]


def configureTiming():
    return [C.AuditorSvc('AuditorSvc', Auditors=['ChronoAuditor'])]


def setFakeEvents(nevt):
    return [C.ApplicationMgr(EvtMax=nevt, EvtSel='NONE')]


def histogramWriting(filename):
    return [
        C.ApplicationMgr(HistogramPersistency='ROOT'),
        C.RootHistCnv.PersSvc('RootHistSvc', OutputFile=filename),
        C.HistogramSvc(
            'HistogramDataSvc',
            Predefined1DHistos={"/stat/Histos2/2": ("TEST2", -100, 200)},
            OutputLevel=2)
    ]


def adjustLogLevels(config):
    config['ApplicationMgr'].TopAlg[0].OutputLevel = 2
    return config


def main(nevt=50000, outfile='histo.root', OutputLevel=3):
    msgSvc = C.MessageSvc(OutputLevel=3)
    app = C.ApplicationMgr(MessageSvcType=msgSvc)
    return adjustLogLevels(
        mergeConfigs([app, msgSvc], setUpAlgorithms(), configureTiming(),
                     setFakeEvents(nevt), histogramWriting(outfile)))
