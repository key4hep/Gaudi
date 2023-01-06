#####################################################################################
# (c) Copyright 2022-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import os
from threading import Thread

from GaudiTests import run_gaudi


def config():
    from Configurables import ApplicationMgr, PerfProfile

    prof = PerfProfile(
        FIFOPath="control.fifo",
        StartFromEventN=3,
        StopAtEventN=8,
    )
    ApplicationMgr(
        EvtSel="NONE",
        EvtMax=10,
        TopAlg=[prof],
    )


def test(tmp_path):
    """Emulate perf record --control"""
    fifo = tmp_path / "control.fifo"
    fifo_lines = []
    os.mkfifo(fifo)

    def reader():
        # open blocks until FIFO is opened for writing by PerfProfile::initialize(), so run it in a thread
        with open(fifo) as f:
            for line in f:
                fifo_lines.append(line)

    t = Thread(target=reader)
    t.start()

    run_gaudi(f"{__file__}:config", check=True, cwd=tmp_path)

    t.join()

    assert fifo_lines == ["enable\n", "disable\n"]
