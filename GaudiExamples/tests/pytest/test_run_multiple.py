#####################################################################################
# (c) Copyright 2022 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
def test_run_multiple_times():
    """Ensure that Gaudi.Application can be reused after deletion"""
    import Gaudi

    app = Gaudi.Application({"ApplicationMgr.JobOptionsType": '"NONE"'})
    app.run()
    del app

    app = Gaudi.Application({"ApplicationMgr.JobOptionsType": '"NONE"'})
    app.run()
    del app
