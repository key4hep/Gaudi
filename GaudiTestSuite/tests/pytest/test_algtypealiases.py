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


class TestAlgTypeAliases(GaudiExeTest):
    command = ["gaudirun.py"]
    reference = "refs/AlgTypeAliases.yaml"

    def options(self):
        from Configurables import ApplicationMgr

        app = ApplicationMgr(EvtSel="NONE", EvtMax=4)

        app.TopAlg = ["HelloWorld/HW1", "unalias:HelloWorld/HW2"]

        app.AlgTypeAliases["HelloWorld"] = "SubAlg"
