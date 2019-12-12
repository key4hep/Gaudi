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
from Gaudi.Configuration import *

ApplicationMgr().ExtSvc += ["RndmGenSvc"]

# Run various tupling algorithms
from Configurables import TupleAlg, TupleAlg2, TupleAlg3
ApplicationMgr().TopAlg = [
    TupleAlg("Tuple", NTupleLUN="MYLUN"),
    TupleAlg2("Tuple2", NTupleLUN="MYLUN"),
    TupleAlg3("Tuple3", NTupleLUN="MYLUN")
]

# Output level
MessageSvc().OutputLevel = 3

# 10000 'events', no input
ApplicationMgr().EvtMax = 10000
ApplicationMgr().EvtSel = "NONE"

# Output ROOT tuple file name
ApplicationMgr().HistogramPersistency = "ROOT"
NTupleSvc().Output = ["MYLUN DATAFILE='TupleEx.root' OPT='NEW' TYP='ROOT'"]

# Set the compression level for the ROOT tuple file
from GaudiKernel.Configurable import ConfigurableGeneric as RFileCnv
RFileCnv('RFileCnv').GlobalCompression = "LZMA:6"
