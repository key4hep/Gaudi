#####################################################################################
# (c) Copyright 2021 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
def config():
    from GaudiConfig2 import Configurables as C
    from Configurables import ApplicationMgr
    ApplicationMgr(AppName="Test")
    app = C.ApplicationMgr(AppVersion="v100r100")
    return [app]
