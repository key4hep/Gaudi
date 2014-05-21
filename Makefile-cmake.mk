################################################################################
#
# Generic Makefile to simplify the use of CMake projects
# ------------------------------------------------------ 
#
# This simple Makefile is meant to provide a simplified entry point for the
# configuration and build of CMake-based projects that use a default toolchain
# (as it is the case for Gaudi-based projects).
#
# Only a few targets are actually provided: all the main targets are directly
# delegated to the CMake Makefile.
#
# Main targets:
#
#     all
#         (default) build everything
#  
#     test [*]_
#         run the declared tests
#
#     install
#         populate the InstallArea with the products of the build
#
#     clean
#         remove build products from the build directory
#
#     purge [*]_
#         deep clean of the build, including InstallArea
#         (requires re-configuration)
#
#     help
#         print the list of available targets
#
#     configure [*]_
#         alias to CMake 'rebuild_cache' target
#
#     tests [*]_
#         backward-compatibility target for the CMT generic Makefile
#
# :Author: Marco Clemencic
#
# .. [*] Targets defined by this Makefile. 
#
################################################################################

# settings
CMAKE := cmake
CTEST := ctest
NINJA := $(shell which ninja 2> /dev/null)

ifneq ($(wildcard $(CURDIR)/toolchain.cmake),)
  override CMAKEFLAGS += -DCMAKE_TOOLCHAIN_FILE=$(CURDIR)/toolchain.cmake
endif

ifndef BINARY_TAG
  ifdef CMAKECONFIG
    BINARY_TAG := ${CMAKECONFIG}
  else 
    ifdef CMTCONFIG
      BINARY_TAG := ${CMTCONFIG}
    endif
  endif
endif

BUILDDIR := $(CURDIR)/build.$(BINARY_TAG)

ifneq ($(NINJA),)
  ifeq ($(USE_MAKE),)
    ifeq ($(shell grep "FORTRAN\|NO_NINJA" CMakeLists.txt),)
      USE_NINJA := 1
    endif
  endif
endif

# build tool
ifneq ($(USE_NINJA),)
  # enable Ninja
  override CMAKEFLAGS += -GNinja
  ifneq ($(VERBOSE),)
    NINJAFLAGS := -v $(NINJAFLAGS)
  endif
  BUILD_CMD := cd build.$(BINARY_TAG) && ninja $(NINJAFLAGS)
  BUILD_CONF_FILE := build.ninja
else
  BUILD_CMD := cd build.$(BINARY_TAG) && $(MAKE)
  BUILD_CONF_FILE := Makefile
endif

# default target
all:

# deep clean
purge:
	$(RM) -r $(BUILDDIR) $(CURDIR)/InstallArea/$(BINARY_TAG)
	find $(CURDIR) -name "*.pyc" -exec $(RM) -v \{} \;

# delegate any target to the build directory (except 'purge')
ifneq ($(MAKECMDGOALS),purge)
%: $(BUILDDIR)/$(BUILD_CONF_FILE) FORCE
	+$(BUILD_CMD) $*
endif

# aliases
.PHONY: configure tests FORCE
ifneq ($(wildcard $(BUILDDIR)/$(BUILD_CONF_FILE)),)
configure: rebuild_cache
else
configure: $(BUILDDIR)/$(BUILD_CONF_FILE)
endif
	@ # do not delegate further

# This wrapping around the test target is used to ensure the generation of
# the XML output from ctest. 
test:
	cd build.$(BINARY_TAG) && $(CTEST) -T test $(ARGS)

tests: all
	-cd build.$(BINARY_TAG) && $(CTEST) -T test $(ARGS)
	$(BUILD_CMD) QMTestSummary

# ensure that the target are always passed to the CMake Makefile
FORCE:
	@ # dummy target

# Makefiles are used as implicit targets in make, but we should not consider
# them for delegation.
$(MAKEFILE_LIST):
	@ # do not delegate further

# trigger CMake configuration
$(BUILDDIR)/$(BUILD_CONF_FILE):
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && $(CMAKE) $(CMAKEFLAGS) $(CURDIR)
