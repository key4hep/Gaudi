############################################################################
# CMakeLists.txt file for building GaudiExamples
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(ROOT)
GAUDI_USE_PACKAGE(AIDA)
GAUDI_USE_PACKAGE(PartPropSvc)
GAUDI_USE_PACKAGE(GaudiPoolDb)
GAUDI_USE_PACKAGE(GaudiPython)
GAUDI_USE_PACKAGE(GaudiGSL)
GAUDI_USE_PACKAGE(RootHistCnv)
GAUDI_USE_PACKAGE(Boost)
GAUDI_USE_PACKAGE(HepPDT)


INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/src/Lib )

#---Libraries---------------------------------------------------------------
SET( GaudiExamples_srcs AbortEvent/*.cpp 
AlgSequencer/*.cpp 
AlgTools/*.cpp 
DataOnDemand/*.cpp 
AlgErrAud/*.cpp 
GSLTools/*.cpp 
RandomNumber/*.cpp 
Histograms/*.cpp 
NTuples/*.cpp 
TupleEx/*.cpp 
CounterEx/*.cpp 
PartProp/*.cpp 
Properties/*.cpp 
ExtendedProperties/*.cpp 
DataListener/*.cpp 
POOLIO/*.cpp 
ColorMsg/ColorMsgAlg.cpp 
History/History.cpp 
THist/*.cpp 
ErrorLog/ErrorLogTest.cpp 
EvtColsEx/EvtColAlg.cpp 
FastContainers/*.cpp 
Maps/*.cpp 
MultipleLogStreams/*.cpp 
GaudiCommonTests/*.cpp 
IncidentSvc/*.cpp 
bug_34121/*.cpp 
Auditors/*.cpp 
Timing/*.cpp 
Selections/*.cpp 
SvcInitLoop/*.cpp 
StringKeys/*.cpp 
StatusCodeSvc/*.cpp 
testing/*.cpp )

GAUDI_COMPONENT_LIBRARY(GaudiExamples "${GaudiExamples_srcs}" GaudiExamplesLib)
GAUDI_LINKER_LIBRARY(GaudiExamplesLib Lib/*.cpp GaudiGSLLib GaudiUtilsLib HepPDT HepPID)

#---Executables-------------------------------------------------------------
GAUDI_EXECUTABLE(Allocator Allocator/*.cpp GaudiExamplesLib GaudiAlgLib GaudiKernel ${ROOT_LIBRARIES} RIO Hist ${CLHEP_LIBRARIES} ${GSL_LIBRARIES})

#---Dictionaries------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiExamples src/POOLIO/dict.h  src/POOLIO/dict.xml  GaudiExamplesLib)


SET(GaudiExamples_environment STDOPTS=${CMAKE_SOURCE_DIR}/GaudiExamples/options
                              JOBOPTSEARCHPATH+=/NONEXISTINGDIR
                              JOBOPTSEARCHPATH+=${CMAKE_SOURCE_DIR}/GaudiExamples/options 
                              PYTHONPATH+=${CMAKE_SOURCE_DIR}/GaudiExamples/tests/qmtest 
                              PARENT_SCOPE)




