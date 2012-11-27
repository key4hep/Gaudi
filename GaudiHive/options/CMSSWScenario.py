import json
from Gaudi.Configuration import *
# ============================================================================
from Configurables import GaudiExamplesCommonConf, CPUCruncher,HiveEventLoopMgr
#GaudiExamplesCommonConf()
# ============================================================================     

#-------------------------------------------------------------------------------
# Metaconfig

NUMBEROFEVENTS = 1
NUMBEROFEVENTSINFLIGHT = 1
NUMBEROFALGOSINFLIGHT = 1000
NUMBEROFTHREADS = 1
CLONEALGOS = False
DUMPQUEUES = False
VERBOSITY = 3


NumberOfEvents = NUMBEROFEVENTS
NumberOfEventsInFlight = NUMBEROFEVENTSINFLIGHT
NumberOfAlgosInFlight = NUMBEROFALGOSINFLIGHT
NumberOfThreads = NUMBEROFTHREADS
CloneAlgos = CLONEALGOS
DumpQueues = DUMPQUEUES
Verbosity = VERBOSITY


def load_CMSSW_scenario(filename):
  data = open(filename).read()
  workflow = eval(data)
  cpu_cruncher_algos = []
  cpu_cruncher_algos_inputs = []
  all_outputs = set()
  all_inputs  = set()
  for algo in workflow["process"]["producers"]:
    # in the next two lines replace the slash as that is a reserved character in the data store 
    inputs = [ item["label"].replace("/","_") for item in algo["toGet"] ]
    outputs = [algo["@label"].replace("/","_"),]
    new_algo = CPUCruncher(algo["@label"],
                           avgRuntime=float(algo["eventTimes"][0]),
                           Inputs = inputs,
                           Outputs = outputs
                          )

    cpu_cruncher_algos.append(new_algo)
    all_outputs.update(outputs)
    all_inputs.update(inputs)
    cpu_cruncher_algos_inputs.append(inputs)
  
  #look for the objects that haven't been provided within the job. Assume this needs to come via input
  new_algo = CPUCruncher("input",
                         avgRuntime=1,
                         Inputs=[],
                         Outputs=[item for item in all_inputs.difference(all_outputs)]
                         )
  cpu_cruncher_algos.append(new_algo)
  cpu_cruncher_algos_inputs.append([])

  print [item for item in all_inputs.difference(all_outputs)]
  return cpu_cruncher_algos,cpu_cruncher_algos_inputs
        
# Set output level threshold 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
ms = MessageSvc() 
ms.OutputLevel     =  Verbosity

crunchers,inputs = load_CMSSW_scenario("CMS_multijet.json")

# Setup the Event Loop Manager
evtloop = HiveEventLoopMgr()
evtloop.MaxAlgosParallel = NumberOfAlgosInFlight
evtloop.MaxEventsParallel = NumberOfEventsInFlight
evtloop.NumThreads = NumberOfThreads 
evtloop.CloneAlgorithms = CloneAlgos
evtloop.DumpQueues = DumpQueues
evtloop.AlgosDependencies = inputs

# And the Application Manager
app = ApplicationMgr()
app.TopAlg = crunchers
app.EvtSel = "NONE" # do not use any event input
app.EvtMax = NumberOfEvents
app.EventLoop = evtloop
#app.MessageSvcType = "TBBMessageSvc"


