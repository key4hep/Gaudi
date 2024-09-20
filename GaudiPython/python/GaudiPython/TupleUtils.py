#!/usr/bin/env python3
#####################################################################################
# (c) Copyright 2023-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
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

warnings.warn(
    "GaudiPython: Use 'GaudiAlg.TupleUtils' module instead of deprecated 'GaudiPython.TupleUtils'",
    DeprecationWarning,
    stacklevel=3,
)
del warnings

import sys

import GaudiAlg.TupleUtils

sys.modules[__name__] = GaudiAlg.TupleUtils
