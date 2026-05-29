#####################################################################################
# (c) Copyright 2021-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from Configurables import ApplicationMgr, EvtStoreSvc
from Configurables import Gaudi__TestSuite__IntDataProducer as IntDataProducer
from Configurables import (
    is_merger_consumer_with_scalar,
    is2f_merger_with_scalar,
    is2ff_merger,
    is2ff_merger_filter,
    is2ff_merger_filter_with_scalar,
    is2ff_merger_with_scalar,
)

int_a = IntDataProducer("IntAProducer", Value=2, OutputLocation="/Event/IntA")
int_b = IntDataProducer("IntBProducer", Value=3, OutputLocation="/Event/IntB")
scale = IntDataProducer("ScaleProducer", Value=4, OutputLocation="/Event/Scale")
scalar_merger = is2f_merger_with_scalar(
    InputInts=[str(int_a.OutputLocation), str(int_b.OutputLocation)],
    Scale=str(scale.OutputLocation),
)
scalar_merger_consumer = is_merger_consumer_with_scalar(
    InputInts=[str(int_a.OutputLocation), str(int_b.OutputLocation)],
    Scale=str(scale.OutputLocation),
)
scalar_multi_merger = is2ff_merger_with_scalar(
    InputInts=[str(int_a.OutputLocation), str(int_b.OutputLocation)],
    Scale=str(scale.OutputLocation),
    O1="/Event/ScalarMM/Float1",
    O2="/Event/ScalarMM/Float2",
)
merger = is2ff_merger(InputInts=[str(int_a.OutputLocation), str(int_b.OutputLocation)])
# This filter should set its status to 'passed' as 2 * 2 * 3 > 10
merger_filter_passing = is2ff_merger_filter(
    "MergerFilterPassing",
    InputInts=[
        str(int_a.OutputLocation),
        str(int_a.OutputLocation),
        str(int_b.OutputLocation),
    ],
    O1="/Event/MF/Float1",
    O2="/Event/MF/Float2",
)
# This filter should set its status to 'failed' as 2 * 3 < 10
merger_filter_failing = is2ff_merger_filter(
    "MergerFilterFailing",
    InputInts=[
        str(int_a.OutputLocation),
        str(int_b.OutputLocation),
    ],
    O1="/Event/MFSwapped/Float1",
    O2="/Event/MFSwapped/Float2",
)
scalar_merger_filter = is2ff_merger_filter_with_scalar(
    InputInts=[str(int_a.OutputLocation), str(int_b.OutputLocation)],
    Scale=str(scale.OutputLocation),
    O1="/Event/ScalarMFF/Float1",
    O2="/Event/ScalarMFF/Float2",
)

app = ApplicationMgr(
    EvtMax=2,
    EvtSel="NONE",
    ExtSvc=[EvtStoreSvc("EventDataSvc")],
    HistogramPersistency="NONE",
    TopAlg=[
        int_a,
        int_b,
        scale,
        scalar_merger,
        scalar_merger_consumer,
        scalar_multi_merger,
        merger,
        merger_filter_passing,
        merger_filter_failing,
        scalar_merger_filter,
    ],
)
