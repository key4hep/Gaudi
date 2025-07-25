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
#
# Write a DST and a miniDST, including File Summary Records
#

from Configurables import Gaudi__TestSuite__FloatDataConsumer as FloatDataConsumer
from Configurables import Gaudi__TestSuite__IntDataConsumer as IntDataConsumer
from Configurables import Gaudi__TestSuite__IntDataProducer as IntDataProducer
from Configurables import (
    Gaudi__TestSuite__IntIntToFloatFloatData as IntIntToFloatFloatData,
)
from Configurables import Gaudi__TestSuite__IntToFloatData as IntToFloatData
from Configurables import TimelineSvc
from Gaudi.Configuration import *

# Application setup
app = ApplicationMgr()
# - Algorithms
OtherIntDataProducer = IntDataProducer("OtherIntDataProducer")
OtherIntDataProducer.OutputLocation = "/Event/MyOtherInt"
app.TopAlg = [
    IntDataProducer("IntDataProducer"),
    OtherIntDataProducer,
    IntDataConsumer("IntDataConsumer"),
    IntToFloatData("IntToFloatData"),
    IntIntToFloatFloatData("IntIntToFloatFloatData"),
    FloatDataConsumer("FloatDataConsumer"),
]
# - Events
app.EvtMax = 2
app.EvtSel = "NONE"
app.HistogramPersistency = "NONE"
# - Activate timeline service
TimelineSvc(RecordTimeline=True, DumpTimeline=True, TimelineFile="myTimeline.csv")
