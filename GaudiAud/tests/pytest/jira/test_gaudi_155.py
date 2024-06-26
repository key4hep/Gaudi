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
import re

from GaudiTesting import GaudiExeTest


class TestAlgContextSvcWarning(GaudiExeTest):
    "https://its.cern.ch/jira/browse/GAUDI-155"

    command = ["Gaudi.exe"]

    options = """
    AuditorSvc.Auditors += { 'AlgContextAuditor' } ;
    ApplicationMgr.ExtSvc += { 'AuditorSvc','AlgContextSvc' } ;
    ApplicationMgr.AuditServices = true ;

    ApplicationMgr.EvtMax    = 1;   // events to be processed (default is 10)
    ApplicationMgr.EvtSel   = 'NONE'; // do not use any event input
    """

    def test_stdout(self, stdout):
        assert not re.search(
            rb"AlgContextSvc.*WARNING.*NULL", stdout
        ), "warning in AlgContextSvc for NULL pointer"
