#!/usr/bin/env gaudirun.py
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
###############################################################
# Job options file
# ==============================================================
from Configurables import DataCreator, GaudiExamplesCommonConf, MyDataAlgorithm
from Gaudi.Configuration import *

GaudiExamplesCommonConf()

mdigi = DataCreator("MuonDigits", Data="Rec/Muon/Digits")
mfoo = DataCreator("MuonFoos", Data="Rec/Muon/Foos")
mold = DataCreator("MuonOld", Data="Rec/Muon/Old")
dondem = DataOnDemandSvc(
    UsePreceedingPath=True,
    NodeMap={
        "Rec": "DataObject",
        "Rec/Muon": "DataObject",
        "Rec/Foo": "DataObject",
    },
    AlgMap={mdigi.Data: mdigi, mfoo.Data: mfoo},
    # obsolete property:
    Algorithms=["DATA='%s' TYPE='%s'" % (mold.Data, mold.getFullName())],
    # obsolete property:
    Nodes=["DATA='Rec/Obsolete' TYPE='DataObject'"],
    Dump=False,
)

ApplicationMgr(TopAlg=[MyDataAlgorithm()], ExtSvc=[dondem], EvtMax=10, EvtSel="NONE")

EventDataSvc().EnableFaultHandler = True
