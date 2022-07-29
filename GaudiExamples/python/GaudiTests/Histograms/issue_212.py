#####################################################################################
# (c) Copyright 2022 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# See https://gitlab.cern.ch/gaudi/Gaudi/-/issues/212
import os
from collections import defaultdict
from traceback import format_exc
from unittest import TestCase

FILENAME = f"{__name__}.root"
FILENAMEJSON = f"{__name__}.json"


def config():
    import GaudiConfig2.Configurables as C
    import GaudiConfig2.Configurables.Gaudi.Tests.Histograms.MultiDimLayout as T

    algs = []
    tools = []
    svcs = []

    Alg = T.TestAlg
    algs.append(Alg("Alg"))

    svcs.append(C.Gaudi.Histograming.Sink.Root(FileName=FILENAME))
    svcs.append(C.Gaudi.Monitoring.JSONSink(FileName=FILENAMEJSON))
    svcs.append(C.Gaudi.Monitoring.MessageSvcSink())

    yield from algs
    yield from tools
    yield from svcs

    yield C.ApplicationMgr(
        EvtMax=1,
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

        # get the 3 expected histograms
        histos = []
        for i in range(1, 4):
            h = f.Get(f"Alg/h{i}")
            if not h:
                k = f.GetKey(f"Alg/h{i}")
                if k:
                    h = k.ReadObj()

            assert h, f"missing histogram Alg/h{i}"
            histos.append(h)

        h1, h2, h3 = histos
        # mimic the C++ filling loop to validate the content against expectation
        value = 0.0
        expected = defaultdict(dict)
        found = defaultdict(dict)
        for x in [i - 0.5 for i in range(12)]:
            value += 1
            expected["h1"][x] = value
            found["h1"][x] = h1.GetBinContent(h1.FindBin(x))
            for y in [i - 0.5 for i in range(12)]:
                value += 1
                expected["h2"][(x, y)] = value
                found["h2"][(x, y)] = h2.GetBinContent(h2.FindBin(x, y))
                for z in [i - 0.5 for i in range(12)]:
                    value += 1
                    expected["h3"][(x, y, z)] = value
                    found["h3"][(x, y, z)] = h3.GetBinContent(h3.FindBin(x, y, z))

        # this is a trick to piggyback on TestCase dict diff report
        t = TestCase()
        diffs = {}
        for name in expected:
            try:
                t.assertEqual(expected[name], found[name])
            except AssertionError as err:
                diffs[name] = str(err).splitlines()[0]

        if diffs:
            causes.append("histograms content")
            for name in diffs:
                result[f"{name}_diff"] = diffs[name]

    except AssertionError as err:
        causes.append(str(err))
        return False

    except Exception as err:
        causes.append("failure reading histogram file")
        result["python_exception"] = result.Quote(format_exc())
        return False

    return True
