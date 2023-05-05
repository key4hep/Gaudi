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
from Gaudi.Configuration import ConfigurableUser, importOptions


class MyAlgType(ConfigurableUser):
    __slots__ = {"Cut": 0.0, "Tool1": "None", "Tool2": "None", "File": "None"}

    def getGaudiType(self):
        return "Test"  # To avoid skipping of ConfigurableUser


class MyToolType(ConfigurableUser):
    __slots__ = {"Cut": 0.0}

    def getGaudiType(self):
        return "Test"  # To avoid skipping of ConfigurableUser


tool = MyToolType()
alg = MyAlgType("MyAlg")

alg.Tool2 = tool

importOptions("reference.opts")

alg.Cut = 20.0
