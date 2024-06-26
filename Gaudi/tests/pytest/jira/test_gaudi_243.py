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
from conftest import OptParseTest


class Test(OptParseTest):
    """
    https://its.cern.ch/jira/browse/GAUDI-243
    """

    def options(self):
        from Configurables import ApplicationMgr

        from Gaudi.Configuration import ConfigurableUser

        class Brunel(ConfigurableUser):
            __slots__ = {
                "EvtMax": -1,  # Maximum number of events to process
            }

        Brunel().EvtMax = 10
        ApplicationMgr().EvtMax = 20

    expected_dump = {"ApplicationMgr": {"EvtMax": 20}}
