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
from GaudiTesting import GaudiExeTest


class Test71216(GaudiExeTest):
    """
    https://its.cern.ch/jira/browse/GAUDI-357
    """

    command = ["gaudirun.py"]

    def options(self):
        from Configurables import ApplicationMgr, NTupleAlgorithm, NTupleSvc

        # Try to write in a directory that doesn't exist
        NTupleSvc(
            Output=["MyTuples DATAFILE='/random/path/ntest.root' TYP='ROOT' OPT='NEW'"]
        )
        # HistogramPersistencySvc(OutputFile = "/random/path/htest.root")

        app = ApplicationMgr(  # noqa: F841
            TopAlg=[NTupleAlgorithm()],
            HistogramPersistency="ROOT",
            EvtSel="NONE",
            EvtMax=5,
        )

    def test_stderr(self, stderr: bytes) -> None:
        stderr = b"\n".join(
            l
            for l in stderr.splitlines()
            if b"file /random/path/ntest.root can not be opened" not in l
        )
        assert not stderr.strip()
