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
# deprecated module
import warnings

from GaudiKernel.GaudiHandles import *

warnings.warn(
    "GaudiPython: Use 'GaudiKernel.GaudiHandles' module instead of deprecated 'GaudiPython.GaudiHandles'",
    DeprecationWarning,
    stacklevel=3,
)
del warnings
