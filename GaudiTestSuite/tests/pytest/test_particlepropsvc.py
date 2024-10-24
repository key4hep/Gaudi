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
from GaudiTesting import NO_ERROR_MESSAGES, GaudiExeTest


class TestGaudiPPS(GaudiExeTest):
    command = ["gaudirun.py"]
    environment = [
        "JOBOPTSEARCHPATH=$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/options:$ENV_PROJECT_SOURCE_DIR/Gaudi/GaudiTestSuite/options"
    ]
    reference = {"messages_count": NO_ERROR_MESSAGES}

    def options(self):
        from Configurables import ApplicationMgr
        from Configurables import Gaudi__ParticlePropertySvc as ParticlePropertySvc
        from Configurables import GaudiTestSuite__GaudiPPS as GaudiPPS

        from Gaudi.Configuration import importOptions

        importOptions("Common.opts")

        from pathlib import Path

        ParticlePropertiesFilePath = Path("..") / "data" / "ParticleTable.txt"

        ParticlePropertySvc(ParticlePropertiesFile=str(ParticlePropertiesFilePath))

        ApplicationMgr(EvtMax=1, EvtSel="NONE", TopAlg=[GaudiPPS()])

    def test_stdout(self, stdout):
        expected = b"Opened particle properties file : ../data/ParticleTable.txt"
        assert expected in stdout
