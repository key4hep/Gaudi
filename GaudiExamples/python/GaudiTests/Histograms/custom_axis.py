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
FILENAMEJSON = f"{__name__}.json"


def config():
    import GaudiConfig2.Configurables as C
    import GaudiConfig2.Configurables.Gaudi.Tests.Histograms.CustomAxis as T

    algs = []
    tools = []
    svcs = []

    Alg = T.EnumAxisAlg
    algs.append(Alg("Alg"))

    svcs.append(C.Gaudi.Histograming.Sink.Root(FileName=FILENAME))
    svcs.append(C.Gaudi.Monitoring.JSONSink(FileName=FILENAMEJSON))
    svcs.append(C.Gaudi.Monitoring.MessageSvcSink())

    yield from algs
    yield from tools
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

        h = f.Get("Alg/Categories")
        if not h:
            k = f.GetKey("Alg/Categories")
            if k:
                h = k.ReadObj()

        assert h, "missing histogram"

        if h.GetNbinsX() != 4:
            causes.append("number of bins")
            result["error"] = f"expected 4 bins, got {h.GetNbinsX()}"
            return False

        axis = h.GetXaxis()
        labels = list(axis.GetLabels())
        expected = ["Simple", "Complex", "Bad", "Wrong"]
        if labels != expected:
            causes.append("wrong axis labels")
            result["expected_labels"] = ", ".join(repr(l) for l in expected)
            result["found_labels"] = ", ".join(repr(l) for l in labels)
            return False

    except AssertionError as err:
        causes.append(str(err))
        return False

    except Exception as err:
        causes.append("failure reading histogram file")
        result["python_exception"] = result.Quote(format_exc())
        return False

    return True
