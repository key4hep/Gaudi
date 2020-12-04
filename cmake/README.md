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
. /cvmfs/sft.cern.ch/lcg/views/LCG_97a/x86_64-centos7-gcc9-opt/setup.sh
```

Note that the version of CMake in LCG_97a view is too old, so one had to add:

```sh
export PATH=/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.18.3/Linux-x86_64/bin:$PATH
```

We suggest the use of [Ninja](https://ninja-build.org/) to build, if you wish
to do so, you can use these settings:

```sh
export PATH=/cvmfs/sft.cern.ch/lcg/contrib/ninja/1.10.0/Linux-x86_64:$PATH
export CMAKE_GENERATOR=Ninja
```

At this point the environment is good enough to use a standard CMake procedure:

```sh
cd Gaudi
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DGAUDI_USE_PYTHON_MAJOR=$(python -c "import sys; print(sys.version_info[0])")
```

The special option `-DGAUDI_USE_PYTHON_MAJOR` is needed with LCG versions less
than 99, where the default version of Python is 2, while Gaudi looks for 3 by
default.

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
