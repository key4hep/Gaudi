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

if [ "x$ENV_CMAKE_SOURCE_DIR" = "x" ]
then
    echo "Error ENV_CMAKE_SOURCE_DIR not set"
    exit 5
fi

if [ "x$ENV_CMAKE_BINARY_DIR" = "x" ]
then
    echo "Error ENV_CMAKE_BINARY_DIR not set"
    exit 6
fi

exit 0
