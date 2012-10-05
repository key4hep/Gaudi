How to build and use the Gaudi with CMake
=========================================

Requirements
------------

The CMake configuration of Gaudi is based on the version 2.8.5 or later of
CMake. On lxplus (SLC5 and SLC6) you need to call::

    $ export PATH=/afs/cern.ch/sw/lcg/external/CMake/2.8.6/x86_64-slc5-gcc43-opt/bin

You also need to ensure that the compiler you want to use is configured and
available on the PATH, for example with:

    $ . /afs/cern.ch/sw/lcg/contrib/gcc/4.6.2/$CMTCONFIG/setup.sh

Build
-----

I suggest to use the off-source build, which means that the files required by
the build and the build products are created in a different directory than the
source one.
In this example I'll use `Gaudi.build`, created at the same level as the
directory containing the sources (`Gaudi.src`).

To prepare the build directory, you have to::

    $ toolchain=$PWD/Gaudi.src/cmake/toolchain/heptools-64a.cmake
    $ moddir=$PWD/Gaudi.src/cmake/module
    $ export GaudiProject_DIR=$PWD/Gaudi.src/cmake
    $ mkdir Gaudi.build
    $ cd Gaudi.build
    $ cmake -DCMAKE_TOOLCHAIN_FILE=$toolchain -DCMAKE_MODULE_PATH=$moddir \
      -DBUILD_TESTS=TRUE -G "Eclipse CDT4 - Unix Makefiles" ../Gaudi.src

This will create the required Unix makefiles and the Eclipse project
configuration to build Gaudi.

Other options are available on the command line when you prepare the build
directory the first time or afterwards via the CMake configuration tool
`ccmake`, for example `CMAKE_USE_DISTCC`.

Now you can build the project with a simple (from `Gaudi.build`)::

    $ make

or from Eclipse after you imported the project.

The tests can be run via the command `ctest` or with `make test`.

Install
-------

The build of the project obtained with CMake cannot be used directly by other
projects: it needs to be installed in the source directory (for backward
compatibility with CMT) with the command::

    $ make install

It must be noted that the special file `python.zip` is not automatically
generated, so, if you want it, you have to call, after the installation::

    $ make python.zip

