projs = A B C D
PWD := $(shell pwd)
# Default project installation path is the <build_dir>/InstallArea
# These needed to be added to CMAKE_PREFIX_PATH so that cmake can find them

empty:=
space:=$(empty) $(empty)
CMAKE_PREFIX_PATH_INSTALL_AREA+=$(subst $(space),:,$(foreach dir,$(projs),${PWD}/${dir}/InstallArea))
CMAKE_PREFIX_PATH:=${CMAKE_PREFIX_PATH_INSTALL_AREA}:$(PWD)/../../..

all: $(patsubst %,build-%,$(projs))

B/build: build-A
C/build: build-A
D/build: build-B build-C

%/build:
	mkdir -p $@
#	cd $@ ; cmake -DCMAKE_TOOLCHAIN_FILE=$(PWD)/toolchain.cmake ..
	echo "CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
#	cd $@ ; cmake -DCMAKE_INSTALL_PREFIX=${PWD}/InstallArea  ..
	cd $@ ; CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} cmake   ..

build-%: %/build
	cd $< && CMAKE_PREFIX_PATH=$(CMAKE_PREFIX_PATH) make install

clean: $(patsubst %,clean-%,$(projs))

clean-%:
	$(RM) -r $*/build $*/InstallArea 
