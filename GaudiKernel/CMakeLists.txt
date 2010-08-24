############################################################################
# CMakeLists.txt file for building GaudiKernel
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(Boost)
GAUDI_USE_PACKAGE(CppUnit)
GAUDI_USE_PACKAGE(ROOT)

#---Libraries---------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiKernel Lib/*.cpp Reflex dl ${Boost_LIBRARIES})

#---Executables-------------------------------------------------------------
GAUDI_EXECUTABLE(genconf Util/genconf.cpp GaudiKernel)

#---Tests-------------------------------------------------------------------
GAUDI_TEST(DirSearchPath_test ../tests/src/DirSearchPath_test.cpp GaudiKernel)
GAUDI_TEST(test_SerializeSTL ../tests/src/test_SerializeSTL.cpp GaudiKernel cppunit dl)

#---Dictionaries------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiKernel dict/dictionary.h  dict/dictionary.xml  GaudiKernel)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiKernel)
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()
