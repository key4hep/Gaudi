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
from __future__ import print_function

from Configurables import GaudiExamplesCommonConf, MyGaudiAlgorithm

###############################################################
# Job options file
# ==============================================================
from Gaudi.Configuration import *

GaudiExamplesCommonConf()

mygalg = MyGaudiAlgorithm("MyGaudiAlg", OutputLevel=DEBUG)
mygalg.PrivToolHandle.String = "Is a private tool"

print(mygalg)

mygalg.tracks.Path = "BestTracks/Tracks"
mygalg.hits.Path = "Rec/Hits"

mygalg.RootInTES = "/Skim"

from Configurables import MyTool

mygalg.MyPublicToolHandleArrayProperty = [
    MyTool("FirstInstance"),
    MyTool("AnotherInstance"),
]

print(mygalg)

ApplicationMgr(
    EvtMax=10,
    EvtSel="NONE",
    HistogramPersistency="NONE",
    TopAlg=[mygalg],
    OutputLevel=DEBUG,
)
