gaudi_subdir(GaudiAud)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost)

# Hide some Boost compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiAud src/*.cpp LINK_LIBRARIES GaudiKernel)

gaudi_add_test(QMTest QMTEST)
