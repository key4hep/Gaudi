Building and Installing Gaudi
=============================

Introduction
------------

Gaudi uses the CMake_ cross-platform build-generator tool as a build system.
CMake does not build the project, it generates the files needed by your build tool (GNU make, Ninja, Visual Studio, etc.) for building Gaudi.
Before attempting the build you need to have installed a number of mandatory prerequisites external packages on which Gaudi depends. Check the pre-requisites_ and `supported platforms`_ for the list of packages that will needed to install and setup before you starting the build. If your platform is one of the common platforms used by the HEP coummunity very likely all the required packages will be available in CVMFS_, alternatively your can install them locally, for example using the lcgcmake_ packaging tool, or one of the alternative tools.

Using the externals from CVMFS
------------------------------
If your system has CVMFS_ installed, the simplest is to setup a build environment using the LCG views. This is done by executing
::

  source /cvmfs/sft.cern.ch/lcg/views/setupViews.sh <lcg-version> <platform-tag>

The list of available LCG versions with their content and availale platforms can be browsed at the lcginfo_ site. You can also use the latest build of the stack of externals using the lcg version ``dev4/latest`` or ``dev3/latest``. For example:
::

  source /cvmfs/sft.cern.ch/lcg/views/setupViews.sh dev4/latest x86_64-centos7-gcc8-opt

Installing the externals locally
--------------------------------
Alternatively you can install all the required external dependencies locally in your system. An easy way to achive this is to use the lcgcmake_ tool. The following are the basic instructions to install all the required packages for Gaudi in a ``<prefix>`` location.

- Install lcgcmake by just cloning the lcgcmake package and ensure that the PATH environment variable contains the the bin directory
  ::

    git clone https://gitlab.cern.ch/sft/lcgcmake.git
    export PATH=$PWD/lcgcmake/bin:$PATH

- Configure the software stack by selecting the compiler
  ::

    mkdir tmp; cd tmp
    lcgcmake configure --compiler=gcc8 --prefix=<prefix>

- Install all the required packages. The target ``GAUDI-externals`` can be used to ease the job
  ::

    lcgcmake install GAUDI-externals

- Setup the environment with the constructed local view
  ::

    source <prefix>/hsf/x86_64-centos7-gcc8-opt/setup.sh

Building Gaudi
--------------
Gaudi builds as any CMake based project. Once the externals packages are installed or accessible it is as simple as running the configuration with the command ``cmake <options>`` on a build area and to execute ``cmake --build``.
::

  git clone https://gitlab.cern.ch/gaudi/Gaudi.git
  mkdir build; cd build
  cmake ../Gaudi -DBoost_NO_BOOST_CMAKE=TRUE
  cmake --build . [ -- -j8]

  # for running all the builtin tests
  ctest -j8

Running some examples
---------------------
The examples can be run using the command ``run`` available in the buiold directory.
::

  ./run gaudirun.py --help
  ./run gaudirun.py Histograms.py
  ./run gaudirun.py FunctionalAlgorithms/ProduceConsume.py

To start a shell with all the running environment start the bash executable
::

  ./run bash

Pre-requisites
--------------
This table lists the external dependencies of Gaudi. The ones marked as mandatory are the only ones essencial for building and running simple tests or examples.

==============  ==========  ===================================================
Package         Version     Mandatory
--------------  ----------  ---------------------------------------------------
ROOT            >= 6.16     yes
Python          >= 2.7      yes
Boost           >= 1.67     yes
TBB                         yes
LibLZMA                     yes
uuid                        yes
VDT                         yes
AIDA                        no, needed for RootHistCnv
XercesC                     no, used for the XML file catalogue
HepPDT                      no
CLHEP                       no
OpenCL                      no
Rangev3                     no, needed with option GAUDI_USE_SYSTEM_RANGES_V3
Doxygen                     no, needed to generate documentation
cppgsl                      no, needed with option GAUDI_USE_SYSTEM_CPP_GSL
zlib                        no, needed for the profiling tools
==============  ==========  ===================================================

Supported platforms
-------------------

- CentOS 7 with gcc 8
- SCL 6
- Ubuntu 18.04
- MacOS 10.14 with Xcode (experimental)

Compilers
---------


.. _CMake: http://www.cmake.org/
.. _CVMFS: https://cvmfs.readthedocs.io/en/stable/
.. _lcgcmake: https://gitlab.cern.ch/sft/lcgcmake/blob/master/README.md
.. _lcginfo: http://lcginfo.cern.ch
