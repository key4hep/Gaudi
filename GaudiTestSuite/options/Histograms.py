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
from Configurables import Gaudi__Monitoring__MessageSvcSink as MessageSvcSink
from Gaudi.Configuration import *

AuditorSvc().Auditors = ["ChronoAuditor"]
MessageSvc().OutputLevel = INFO
RootHistSvc("RootHistSvc").OutputFile = "histo.root"

from Configurables import Gaudi__Histograming__Sink__Root as RootHistoSink
from Configurables import (
    Gaudi__TestSuite__Counter__GaudiHistoAlgorithm as CounterHistoAlg,
)
from Configurables import (
    Gaudi__TestSuite__Counter__GaudiRootHistoAlgorithm as RootCounterHistoAlg,
)

histoAlg = CounterHistoAlg(
    "SimpleCounterHistos",
    OutputLevel=DEBUG,
    CustomGauss_Title="Gaussian mean=0, sigma=1, atomic",
    CustomGauss_Axis0=(100, -5, 5, "X"),
    CustomGaussFlat_Title="Gaussian V Flat, atomic",
    CustomGaussFlat_Axis0=(50, -5, 5, "X"),
    CustomGaussFlat_Axis1=(50, -5, 5, "Y"),
    CustomGaussFlatGauss_Title="Gaussian V Flat V Gaussian, atomic",
    CustomGaussFlatGauss_Axis0=(10, -5, 5, "X"),
    CustomGaussFlatGauss_Axis1=(10, -5, 5, "Y"),
    CustomGaussFlatGauss_Axis2=(10, -5, 5, "Z"),
    CustomGaussW_Title="Gaussian mean=0, sigma=1, weighted",
    CustomGaussW_Axis0=(100, -5, 5),
    CustomGaussFlatW_Title="Gaussian V Flat, weighted",
    CustomGaussFlatW_Axis0=(50, -5, 5),
    CustomGaussFlatW_Axis1=(50, -5, 5),
    CustomGaussFlatGaussW_Title="Gaussian V Flat V Gaussian, weighted",
    CustomGaussFlatGaussW_Axis0=(10, -5, 5),
    CustomGaussFlatGaussW_Axis1=(10, -5, 5),
    CustomGaussFlatGaussW_Axis2=(10, -5, 5),
    CustomProfGauss_Title="Profile, Gaussian mean=0, sigma=1, atomic",
    CustomProfGauss_Axis0=(100, -5, 5),
    CustomProfGaussFlat_Title="Profile, Gaussian V Flat, atomic",
    CustomProfGaussFlat_Axis0=(50, -5, 5),
    CustomProfGaussFlat_Axis1=(50, -5, 5),
    CustomProfGaussFlatGauss_Title="Profile, Gaussian V Flat V Gaussian, atomic",
    CustomProfGaussFlatGauss_Axis0=(10, -5, 5),
    CustomProfGaussFlatGauss_Axis1=(10, -5, 5),
    CustomProfGaussFlatGauss_Axis2=(10, -5, 5),
    CustomProfGaussW_Title="Profile, Gaussian mean=0, sigma=1, weighted",
    CustomProfGaussW_Axis0=(100, -5, 5),
    CustomProfGaussFlatW_Title="Profile, Gaussian V Flat, weighted",
    CustomProfGaussFlatW_Axis0=(50, -5, 5),
    CustomProfGaussFlatW_Axis1=(50, -5, 5),
    CustomProfGaussFlatGaussW_Title="Profile, Gaussian V Flat V Gaussian, weighted",
    CustomProfGaussFlatGaussW_Axis0=(10, -5, 5),
    CustomProfGaussFlatGaussW_Axis1=(10, -5, 5),
    CustomProfGaussFlatGaussW_Axis2=(10, -5, 5),
    CustomGaussNoInit_Title="Gaussian mean=0, sigma=1, atomic",
    CustomGaussNoInit_Axis0=(100, -5, 5, "X"),
)

rootHistoAlg = RootCounterHistoAlg(
    "SimpleRootCounterHistos",
    OutputLevel=DEBUG,
    CustomGauss_Title="Gaussian mean=0, sigma=1, atomic",
    CustomGauss_Axis0=(100, -5, 5, "X"),
    CustomGaussFlat_Title="Gaussian V Flat, atomic",
    CustomGaussFlat_Axis0=(50, -5, 5, "X"),
    CustomGaussFlat_Axis1=(50, -5, 5, "Y"),
    CustomGaussFlatGauss_Title="Gaussian V Flat V Gaussian, atomic",
    CustomGaussFlatGauss_Axis0=(10, -5, 5, "X"),
    CustomGaussFlatGauss_Axis1=(10, -5, 5, "Y"),
    CustomGaussFlatGauss_Axis2=(10, -5, 5, "Z"),
)

app = ApplicationMgr(
    EvtMax=50000,
    EvtSel="NONE",
    HistogramPersistency="ROOT",
    TopAlg=[histoAlg, rootHistoAlg],
    ExtSvc=[
        MessageSvcSink(TypesToSave=["counter:.*", "histogram:.*"]),
        RootHistoSink(),
    ],
)
