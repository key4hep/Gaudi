#####################################################################################
# (c) Copyright 2020 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# This file is to be called with the script run generated when building dummyProject.
# It tests the content of the environnement variables.

if [[ $PATH != *"dummyProjectBinaryDir"* ]]
then
    echo "Error built executable are not in PATH."
    exit 1
fi

if [[ $LD_LIBRARY_PATH != *"dummyProjectBinaryDir"* ]]
then
    echo "Error plugins are not in LD_LIBRARY_PATH."
    exit 2
fi

if [[ $PYTHONPATH != *"cmake/tests/dummyProject/python"* ]]
then
    echo "Error python packages are not in PYTHONPATH."
    exit 3
fi

if [[ $PATH != *"cmake/tests/dummyProject/scripts"* ]]
then
    echo "Error scripts are not in PATH."
    exit 4
fi

if [ "x$ENV_PROJECT_SOURCE_DIR" = "x" ]
then
    echo "Error ENV_PROJECT_SOURCE_DIR not set"
    exit 5
fi

if [ "x$ENV_PROJECT_BINARY_DIR" = "x" ]
then
    echo "Error ENV_PROJECT_BINARY_DIR not set"
    exit 6
fi

exit 0
