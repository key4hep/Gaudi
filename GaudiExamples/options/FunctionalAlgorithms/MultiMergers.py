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
from Configurables import (
    ApplicationMgr,
    EvtStoreSvc,
    Gaudi__Examples__IntDataProducer as IntDataProducer,
    is2ff_merger,
    is2ff_merger_filter,
)

int_a = IntDataProducer("IntAProducer", Value=2, OutputLocation="/Event/IntA")
int_b = IntDataProducer("IntBProducer", Value=3, OutputLocation="/Event/IntB")
merger = is2ff_merger(
    InputInts=[str(int_a.OutputLocation),
               str(int_b.OutputLocation)])
# This filter should set its status to 'passed' as 2 * 2 * 3 > 10
merger_filter_passing = is2ff_merger_filter(
    "MergerFilterPassing",
    InputInts=[
        str(int_a.OutputLocation),
        str(int_a.OutputLocation),
        str(int_b.OutputLocation),
    ],
    O1="/Event/MF/Float1",
    O2="/Event/MF/Float2")
# This filter should set its status to 'failed' as 2 * 3 < 10
merger_filter_failing = is2ff_merger_filter(
    "MergerFilterFailing",
    InputInts=[
        str(int_a.OutputLocation),
        str(int_b.OutputLocation),
    ],
    O1="/Event/MFSwapped/Float1",
    O2="/Event/MFSwapped/Float2")

app = ApplicationMgr(
    EvtMax=2,
    EvtSel="NONE",
    ExtSvc=[EvtStoreSvc("EventDataSvc")],
    HistogramPersistency="NONE",
    TopAlg=[
        int_a,
        int_b,
        merger,
        merger_filter_passing,
        merger_filter_failing,
    ],
)
