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
# File: GaudiPython/__init__.py
# Author: Pere Mato (pere.mato@cern.ch)
"""
   GaudiPython main module.
   It makes available a number of APIs and classes to be used by end user scripts

   Usage:
      import GaudiPython
"""
from __future__ import absolute_import, print_function

# ensure that we (and the subprocesses) use the C standard localization
import os
if os.environ.get('LC_ALL') != 'C':
    print('# setting LC_ALL to "C"')
    # !!!
    os.environ['LC_ALL'] = 'C'

from .Bindings import *
from .Pythonizations import *
