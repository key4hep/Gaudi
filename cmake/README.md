# How to build and use Gaudi with CMake

## Requirements

The CMake configuration of Gaudi is based on the version 3.15.0 or later of
CMake. On lxplus (SLC6 and CentOS7) you need to call something **like**:

```sh
$ export PATH=/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.15.0/Linux-x86_64/bin:$PATH
```

## Quick Start

To quickly get started, you can use the `Makefile`, which
will take care of the main details (except the value of `BINARY_TAG`):

```sh
$ make -j 8
$ make test
```

The main targets are:

* `configure`

    just run CMake to generate the build directory (or reconfigure)

* `all` (default)

    build everything (implies `configure`)

* `test`

    run the tests (and generate HTML reports), note that it does not
    imply the build and does not require installation

* `install`

    populate the `InstallArea` directory, *required for deployment*

More info can be found in the top-level CMakeLists.txt of Gaudi.

## Run from the build directory

For testing and debugging (as already mentioned) there is no need to install.

To run an application using the build directory, you can use the script
`run` in the build directory, for example like this:

```sh
$ cd build.$BINARY_TAG
$ ./run gaudirun.py --help
$ ./run bash
```

When using `Makefile` the above lines should be changed in

```sh
$ build.$BINARY_TAG/run gaudirun.py --help
$ build.$BINARY_TAG/run bash
```

## Resources

* [CMake documentation](http://www.cmake.org/cmake/help/documentation.html)
* `FindTBB.cmake` was taken from <https://bitbucket.org/sergiu/tbb-cmake>
  And should be removed in a near futur.
