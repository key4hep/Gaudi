# This is an example of how the configuration could look like without a
# global store of instances

from GaudiConfig2 import Configurables as C, Configurable


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


def merge(*configs):
    '''
    Temporary example of merging step for configuration parts.
    '''
    from itertools import chain
    out = {}
    for c in chain(*configs):
        if c.name in out:
            out[c.name]._properties.update(c._properties)
        else:
            out[c.name] = c
    return out


def main(nevt, outfile, OutputLevel=3):
    msgSvc = C.MessageSvc(OutputLevel=3)
    app = C.ApplicationMgr(MessageSvcType=msgSvc)

    return adjustLogLevels(
        merge([app, msgSvc], setUpAlgorithms(), configureTiming(),
              setFakeEvents(nevt), histogramWriting(outfile)))


# This is a temporary hack to make gaudirun take into account the configration
Configurable.instances = main(50000, 'histo.root')
