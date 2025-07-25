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
from Configurables import ApplicationMgr
from Configurables import Gaudi__TestSuite__OpenCLAlg as OpenCLAlg
from Gaudi.Configuration import DEBUG

alg = OpenCLAlg("OpenCLAlg", OutputLevel=DEBUG)

ApplicationMgr(TopAlg=[alg], EvtSel="NONE", EvtMax=5)
