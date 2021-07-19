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
    import GaudiConfig2.Configurables.Gaudi.Tests.Histograms.Directories as D

    algs = []
    tools = []
    svcs = []

    Alg = D.HistoGroupsAlg
    algs.append(Alg("Alg"))

    svcs.append(C.Gaudi.Histograming.Sink.Root(FileName=FILENAME))
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
        for name in [
                f"{component}/{histogram}" for component in ["Alg"]
                for histogram in
            ["Top", "Group/First", "Group/Second", "Group/SubGroup/Third"]
        ]:
            h = f.Get(name)
            assert h, f"missing histogram {name}"

    except AssertionError as err:
        causes.append(str(err))
        return False

    except Exception as err:
        causes.append("failure reading histogram file")
        result["python_exception"] = result.Quote(format_exc())
        return False

    return True
