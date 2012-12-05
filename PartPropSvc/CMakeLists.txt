gaudi_subdir(PartPropSvc v4r6)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost COMPONENTS regex)
find_package(ROOT COMPONENTS Reflex)
find_package(HepPDT)

#---Libraries---------------------------------------------------------------
gaudi_add_module(PartPropSvc src/*.cpp
                 LINK_LIBRARIES GaudiKernel Boost ROOT HepPDT
                 INCLUDE_DIRS Boost ROOT HepPDT)

#---Installation------------------------------------------------------------
gaudi_install_joboptions(share/PartPropSvc.py)
install(FILES share/PDGTABLE share/PDGTABLE.MeV DESTINATION share)

#---Environment-------------------------------------------------------------
# The order is important to have the right precedence when running from build.
gaudi_build_env(APPEND DATA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/share)
# Note: ${.} in the environment definition refers to the install prefix.
gaudi_env(APPEND DATA_PATH \${.}/share)
