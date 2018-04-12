import json
from Gaudi.Configuration import *
# ============================================================================
from Configurables import GaudiExamplesCommonConf, CPUCruncher, HiveSlimEventLoopMgr, HiveWhiteBoard, InertMessageSvc, ForwardSchedulerSvc
# ============================================================================
'''
Json file format:

"algorithms" : [
  {
    "name" : "Loader",
    "type" : "HiveReader",
    "inputs" : ["ROOT File"],
    "outputs" : ["DAQ/ODIN", "DAQ/RawEvent"
    
    ],
    "runtimes_wall" : [0.05]
    },
'''
# -------------------------------------------------------------------------------
# Metaconfig

NUMBEROFEVENTS = 828
NUMBEROFEVENTSINFLIGHT = 1
NUMBEROFALGOSINFLIGHT = 100
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


miniBruneljson = {
    "algorithms": [
        {
            "name": "Loader",
            "type": "HiveReader",
            "inputs": [],
            "outputs": ["/Event/DAQ/ODIN", "/Event/DAQ/RawEvent"

                        ],
            "runtimes_wall": [0.16]
        },
        {
            "name": "BrunelInit",
            "type": "RecInit",
            "inputs": ["DAQ/ODIN", "DAQ/RawEvent"],
            "outputs": ["/Event/Rec/Status", "/Event/Rec/Header"],
            "runtimes_wall": [0.183]
        },
        {
            "name": "BrunelEventCount",
            "type": "EventCountHisto",
            "inputs": [],
            "outputs": [],
            "runtimes_wall": [2.000000e-03]
        },
        {
            "name": "HltDecReportsDecoder",
            "type": "HltDecReportsDecoder",
            "inputs": ["DAQ/RawEvent"],
            "outputs": ["/Event/Hlt/DecReports"],
            "runtimes_wall": [2.3]
        },
        {
            "name": "PrsFromRaw",
            "type": "CaloDigitsFromRaw",
            "inputs": ["DAQ/RawEvent"],
            "outputs": ["/Event/Raw/Prs/Digits"],
            "runtimes_wall": [0.357]
        },
        {
            "name": "EcalZSup",
            "type": "CaloZSupAlg",
            "inputs": ["DAQ/RawEvent"],
            "outputs": ["/Event/Raw/Ecal/Digits"],
            "runtimes_wall": [0.750]
        },
        {
            "name": "HcalZSup",
            "type": "CaloZSupAlg",
            "inputs": ["DAQ/RawEvent"],
            "outputs": ["/Event/Raw/Hcal/Digits"],
            "runtimes_wall": [0.197]
        },
        {
            "name": "L0DUFromRaw",
            "type": "L0DUFromRawAlg",
            "inputs": ["DAQ/RawEvent"],
            "outputs": ["/Event/Trig/L0/L0DUReport"],
            "runtimes_wall": [0.293]
        },
        {
            "name": "DecodeVeloClusters",
            "type": "DecodeVeloRawBuffer",
            "inputs": ["DAQ/RawEvent"],
            "outputs": ["/Event/Raw/Velo/LiteClusters", "/Event/Raw/Velo/Clusters"],
            "runtimes_wall": [1.095]
        },
        {
            "name": "CreateTTClusters",
            "type": "RawBankToSTClusterAlg",
            "inputs": ["DAQ/ODIN", "DAQ/RawEvent"],
            "outputs": ["/Event/Raw/TT/Clusters", "/Event/Rec/TT/Summary"],
            "runtimes_wall": [0.809]
        },
        {
            "name": "CreateTTLiteClusters",
            "type": "RawBankToSTLiteClusterAlg",
            "inputs": ["DAQ/ODIN", "DAQ/RawEvent"],
            "outputs": ["/Event/Raw/TT/LiteClusters"],
            "runtimes_wall": [0.208]
        },
        {
            "name": "CreateITClusters",
            "type": "RawBankToSTClusterAlg",
            "inputs": ["DAQ/ODIN", "DAQ/RawEvent"],
            "outputs": ["/Event/Raw/IT/Clusters", "/Event/Rec/IT/Summary"],
            "runtimes_wall": [0.672]
        },
        {
            "name": "CreateITLiteClusters",
            "type": "RawBankToSTLiteClusterAlg",
            "inputs": ["DAQ/ODIN", "DAQ/RawEvent"],
            "outputs": ["/Event/Raw/IT/LiteClusters"],
            "runtimes_wall": [0.177]
        },
        {
            "name": "FastVeloTracking",
            "type": "FastVeloTracking",
            "inputs": ["Raw/Velo/LiteClusters"],
            "outputs": ["/Event/Rec/Track/Velo"],
            "runtimes_wall": [4.3]
        }
    ]
}


def load_scenario():
    workflow = miniBruneljson
    cpu_cruncher_algos = []
    for algo in workflow["algorithms"]:
        theAvgRuntime = float(algo["runtimes_wall"][0] / 1000.)
        new_algo = CPUCruncher("%s@%s" % (algo["name"], algo["type"]),
                               avgRuntime=theAvgRuntime,
                               varRuntime=theAvgRuntime * 0.01,
                               DataInputs=algo["inputs"],
                               DataOutputs=algo["outputs"],
                               OutputLevel=INFO
                               )
        cpu_cruncher_algos.append(new_algo)
    return cpu_cruncher_algos

# Set output level threshold 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )


msgSvc = InertMessageSvc("MessageSvc", OutputLevel=INFO)
ApplicationMgr().SvcMapping.append(msgSvc)
ApplicationMgr(MessageSvcType=msgSvc.getType(),
               OutputLevel=INFO)

crunchers = load_scenario()

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=NumberOfEventsInFlight)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

scheduler = ForwardSchedulerSvc(MaxEventsInFlight=NumberOfEventsInFlight,
                                MaxAlgosInFlight=100,
                                OutputLevel=WARNING)

# And the Application Manager
app = ApplicationMgr()
app.TopAlg = crunchers
app.EvtSel = "NONE"  # do not use any event input
app.EvtMax = NumberOfEvents
app.EventLoop = slimeventloopmgr
app.ExtSvc = [whiteboard]
