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
# - Simple module to add the pytools directory to the python path
#
# PYTOOLS_FOUND
# PYTOOLS_PYTHON_PATH
# PYTOOLS_BINARY_PATH

set(PYTOOLS_FOUND 1)
set(PYTOOLS_PYTHON_PATH ${pytools_home}/lib/python${Python_config_version_twodigit}/site-packages
    CACHE PATH "Path to the pytools LCG package (Python modules)")

set(PYTOOLS_BINARY_PATH ${pytools_home}/bin
    CACHE PATH "Path to the pytools LCG package (scripts)")

mark_as_advanced(PYTOOLS_FOUND PYTOOLS_PYTHON_PATH PYTOOLS_BINARY_PATH)
