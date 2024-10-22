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
from GaudiTesting import GaudiExeTest


class Test76642(GaudiExeTest):
    command = ["gaudirun.py"]
    returncode = 3

    def options(self):
        from Configurables import ApplicationMgr, GaudiPersistency

        from Gaudi.Configuration import InputCopyStream, OutputStream

        GaudiPersistency()

        out = OutputStream(
            Output="DATAFILE='PFN:76642_a.dst' SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"
        )
        out.ItemList = ["/Event#999"]
        copy = InputCopyStream(
            Output="DATAFILE='PFN:76642_b.dst' SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"
        )

        ApplicationMgr(OutStream=[out, copy], EvtSel="NONE", EvtMax=5)

    def test_check_line(self, stdout):
        expected = b"ERROR Origin of data has changed"
        assert expected in stdout
