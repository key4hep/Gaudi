############################################################################
# CMakeLists.txt file for building GaudiExamples
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
find_package(Boost COMPONENTS python)
include_directories(${Boost_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})

find_package(AIDA)
include_directories(${AIDA_INCLUDE_DIRS})

find_package(CLHEP)
include_directories(${CLHEP_INCLUDE_DIRS})

find_package(HepPDT)
include_directories(${HEPPDT_INCLUDE_DIRS})

find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_PATH})

find_package(GSL)
include_directories(${GSL_INCLUDE_DIRS})

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src/Lib )

#---Libraries---------------------------------------------------------------
set( GaudiExamples_srcs AbortEvent/*.cpp
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

GAUDI_COMPONENT_LIBRARY(GaudiExamples ${GaudiExamples_srcs} LIBRARIES GaudiExamplesLib)
GAUDI_LINKER_LIBRARY(GaudiExamplesLib Lib/*.cpp LIBRARIES GaudiGSLLib GaudiUtilsLib ${HEPPDT_LIBRARIES} ${ROOT_Tree_LIBRARY})
GAUDI_PYTHON_MODULE(PyExample PythonModule/*.cpp LIBRARIES ${Boost_PYTHON_LIBRARY} )

#---Executables-------------------------------------------------------------
GAUDI_EXECUTABLE(Allocator Allocator/*.cpp LIBRARIES GaudiExamplesLib GaudiAlgLib GaudiKernel ${ROOT_LIBRARIES}
                 ${ROOT_RIO_LIBRARY} ${ROOT_Hist_LIBRARY} ${CLHEP_LIBRARIES} ${GSL_LIBRARIES})


#---Dictionaries------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiExamples src/POOLIO/dict.h  src/POOLIO/dict.xml  LIBRARIES GaudiExamplesLib)


#---Tests-------------------------------------------------------------------
GAUDI_FRAMEWORK_TEST(Properties options/Properties.opts )
GAUDI_FRAMEWORK_TEST(AlgTools options/AlgTools.opts )

set(QMTests
abort_event
aida2root
aida2rootex
algerraud
algorithm_destructor
algsequencer
algsequencer_pyopts
algtools
algtools2
algtools_pyopts
array_properties
auditors_user_timing
boost_array_properties
bug_30209_namespace_in_sequencer
bug_34121_tool_properties_in_python
bug_35913
bug_38882
bug_41136
colormsg
configurableuser
counterex
counterex_py
countersvcalg
dataondemand
errorlog
event_timeout
ext_props_opts
ext_props_pyopts
extended_properties2
fastcontainers
gaudicommontests
gaudipython_mix
gsltools
histo_ascii_dump
histoex
histoex1
histoex2
histograms
history
histoutilsex
incidentsvc
init_loop_check_off
init_loop_check_on
maps
ntuples
particlepropsvc
properties
qotdalg
randomnumber
selections
signal_handler
statsvcalg
statuscodesvc
stringkey
templatedalg
templatedalg_pyopts
timing
tupleex
tupleex1
tupleex2
tupleex3
tupleex4
tupleex_py )

#----Not supported tests on this platform-----------------------------------------------------------
# datalistener (MonALISA...)
# gaudimt

foreach( test ${QMTests})
  GAUDI_QMTEST_TEST(gaudiexamples.${test} )
endforeach()

#---Groups of QMtests-------------------------------------------------------------------------------
GAUDI_QMTEST_TEST(gaudiexamples.poolio TESTS gaudiexamples.pooliowrite gaudiexamples.poolioread
                                             gaudiexamples.pooliocollwrite gaudiexamples.pooliocollread
                                             gaudiexamples.poolioextcollwrite gaudiexamples.poolioextcollread )
GAUDI_QMTEST_TEST(gaudiexamples.evtcol TESTS gaudiexamples.evtcolwrite gaudiexamples.evtcolread
                                             gaudiexamples.evtcolsex_prepare gaudiexamples.evtcolsex_write  gaudiexamples.evtcolsex_read)
GAUDI_QMTEST_TEST(gaudiexamples.thist  TESTS gaudiexamples.thistwrite gaudiexamples.thistread)
GAUDI_QMTEST_TEST(gaudiexamples.return_codes)
GAUDI_QMTEST_TEST(gaudiexamples.bug)


SET(GaudiExamples_environment STDOPTS=${CMAKE_SOURCE_DIR}/GaudiExamples/options
                              JOBOPTSEARCHPATH+=${CMAKE_SOURCE_DIR}/GaudiExamples/options
                              PYTHONPATH+=${CMAKE_SOURCE_DIR}/GaudiExamples/tests/qmtest
                              PARENT_SCOPE)
