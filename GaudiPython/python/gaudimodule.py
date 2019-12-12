#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
""" Gaudi Python module interface.
    This module provides the basic bindings of the main Gaudi
    components to Python. It is itself based on the ROOT cppyy
    Python extension module.
"""
from GaudiPython import *
deprecation("Use 'GaudiPython' module instead of deprecated 'gaudimodule'")
