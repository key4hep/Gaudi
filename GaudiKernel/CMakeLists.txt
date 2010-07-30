###############################################
# CMakeLists.txt file for building GaudiKernel
# @author Pere Mato, CERN
###############################################

GAUDI_USE_PACKAGE(Boost)
GAUDI_USE_PACKAGE(ROOT)
GAUDI_USE_PACKAGE(AIDA)

#---Linker Library---------------------------------------------------------------------------
GAUDI_LINKER_LIBRARY( GaudiKernel 
                      Lib/*.cpp 
                      ${ROOT_LIBRARIES} ${Boost_LIBRARIES})

#---Dictionary-------------------------------------------------------------------------------
REFLEX_BUILD_DICTIONARY( GaudiKernel 
                         dict/dictionary.h 
                         dict/dictionary.xml
                         GaudiKernel )

#---genconf command--------------------------------------------------------------------------
GAUDI_EXECUTABLE( genconf.exe Util/genconf.cpp GaudiKernel)

#---Installation-----------------------------------------------------------------------------
GAUDI_INSTALL_HEADERS()
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()



