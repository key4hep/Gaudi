# deprecated module
from GaudiKernel.GaudiHandles import *

import warnings
warnings.warn("GaudiPython: Use 'GaudiKernel.GaudiHandles' module instead of deprecated 'GaudiPython.GaudiHandles'",
              DeprecationWarning, stacklevel=3)
del warnings
