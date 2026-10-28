#####################################################################################
# (c) Copyright 2024-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiTesting import GaudiExeTest


class Test(GaudiExeTest):
    command = [
        "gaudirun.py",
        "-v",
        "../../../options/ROOT_IO/WriteAndReadHandleWhiteBoard.py",
    ]
    timeout = 30

    def options():
        from Configurables import ApplicationMgr, IncidentSvc, OutputStream
        from Configurables import Gaudi__TestSuite__ReadHandleAlg as ReadHandleAlg
        from Configurables import Gaudi__TestSuite__WriteHandleAlg as WriteHandleAlg

        from Gaudi.Configuration import WARNING

        ApplicationMgr(EvtMax=5000)
        # Extra registry entries expose concurrent reference writing if RootMini's
        # dependency on Writer is missing.
        WriteHandleAlg(
            "Writer",
            OutputLevel=WARNING,
            AdditionalOutputs=[f"/Event/Extra{i}" for i in range(31)],
        )
        ReadHandleAlg("Reader", OutputLevel=WARNING)
        OutputStream("RootMini", OutputLevel=WARNING)
        IncidentSvc(OutputLevel=WARNING)
