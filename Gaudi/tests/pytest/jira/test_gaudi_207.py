#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
def test():
    "https://its.cern.ch/jira/browse/GAUDI-1179"
    try:
        from Gaudi.Configuration import ApplicationMgr, OutputStream

        ApplicationMgr().OutStream = OutputStream("Test")
    except ValueError:
        return  # success
    assert False, "exception ValueError not raised"
