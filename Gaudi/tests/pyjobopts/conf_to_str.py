#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from Gaudi.Configuration import *

MessageSvc().OutputLevel = INFO


class MyAlg(ConfigurableUser):
    __slots__ = {
        "OutputLevel": INFO,
        "Service": "DefaultService",
        "Members": ["HelloWorld"]
    }

    def getGaudiType(self):
        return "Test"  # To avoid skipping of ConfigurableUser


class MySvc(ConfigurableUser):
    __slots__ = {"OutputLevel": INFO}

    def getGaudiType(self):
        return "Test"  # To avoid skipping of ConfigurableUser


mySvc = MySvc("JustAName", OutputLevel=VERBOSE)

myAlg1 = MyAlg("Algorithm1", Service=mySvc)

myAlg2 = MyAlg("Algorithm2", Members=["HelloWorld", myAlg1])

myAlg3 = MyAlg("Algorithm3", Members=[myAlg1, "HelloWorld", myAlg2])
