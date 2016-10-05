import json
from Gaudi.Configuration import *
# ============================================================================
from Configurables import (GaudiExamplesCommonConf,
                           CPUCruncher,
                           HiveWhiteBoard,
                           ForwardSchedulerSvc,
                           HiveSlimEventLoopMgr)
#GaudiExamplesCommonConf()
# ============================================================================

#-------------------------------------------------------------------------------
# Metaconfig

NUMBEROFEVENTS = 1
NUMBEROFEVENTSINFLIGHT = 1
NUMBEROFALGOSINFLIGHT = 1
NUMBEROFTHREADS = NUMBEROFALGOSINFLIGHT
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


def load_athena_scenario(filename):
  data = open(filename).read()
  workflow = eval(data)
  cpu_cruncher_algos = []
  cpu_cruncher_algos_inputs = []
  all_outputs = set()
  all_inputs  = set()
  for algo in workflow["algorithms"]:
    # Correct in presence of list wi
    for starputs in ("inputs","outputs"):
      algo[starputs] = [item.replace("/","_") for item in algo[starputs]]
      if algo[starputs] == ['']: algo[starputs] = []
      if algo[starputs] == ['dummy']: algo[starputs] = []
    cleaned_inputs = [input for input in algo["inputs"] if (input not in algo["outputs"] ) ]

    # fix double declaration of outputs (nokey)
    cleaned_outputs = [output for output in algo["outputs"] if (output not in all_outputs)]
    new_algo = CPUCruncher(algo["name"],
                           avgRuntime=float(algo["runtimes"][0]/1000000.),
                           inpKeys = cleaned_inputs,
                           outKeys = cleaned_outputs
                           )
    cpu_cruncher_algos.append(new_algo)
    all_outputs.update(algo["outputs"])
    all_inputs.update(algo["inputs"])
    cpu_cruncher_algos_inputs.append(algo["inputs"])

  #look for the objects that haven't been provided within the job. Assume this needs to come via input
  new_algo = CPUCruncher("input",
                         avgRuntime=1,
                         inpKeys=[],
                         outKeys=[item for item in all_inputs.difference(all_outputs)]
                         )
  cpu_cruncher_algos.append(new_algo)
  cpu_cruncher_algos_inputs.append([])

  print [item for item in all_inputs.difference(all_outputs)]

  print len(all_outputs)
  print len(cpu_cruncher_algos)
  return cpu_cruncher_algos,cpu_cruncher_algos_inputs


# Set output level threshold 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
ms = MessageSvc()
ms.OutputLevel     =  Verbosity

crunchers,inputs = load_athena_scenario("Athena_loopfixed.json")

whiteboard   = HiveWhiteBoard("EventDataSvc", EventSlots = NumberOfEventsInFlight)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=DEBUG)

scheduler = ForwardSchedulerSvc(MaxAlgosInFlight = NumberOfAlgosInFlight,
                                ThreadPoolSize = NumberOfThreads,
                                useGraphFlowManagement = True,
                                OutputLevel=INFO)

# And the Application Manager
app = ApplicationMgr()
app.TopAlg = crunchers
app.EvtSel = "NONE" # do not use any event input
app.EvtMax = NumberOfEvents
app.EventLoop = slimeventloopmgr
app.ExtSvc =[whiteboard]
app.MessageSvcType = "InertMessageSvc"
#app.MessageSvcType = "TBBMessageSvc"

