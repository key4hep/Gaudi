#####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import re
import textwrap

from GaudiTesting import GaudiExeTest


class Test(GaudiExeTest):
    command = ["gaudirun.py"]

    def options(self):
        from Configurables import (
            ApplicationMgr,
            AuditorSvc,
            ChronoStatSvc,
            GaudiTestSuiteCommonConf,
            TimingAuditor,
        )

        from Gaudi.Configuration import DEBUG

        GaudiTestSuiteCommonConf()

        ChronoStatSvc(PerEventFile="timing_data.log", OutputLevel=DEBUG)

        auditSvc = AuditorSvc()
        auditSvc.Auditors.append(TimingAuditor("TIMER"))

        app = ApplicationMgr()
        app.TopAlg = ["GaudiTestSuite::TimingAlg/Timing"]
        app.EvtSel = "NONE"  # do not use any event input
        app.EvtMax = 400
        app.ExtSvc.extend(["ToolSvc", auditSvc])
        app.AuditAlgorithms = True

    def test_stdout(self, stdout):
        expected = textwrap.dedent(r"""
            Timing\s+SUCCESS\s+The timing is \(in us\)\s*
            \|\s*\|\s*#\s*\|\s*Total\s*\|\s*Mean\+-RMS\s*\|\s*Min/Max\s*\|\s*
            \|\s*\(1U\)\s*\|\s*\d*\s*\|(?:[-+.\deE /]+\|){3}\s*
            \|\s*\(2U\)\s*\|\s*\d*\s*\|(?:[-+.\deE /]+\|){3}\s*
            \|\s*\(3U\)\s*\|\s*\d*\s*\|(?:[-+.\deE /]+\|){3}\s*
            """).lstrip()  # we have to strip the leading \n

        assert re.search(expected, stdout.decode()), "missing timing report"

    def test_timing_data_log(self, cwd, record_property):
        logfile = cwd / "timing_data.log"
        assert logfile.exists(), "missing timing log file"

        with open(logfile, "r") as f:
            timing_data_log = f.read()
            record_property("log_content", timing_data_log)

        timing_data = timing_data_log.splitlines()
        assert len(timing_data) == 1

        entries = timing_data[0].split()
        assert entries[0] == "Timing"
        assert len(entries) == 401
