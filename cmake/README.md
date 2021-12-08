# How to build and use Gaudi with CMake

## Requirements

The CMake configuration of Gaudi is based on the version 3.15.0 or later of
CMake. On lxplus (SLC6 and CentOS7) you need to call something **like**:

```sh
$ export PATH=/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.18.3/Linux-x86_64/bin:$PATH
```

## Quick Start

Gaudi requires some uncommon external libraries, so the quickest way to build
it is to use the CERN SFT provided LCG views, for example:

```sh
. /cvmfs/sft.cern.ch/lcg/views/LCG_101/x86_64-centos7-gcc11-opt/setup.sh
```

We suggest the use of [Ninja](https://ninja-build.org/) to build, if you wish
to do so, you can use these settings:

```sh
export CMAKE_GENERATOR=Ninja
```

At this point the environment is good enough to use a standard CMake procedure:

```sh
cd Gaudi
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Older versions of CERN SFT LCG views
Prior to LCG 99, the versions of CMake and Ninja in the CERN SFT LCG views were
not good enough and Python was still 2.x, so the recipe to build Gaudi is
slightly different in that case:

```sh
. /cvmfs/sft.cern.ch/lcg/views/LCG_97a/x86_64-centos7-gcc9-opt/setup.sh
export PATH=/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.18.3/Linux-x86_64/bin:$PATH
export PATH=/cvmfs/sft.cern.ch/lcg/contrib/ninja/1.10.0/Linux-x86_64:$PATH
export CMAKE_GENERATOR=Ninja
cd Gaudi
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DGAUDI_USE_PYTHON_MAJOR=$(python -c "import sys; print(sys.version_info[0])")
cmake --build build
```

## Run from the build directory

For testing and debugging there is no need to install, but Gaudi requires some
environment variables to be set, so the build procedure generates a special
script (`build/run`) that can be used to run a command in the correct environment,
for example:

```sh
build/run gaudirun.py --help
```

or

```sh
build/run bash
```

to have a shell with the correct environment.

## Testing

Gaudi comes with a pool of tests (built by default, unless one passes the
option `-DBUILD_TESTING=FALSE` to Gaudi) that can be run with
[`ctest`](https://cmake.org/cmake/help/latest/manual/ctest.1.html):

```sh
cd build
ctest
```

## Resources

* [CMake documentation](http://www.cmake.org/cmake/help/documentation.html)
* `FindTBB.cmake` was taken from <https://bitbucket.org/sergiu/tbb-cmake>
  And should be removed in a near future.
