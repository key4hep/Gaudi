#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
"""
Bunch of fake configurables used for testing.
"""

from GaudiKernel.DataHandle import DataHandle
from GaudiKernel.GaudiHandles import PrivateToolHandle, PublicToolHandle
from GaudiKernel.Proxy.Configurable import Configurable, ConfigurableAlgTool


class APublicTool(ConfigurableAlgTool):
    __slots__ = {}

    def __init__(self, name=Configurable.DefaultName, **kwargs):
        super(APublicTool, self).__init__(name)
        for n, v in kwargs.items():
            setattr(self, n, v)

    def getDlls(self):
        return "None"

    def getType(self):
        return "APublicTool"


class APrivateTool(ConfigurableAlgTool):
    __slots__ = {}

    def __init__(self, name=Configurable.DefaultName, **kwargs):
        super(APrivateTool, self).__init__(name)
        for n, v in kwargs.items():
            setattr(self, n, v)

    def getDlls(self):
        return "None"

    def getType(self):
        return "APrivateTool"


class MyTestTool(ConfigurableAlgTool):
    __slots__ = {
        "PubToolHndl": PublicToolHandle("APublicTool"),
        "PrivToolHndl": PrivateToolHandle("APrivateTool"),
        "PrivEmptyToolHndl": PrivateToolHandle(""),
        "Text": "some text",
        "Int": 23,
        "DataHandle": DataHandle("Location", "R"),
    }

    def __init__(self, name=Configurable.DefaultName, **kwargs):
        super(MyTestTool, self).__init__(name)
        for n, v in kwargs.items():
            setattr(self, n, v)

    def getDlls(self):
        return "None"

    def getType(self):
        return "MyTestTool"
