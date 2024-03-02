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
from Configurables import Gaudi__TestSuite__TestObjectVersion__CreateObject as CO
from Configurables import Gaudi__TestSuite__TestObjectVersion__UseObject as UO
from Gaudi.Configuration import ApplicationMgr

ApplicationMgr(TopAlg=[CO(), UO()], EvtSel="NONE", EvtMax=1)
