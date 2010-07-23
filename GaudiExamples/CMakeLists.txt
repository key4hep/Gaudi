###############################################
# CMakeList.txt file for building GaudiExamples
# @author Pere Mato, CERN
###############################################

GAUDI_USE_PACKAGE(CLHEP)
GAUDI_USE_PACKAGE(GSL)
GAUDI_USE_PACKAGE(HepPDT)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/src/Lib ${GAUDI_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${GSL_INCLUDE_DIRS} 
${HepPDT_INCLUDE_DIRS})
LINK_DIRECTORIES( ${GAUDI_LIBRARY_DIRS} ${CLHEP_LIBRARY_DIRS} ${GSL_LIBRARY_DIRS} ${HepPDT_LIBRARY_DIRS}) 

#---Linker Library-------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiExamplesLib Lib/*.cpp GaudiKernel)

#---Component Library----------------------------------------------------------
SET(srcs  AbortEvent/*.cpp 
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
                          StatusCodeSvc/*.cpp 
                          StringKeys/*.cpp 
                          testing/*.cpp 
                          Dll/GaudiExamples*.cpp )

GAUDI_COMPONENT_LIBRARY(GaudiExamples "${srcs}" GaudiExamplesLib GaudiAlgLib GaudiGSLLib ${CLHEP_LIBRARIES} ${HepPDT_LIBRARIES} Tree RIO)

#---Dictionary-----------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiExamples src/POOLIO/dict.h src/POOLIO/dict.xml GaudiExamplesLib)

SET(GaudiExamples_environment STDOPTS=${CMAKE_SOURCE_DIR}/GaudiExamples/options
                              JOBOPTSEARCHPATH+=/NONEXISTINGDIR
                              JOBOPTSEARCHPATH+=${CMAKE_SOURCE_DIR}/GaudiExamples/options 
                              PYTHONPATH+=${CMAKE_SOURCE_DIR}/GaudiExamples/tests/qmtest 
                              PARENT_SCOPE)




