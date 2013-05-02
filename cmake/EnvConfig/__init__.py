__author__ = "Marco Clemencic <marco.clemencic@cern.ch>"

__all__ = []
import os

# Prepare the search path for environment XML files
path = ['.']
if 'ENVXMLPATH' in os.environ:
    path.extend(os.environ['ENVXMLPATH'].split(os.pathsep))

import Script
import Control
