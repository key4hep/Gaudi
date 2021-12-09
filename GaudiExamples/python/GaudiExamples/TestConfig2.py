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
from GaudiConfig2 import Configurable
from GaudiConfig2 import Configurables as C
from GaudiConfig2 import mergeConfigs


def setUpAlgorithms():
    algorithms = [
        C.GaudiHistoAlgorithm("SimpleHistos", HistoPrint=True, OutputLevel=3),
    ]
    app = C.ApplicationMgr(TopAlg=algorithms)
    return algorithms + [app]


def configureTiming():
    return [C.AuditorSvc("AuditorSvc", Auditors=["ChronoAuditor"])]


def setFakeEvents(nevt):
    return [C.ApplicationMgr(EvtMax=nevt, EvtSel="NONE")]


def histogramWriting(filename):
    return [
        C.ApplicationMgr(HistogramPersistency="ROOT"),
        C.RootHistCnv.PersSvc("RootHistSvc", OutputFile=filename),
        C.HistogramSvc(
            "HistogramDataSvc",
            OutputLevel=2,
            Input=["InFile DATAFILE='../data/input.hbook' TYP='HBOOK'"],
        ),
    ]


def adjustLogLevels(config):
    config["ApplicationMgr"].TopAlg[0].OutputLevel = 2
    return config


def main(nevt=50000, outfile="histo-c2.root", OutputLevel=3):
    msgSvc = C.MessageSvc(OutputLevel=3)
    app = C.ApplicationMgr(MessageSvcType=msgSvc)
    app.ExtSvc = ["Gaudi::Monitoring::MessageSvcSink"]
    return adjustLogLevels(
        mergeConfigs(
            [app, msgSvc],
            setUpAlgorithms(),
            configureTiming(),
            setFakeEvents(nevt),
            histogramWriting(outfile),
        )
    )
