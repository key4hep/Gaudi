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

# Hack needed for the test
from GaudiKernel.Configurable import ConfigurableGeneric
myAlg = ConfigurableGeneric("myAlg")
myAlg.Dict = {
    "a": 1,
    "b": 2,
    # "c":3 commented-out
}  # another comment

importOptions("test1.opts")
importOptions("test2.opts")
importOptions("test1.opts")

importOptions("test1.py")
