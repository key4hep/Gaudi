Platform Specific Sources
=========================

This directory holds some platform specific source files, which are only
picked up selectively for the build by the CMake configuration.

The source files are put into a separate subdirectory to make it possible
to use the `src/Lib/*.cpp` pattern to pick up all of the non platform
specific source files of GaudiKernel for the build.
