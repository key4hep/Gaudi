# File: GaudiPython/__init__.py
# Author: Pere Mato (pere.mato@cern.ch)

"""
   GaudiPython main module.
   It makes available a number of APIs and classes to be used by end user scripts

   Usage:
      import GaudiPython
"""

# ensure that we (and the subprocesses) use the C standard localization
import os
if os.environ.get('LC_ALL') != 'C':
    print '# setting LC_ALL to "C"'
    os.environ['LC_ALL'] = 'C'

from Bindings import *
from Pythonizations import *
