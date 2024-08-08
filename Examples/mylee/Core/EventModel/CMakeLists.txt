#####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

# `gaudi_add_library` is a helper function that wraps standard CMake `add_library`
# trying to simplify the most common use case. See also `gaudi_add_header_only_library`.
gaudi_add_library(mylee_em
    # list of translation units that constitute the library
    SOURCES
        src/mc/event.cpp
    # libraries needed to build this library
    LINK
        # public dependencies are those exposed through public headers (in `include`)
        PUBLIC
            Gaudi::GaudiKernel
            Gaudi::GaudiPartProp
        # private dependencies are the libraries used only inside private code (cpp or internal headers)
        PRIVATE
            Rangev3::rangev3
)

# `gaudi_add_dictionary` is used to create a ROOT dictionary plugin library
# that instructs ROOT about the layout of event model classes we want to
# persistify.
gaudi_add_dictionary(mylee_em_dict
    # the header file provided should include a number of public headers
    # that declare the classes we need to persistify
    HEADERFILES src/dict.h
    # not all classes from the header are included in the dictionary, so
    # we pass a list of classes to use by means of a selection XML file
    SELECTION src/dict.xml
    # private dependency that make sure symbols used by the included headers
    # are correctly resolved
    LINK mylee_em
)
