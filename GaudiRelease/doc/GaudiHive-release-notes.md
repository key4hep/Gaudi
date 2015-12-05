# GaudiHive release v0.6 (2014-04-22)

Changes:
- Scheduling
  + Graph-based decision making unit for execution flow management:
    * unified control and data flow handling
    * predictive Data Flow management
    * new traversal strategy of the Control Flow graph realm
    * use via the useUnifiedFlowManager option of ForwardSchedulerSvc

- Data object tracking
  + declaration of inputs and outputs via declareInput/declareOutput in
    Algorithms and Tools
  + deprecation of declareDataObj

- Tool tracking
  + declaration of used Tools via declarePrivateTool/declarePublicTool

- TimelineSvc (beta)
  + timeline of the execution of algorithms
  + keeping track of used event context (slot) and thread

- Algorithms and Tools of MiniBrunel workflow adapted to new declaration scheme
- see GaudiExamples/src/AlgTools/MyGaudiAlgorithm for example of tool declaration
- see GaudiExamples/src/POOLIO/ReadHandleAlg or WriteHandleAlg for data handle usage
