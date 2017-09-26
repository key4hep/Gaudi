gaudi_subdir(PartPropSvc)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost COMPONENTS regex)
find_package(HepPDT)

# If HepPDT is not available, give up on this subdirectory:
if( NOT HEPPDT_FOUND )
   return()
endif()

# Hide some HepPDT, Boost compile time warnings
include_directories(SYSTEM ${HEPPDT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_module(PartPropSvc src/*.cpp
                 LINK_LIBRARIES GaudiKernel Boost HepPDT
                 INCLUDE_DIRS Boost HepPDT)

#---Installation------------------------------------------------------------
gaudi_install_joboptions(share/PartPropSvc.py)
file(GLOB PDG_files share/PDGTABLE*)
install(FILES ${PDG_files} DESTINATION share)

#---Environment-------------------------------------------------------------
# The order is important to have the right precedence when running from build.
gaudi_build_env(APPEND DATA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/share)
# Note: ${.} in the environment definition refers to the install prefix.
gaudi_env(APPEND DATA_PATH \${.}/share)
