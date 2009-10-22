###############################################################
# Job options file
#==============================================================
from Gaudi.Configuration import *
from Configurables import MyAlgorithm, MyTool, MyGaudiAlgorithm

importOptions('Common.opts')

MessageSvc().OutputLevel = INFO

myalg = MyAlgorithm( 'MyAlg' )

myalg.addTool( MyTool( Int = 101,
                       Double = 101.1e+10,
                       String = "hundred one",
                       Bool = False ) ) 

gtool = MyTool( 'MyTool', 
                Int = 201,
                Double = 201.1e+10,
                String = "two hundred and one",
                Bool = True,
                OutputLevel = INFO )

tool_conf1 = MyTool( 'MyTool_conf1',
                     Int = 1,
                     Double = 2,
                     String = "three",
                     Bool = True,
                     OutputLevel = INFO )

tool_conf2 = MyTool( 'MyTool_conf2',
                     Int = 10,
                     Double = 20,
                     String = "xyz",
                     Bool = False,
                     OutputLevel = INFO )


myalg.addTool(tool_conf2,"ToolWithName")

myalg.ToolWithName.String = "abc"

mygalg = MyGaudiAlgorithm('MyGaudiAlg')
mygalg.PrivToolHandle.String = "Is a private tool"

pubtool = MyTool('TestPubToolHandle', String = "Is a public tool")
mygalg.PubToolHandle = pubtool

ApplicationMgr( EvtMax = 10,
                EvtSel = 'NONE',
                HistogramPersistency = 'NONE',
                TopAlg = [myalg, mygalg] )
#--------------------------------------------------------------
# Test circular tool dependencies  (by Chris Jones)
#--------------------------------------------------------------
from Configurables import TestToolAlg, TestTool

tA = TestTool('ToolA', Tools = ['TestTool/ToolB'], OutputLevel = DEBUG )
tB = TestTool('ToolB', Tools = ['TestTool/ToolA'], OutputLevel = DEBUG )
testalg = TestToolAlg( Tools = ['TestTool/ToolA'])
ApplicationMgr().TopAlg += [ testalg ]

