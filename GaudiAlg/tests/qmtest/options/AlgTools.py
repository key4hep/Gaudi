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
from Configurables import GaudiExamplesCommonConf, MyGaudiAlgorithm, MyGaudiTool
from Gaudi.Configuration import *

GaudiExamplesCommonConf()

myalg = MyGaudiAlgorithm("MyAlg")

myalg.addTool(MyGaudiTool(Int=101, Double=101.1e10, String="hundred one", Bool=False))

gtool = MyGaudiTool(
    "MyGaudiTool",
    Int=201,
    Double=201.1e10,
    String="two hundred and one",
    Bool=True,
    OutputLevel=INFO,
)

tool_conf1 = MyGaudiTool(
    "MyTool_conf1", Int=1, Double=2, String="three", Bool=True, OutputLevel=INFO
)

tool_conf2 = MyGaudiTool(
    "MyTool_conf2", Int=10, Double=20, String="xyz", Bool=False, OutputLevel=INFO
)

myToolWithName = myalg.addTool(tool_conf2, "ToolWithName")

myalg.ToolWithName.String = "xyz"

assert myToolWithName.String == "xyz"
assert myalg.ToolWithName == myToolWithName

myToolWithName.String = "abc"

myalg.PrivToolHandle.String = "Is a private tool"

pubtool = MyGaudiTool("TestPubToolHandle", String="Is a public tool")
myalg.PubToolHandle = pubtool

# disable a ToolHandle
myalg.InvalidToolHandle = ""

ApplicationMgr(EvtMax=10, EvtSel="NONE", HistogramPersistency="NONE", TopAlg=[myalg])
# --------------------------------------------------------------
# Test circular tool dependencies  (by Chris Jones)
# --------------------------------------------------------------
from Configurables import TestTool, TestToolAlg

tA = TestTool("ToolA", Tools=["TestTool/ToolB"], OutputLevel=DEBUG)
tB = TestTool("ToolB", Tools=["TestTool/ToolA"], OutputLevel=DEBUG)
testalg = TestToolAlg(Tools=["TestTool/ToolA"])
ApplicationMgr().TopAlg += [testalg]
