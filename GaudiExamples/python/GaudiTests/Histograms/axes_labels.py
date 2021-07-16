#####################################################################################
# (c) Copyright 2021 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import os
from traceback import format_exc

FILENAME = f"{__name__}.root"


def config():
    import GaudiConfig2.Configurables as C
    import GaudiConfig2.Configurables.Gaudi.Tests.Histograms.AxesLabels as T

    algs = []
    svcs = []

    HistoAlgo = T.HistWithLabelsAlg
    algs.append(HistoAlgo("HistoAlgo"))

    svcs.append(C.Gaudi.Histograming.Sink.Root(FileName=FILENAME))
    svcs.append(C.Gaudi.Monitoring.MessageSvcSink())

    yield from algs
    yield from svcs

    yield C.ApplicationMgr(
        EvtMax=5,
        EvtSel="NONE",
        TopAlg=algs,
        ExtSvc=svcs,
    )

    # make sure the histogram file is not already there
    if os.path.exists(FILENAME):
        os.remove(FILENAME)


def check(causes, result):
    result["root_output_file"] = FILENAME

    if not os.path.exists(FILENAME):
        causes.append("missing histogram file")
        return False

    try:
        import ROOT
        f = ROOT.TFile.Open(FILENAME)
        h = f.Get("HistoAlgo/hist")
        axis = h.GetXaxis()
        labels = list(axis.GetLabels())
        expected = ["a", "b", "c", "d", "e"]
        if labels != expected:
            causes.append("wrong axis labels")
            result["expected_labels"] = ", ".join(repr(l) for l in expected)
            result["found_labels"] = ", ".join(repr(l) for l in labels)
            return False

    except Exception as err:
        causes.append("failure reading histogram file")
        result["python_exception"] = result.Quote(format_exc())
        return False

    return True
