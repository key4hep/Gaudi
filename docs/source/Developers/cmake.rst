
Gaudi CMake Documentation
=========================

This documentation is the updated version of
https://twiki.cern.ch/twiki/bin/view/LHCb/GaudiCMakeConfiguration and is
based on `CMake
3.15 <https://cmake.org/cmake/help/latest/release/3.15.html>`__. It was
written after the modernization of the configuration of Gaudi.

Introduction
------------

`CMake <http://www.cmake.org>`__ is an Open Source, cross-platform
configuration and build tool used in several projects around the world.

Among its advantages we can count the support that comes from a large
user base and the CMake configuration language that doubles as a
portable and powerful scripting language.

The main drawbacks of CMake are the syntax of its language (not very
nice looking) and the lack of support for runtime environment
definition, but, thanks to the power of the language, we can overcome
the drawbacks.

Here I'll describe how to write the CMake configuration for projects and
packages. See also the `CMake
Documentation <https://cmake.org/cmake/help/latest/>`__.

Understanding and Writing CMake Code
------------------------------------

A minimal introduction on the CMake syntax and conventions is mandatory
for people who never used it. info The syntax highlighting in the code
blocks is not correct because TWiki.SyntaxHighlightingPlugin does not
understand the CMake language.

In CMake every statement is a function call, written as an identifier
followed by optional spaces and the arguments to the function as a
space-separated list enclosed in parentheses. The arguments to a
function can span over several lines, and double quotes (``"``) can be
used to pass arguments containing spaces and new-lines. Examples of
functions: 

.. code-block:: cmake

  message("hello world")

.. code-block:: cmake

  if(1 GREATER 0) 
    message("this is true") 
  else() 
    message("this is false")
  endif() 

One can add comments to the code using ``#`` at the beginning of the
comment text (spaces preceding ``#`` are ignored), like, e.g., in Unix
shells and Python:

.. code-block:: cmake

  # I'm a comment
  if(TRUE)
    # this is always printed 
    message("it's true")
  else() # this is never printed
    message("it's false")
  endif()

The CMake language supports variables, which are set with the function
``set`` and dereferenced with ``${...}``, e.g.: 

.. code-block:: cmake


  set(MyMessage "hello world")
  message(${MyMessage}) 

Dereferencing of variables can be nested and works also in between
double quotes: 

.. code-block:: cmake

  set(name MyName)
  set(${name}_msg "This is${name}")
  message(${${name}_msg})

There is something special in the way CMake functions are invoked. CMake
functions and macros (similar to functions) accept variable number of
arguments and the only separators between arguments are spaces (tabs and
new-lines too), so it might not be obvious how to pass optional
arguments (like it's done in Python with named arguments). The solution
found by CMake developers is to use *keyword-separated* lists of
arguments. For example, we can imagine a function that requires a
mandatory argument *name* and two optional lists of files, C sources and
C++ sources. In CMake you could find it invoked like this:


.. code-block:: cmake
  
  make_a_library(MyLibrary C_SOURCES file1.c file2.c file3.c)

  make_a_library(MyLibrary CXX_SOURCES file1.cpp file2.cpp file3.cpp)

  make_a_library(MyLibrary C_SOURCES file1.c file2.c file3.c CXX_SOURCES file1.cpp file2.cpp file3.cpp)

Warning: Function names are case-insensitive, but variable names are case
sensitive as well as string comparison.

CMake Configuration of a Gaudi-based Project
--------------------------------------------

Several steps must be performed to build a Gaudi based project:

-  Set up the build environment (if not already set up)
-  Configure the project with CMake (generate *Makefile* or
   *build.ninja*)
-  Compile the source files and link the binaries
-  Test the previously build binaries (optional but recommended)

Set up the build environment
----------------------------

To set up the build environment, some environment variables need to be set. 

* **BINARY_TAG**: the variable that describe the platform 
* **CMAKE_PREFIX_PATH**: is a path-like variable that must contain the list the list of path to: 

  * the compiler e.g. *g++*, *clang++* (the compiler may be a wrapper) 
  * the build system e.g. *make*, *ninja* 
  * all third-party dependencies e.g. Boost, ROOT 

* **PATH** (optional): may contain the paths to the compiler and the build system

There are several ways to set these variables: 
* use *export* several time (or run a shell script that will do so) 
* source a view (a shell script that sets the aforementioned variables to a directory of symlinks) 
* specify a toolchain to the configuration

With *export*
~~~~~~~~~~~~~

.. code-block:: bash

  export BINARY_TAG="x86_64-centos7-gcc8-opt"
  export CMAKE_PREFIX_PATH="/path/to/g++:/path/to/boost:/path/to/ROOT:..."

It is also possible to list the call to *export* in a shell script to be
able to source it later on.

With a view
~~~~~~~~~~~


.. code-block:: bash

  source/cvmfs/sft.cern.ch/lcg/views/LCG_96/x86_64-centos7-gcc8-opt/setup.sh
  # the views may lack some stuff
  export CMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH:/cvmfs/sft.cern.ch/lcg/releases/LCG_95/vectorclass/1.30/x86_64-centos7-gcc8-opt:/cvmfs/projects.cern.ch/intelsw/psxe/linux/x86_64/2019/vtune_amplifier"

Warning: Sourcing a view is not the same as sourcing a shell script that
uses *export*. A view is a directory of symbolic links and a *setup.sh*
script.

With a toolchain
~~~~~~~~~~~~~~~~


.. code-block:: bash

  # Either use 
  -D CMAKE_TOOLCHAIN_FILE=/path/to/a/toolchain 
  # at configure time when calling cmake 
  # or 
  ln -s /path/to/a/toolchain 
  toolchain.cmake 
  # right away

Configuration of the project
----------------------------

The configuration requires at least CMake 3.15. CMake 3.15.0 was
released on 2019-07-17. 


.. code-block:: bash

  # Check CMake version 
  cmake --version 
  # if version < 3.15
  export PATH="/cvmfs/lhcb.cern.ch/lib/contrib/CMake/3.15.2/Linux-x86_64/bin:$PATH"

The configuration is the step when CMake is called and produces the
files for the build system (e.g. *make*, *ninja*).

Two directories must be specified: 
* the source tree: contains the sources 
* the build tree: will contain the outputs of the build

.. code-block:: bash

  cmake -S . -B build.$BINARY_TAG 
  # options can be passed at configure time
  cmake -S . -B build.$BINARY_TAG -G Ninja
  # set the build system
  cmake -S . -B build.$BINARY_TAG -D GAUDI_USE_AIDA=OFF
  # enable/disable a third party dependency
  cmake -S . -B build.$BINARY_TAG -D CMAKE_BUILD_TYPE=Developer
  # select a build type (a set of compile and link options)
  cmake -S . -B build.$BINARY_TAG -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake
  # specify a toolchain
  # several options may be specified with: -D ... -D ... -D ...
  # ccmake and cmake-gui can be used
  cmake -LH build.$BINARY_TAG
  # to see all the options and their help messages

Compilation of the project
--------------------------



Once the project is configured, several files are already in the build
tree. They will be used to compile the project.


.. code-block:: bash

  cd build.$BINARY_TAG
  make # or ninja or an IDE 
  # or directly with CMake
  cmake --build build.$BINARY_TAG

Test the binaries
-----------------

Good developers test their code. To run the tests:

.. code-block:: bash

  cd build.$BINARY_TAG 
  ctest  -j <nproc> --output-on-failure --schedule-random

The wrapper
-----------

Doing all these steps each time may be tedious so there is a *Makefile*
file at the top level of Gaudi-based projects to wrap the call to these
commands.


.. code-block:: bash

  # configure + compile
  make
  # run the tests
  make test

The wrapper uses a toolchain if a file called *toolchain.cmake* exists
in the current directory. It might be useful to have a shell function to
easily switch from one toolchain to another.

.. code-block:: bash

  function switch_platform {
    export BINARY_TAG=$1
    rm -f toolchain.cmake
    ln -s /cvmfs/.../toolchains/$1.cmake toolchain.cmake # !! Use the right path on CVMFS
  }

Use the software
----------------

In order to use the previously built software, it is mandatory to use
the **runtime environment** (it may differ from the build environment).
The runtime environment is generated by the configuration at configure
time in the build tree in a script called *run*.


.. code-block:: bash

  cd build.$BINARY_TAG
  # ./run <program> <args...>
  ./run listcomponents -h
  ./run gaudirun.py

Modify the configuration
------------------------

At the top level directory of a project and in every sub-project
(package) there must be one file: ``CMakeLists.txt``

The file at the top level directory describes the build of the whole
project: 
* may contain a licence block 
* contains documentation (how to configure it, available options) 
* describes the project (name, version) 
* fetches the dependencies 
* sets options for the build (C++ standard) 
* list all the sub-projects (packages in sub-directories) 
* handles the installation The files in the sub-projects directories: 
* describes the binaries that will be compiled (libraries, modules, executable, ROOT dictionaries) 
* register tests for these binaries 
* handles the installation of their python packages and scripts

Configuration of sub-projects
-----------------------------



Look and feel of typical sub-project ``CMakeLists.txt``: 

.. code-block:: cmake

  # {licence block if needed}
  # {SubdirName} subdirectory
  # Build the library
  gaudi_add_library(SubdirNameLib SOURCES src/Lib/Counter.cpp src/Lib/Event.cpp LINK PUBLIC GaudiKernel)
  # Build the plugin gaudi_add_module(SubdirName
    SOURCES src/AbortEvent/AbortEventAlg.cpp src/AlgSequencer/HelloWorld.cpp
    LINK GaudiKernel GaudiExamplesLib ROOT::Tree Rangev3::rangev3)
  if(GAUDI_USE_AIDA) # optional dependency
    target_sources(SubdirName
      PRIVATE src/EvtColsEx/EvtColAlg.cpp src/Histograms/Aida2Root.cpp)
    target_link_libraries(SubdirName PRIVATE AIDA::aida)
  endif()
  # Build the executable
  gaudi_add_executable(Allocator
    SOURCES src/Allocator/Allocator.cpp src/Allocator/MyClass1.cpp
    LINK SubdirNameLib GaudiKernel)
  # Generate GaudiExamples_user.confdb
  gaudi_generate_confuserdb()
  
  # Tests
  gaudi_add_tests(QMTest)
  gaudi_add_tests(nosetests)
  
  # Compiled python module
  gaudi_add_python_module(PyExample
    SOURCES src/PythonModule/Functions.cpp src/PythonModule/PyExample.cpp
    LINK Python::Python Boost::python)
  
  # ROOT dictionaries
  gaudi_add_dictionary(SubdirNameDict
    HEADERFILES src/IO/dict.h
    SELECTION src/IO/dict.xml
    LINK SubdirNameLib)
  
  # Install python modules
  gaudi_install(PYTHON) 
  # Install other scripts
  gaudi_install(SCRIPTS)

Why is the explicit list of sources mandatory?

Even tough CMake is
able to use glob patterns with ``file(GLOB...)``, those glob patterns
are expanded at configure time and their results hardcoded in
``makefile`` or ``build.ninja`` or whichever file used by IDEs. This
means that if a new file that matches the pattern is added, there is no
way for the build system (make, ninja...) to notice it. The first
solution is to reconfigure the project each time a new file is added to
update the hardcoded list of sources. (This can be done either by
rerunning the configuration command or by touching a
``CMakeLists.txt``.) The other solution would be to forward the glob
pattern to the build system. CMake offers a way to do so:
``file(GLOB ... CONFIGURE_DEPENDS)`` but for the time being, only
Makefiles generators and Ninja are supported, meaning that people using
IDEs would still have to reconfigure the project themselves.

Adding a new sub-project to the project
---------------------------------------

If a new sub-project is added to a project, its directory must be added
to the list of sub-projects in the top-level ``CMakeLists.txt``
alongside with the other sub-projects with:

.. code-block:: cmake

  add_subdirectory(SubdirName)

The directory of the sub-project must also contain a ``CMakeLists.txt``
that looks like the one above.

Adding a new third-party dependency
-----------------------------------

First, look in the documentation of the dependency. If it uses CMake, it
may provide a **config file** (a file named
``{DependencyName}Config.cmake``). Otherwise, it is necessary to write a
**find module file** (a file named ``Find{DependencyName}.cmake``) that
will do the look up of this dependency (find the include directory, find
all the libraries, find any other useful files provided by the
dependency and create some IMPORTED targets). (Have a look at the other
find module files in the project to get an idea of what it should look
like. They should be in ``cmake/modules``.)

Then, add the look up of the dependency in the file
``cmake/{ProjectName}Dependencies.cmake`` (replace <DependencyName> with
the name of the dependency and <minVersion> with the minimal required
version). 

.. code-block:: cmake

  # For mandatory dependencies
  find_package(<DependencyName> <minVersion> ${__quiet})
  set_package_properties(<DependencyName> PROPERTIES TYPE REQUIRED)
  # For optional dependencies
  if(GAUDI_USE_<DEPENDENCY_NAME>)
    find_package(<DependencyName> <minVersion> ${__quiet})
    if(CMAKE_FIND_PACKAGE_NAME)
      # if the lookup is perform from ProjectConfig.cmake
      # then, all optional dependencies become required
      set_package_properties(<DependencyName> PROPERTIES TYPE REQUIRED)
    else()
      set_package_properties(<DependencyName> PROPERTIES TYPE RECOMMENDED)
    endif()
  endif()
  # and add the option GAUDI_USE_<DEPENDENCY_NAME> in the top level CMakeLists.txt
  #Finally, it is usable.
  gaudi_add_<type>(... LINK ...  Dependency::Target ...)

Removing an old third-party dependency
--------------------------------------

If a dependency is no longer used (nothing defined by it is used
anywhere), it is no use keeping to look for it. In this case, remove the
chunk of code that looks for it in
``cmake/{ProjectName}Dependencies.cmake`` (see Adding a new third-party
dependency).

Then remove the **find module file** ``Find{DependencyName}.cmake`` in
cmake/modules if it exists.

Gaudi CMake functions to help the configuration
-----------------------------------------------

All the ``gaudi_*()`` functions are defined by Gaudi. Their content and
documentation can be found
`here <https://gitlab.cern.ch/gaudi/Gaudi/blob/master/cmake/GaudiToolbox.cmake>`__
in ``GaudiToolbox.cmake``.

List of defined functions: 

* ``gaudi_add_library()`` 
* ``gaudi_add_header_only_library()`` 
* ``gaudi_add_module()`` 
* ``gaudi_add_python_module()`` 
* ``gaudi_add_executable()`` 
* ``gaudi_add_tests()`` 
* ``gaudi_add_dictionary()`` 
* ``gaudi_install()`` 
* ``gaudi_generate_confuserdb()`` 
* ``gaudi_check_python_module()`` 
* ``gaudi_generate_version_header_file()``

Building a stack of project at once
-----------------------------------

With the configuration it is possible to build a stack of project at
once. CMake may configure all the projects of the stack in one go,
enabling the compilation to be done in parallel for all the projects.

Example of a stack: Gaudi, LHCb, Lbcom, Rec, Brunel


.. code-block:: bash

  mkdir workspace
  cd workspace
  git clone project_url
  # of all the projects of the desired stack
  cat <<EOF > CMakeLists.txt
    cmake_minimum_required(VERSION 3.15)

    project(LHCbFullStack LANGUAGES CXX DESCRIPTION "LHCb full stack")

    enable_testing()

    add_subdirectory(Gaudi)
    add_subdirectory(LHCb)
    # add_subdirectory() ... all the other projects of the stack
    EOF 

Using GaudiObjDesc (LHCb-specific)
----------------------------------

Warning: GaudiObjDesc has not been modernized because it should be
removed soon. Do not use GaudiObjDesc in future project. However, if you
need to maintain a package that uses GaudiObjDesc have a look at the old
documentation
`here <https://twiki.cern.ch/twiki/bin/view/LHCb/GaudiCMakeConfiguration#Using_GaudiObjDesc_LHCb_specific>`__

Pro tip: do not document too much GaudiObjDesc so that people begrudge to
use GaudiObjDesc.

Update old Gaudi Projects to the new CMake Configuration in v35
---------------------------------------------------------------

When updating an existing Gaudi Project to use v35, the following changes need to be made:


* Add a ``project(PROJECTNAME)`` line in the top level CMakeLists.txt
* change ``find_package(GaudiProject)`` to ``find_package(Gaudi)`` in the top level CMakeLists.txt
* remove any calls to ``gaudi_project(...)``
* explicitly add all subdirectories that contain a CMakeLists.txt with ``add_subdirectory(...)`` 
* update any calls to ```gaudi_add_library``` to use the new signature
* update any calls to ```gaudi_add_module``` to use the new signature
* change ``gaudi_install_*()`` to ``gaudi_install(*)``
* Export all libraries and modules as targets with ``install(TARGETS ...  EXPORT ...)``
* add a call to ``gaudi_install(CMAKE)`` int the top level CMakeLists.txt
* add a CMake config file ``cmake/PROJECTNAMEConfig.cmake`` which should include the targets file

For a concrete example, see the changes needed to update key4hep framework core library: https://github.com/key4hep/k4FWCore/pull/19/files



