CXX=clang++
CXXFLAGS=-g -rdynamic --std=c++14 -DLAIN_ENABLE_STACKTRACE -DLAIN_STACKTRACE_IN_DESCRIPTION -I./toolbox/include -I./include
LDFLAGS=-lSDL2 -lSDL2_image
WX_CXXFLAGS=`wx-config --cxxflags`
WX_LDFLAGS=`wx-config --libs`
LDLIBS=
RC_DIR=./resources
OUTPUT=./build

all: build-demos build-editor

# A workaround for newlines in foreach loops.
define \n


endef

#
# build(): Perform a build for a target with the given CXXFLAGS.
# @param $1 CXXARGS: The CXXARGS to be passed to the compiler.
# @param $2 SRC: The source file to be compiled.
# @param $3 OBJ: The object file to output.
#
define build
$(CXX) -c $1 $2 -o $3
endef

#
# build-all(): Build all source files for the given project
# 		to a subdirectory of $(OUTPUT)/obj/$(PROJ)
#
# @param $1 CXXARGS: The CXXFLAGS to be passed to the compiler.
# @param $2 PROJ: The directory containing project source files to compile.
#
define build-all
mkdir -p $(OUTPUT)/obj/$2
$(foreach source,$(wildcard $2/*.cpp),$(call build,$1 -I$2 -I$2/include ,$(source),$(OUTPUT)/obj/$(patsubst %.cpp,%.o,$(source)))${\n})
endef

#
# link-all(): Link all of the object files in the given project
# 		into an executable in $(OUTPUT).
#
# @param $1 CXXARGS: The CXXFLAGS to be passed to the compiler.
# @param $2 PROJ: The project whos source files should be linked.
#
define link-all
$(CXX) $1 $(OUTPUT)/obj/$2/*.o -o $(OUTPUT)/$2
endef

#
# link-each(): Link each of the object files in the given project
# 		into different executables in $(OUTPUT).
#
# @param $1 CXXARGS: The CXXFLAGS to be passed to the compiler.
# @param $2 PROJ: The project whos source files should be linked.
#
define link-each
mkdir -p $(OUTPUT)/$2
$(foreach source,$(wildcard $2/*.cpp),$(CXX) $1 $(OUTPUT)/obj/$(patsubst %.cpp,%.o,$(source)) -o $(OUTPUT)/$(patsubst %.cpp,%,$(source))${\n})
endef

build-demos:
	$(call build-all,$(CXXFLAGS),demo)
	$(call link-each,$(LDFLAGS),demo)
	$(foreach source,$(wildcard demo/*.cpp),cp -r $(patsubst %.cpp,%,$(source))-rc $(OUTPUT)/demo/${\n})

build-editor:
	$(call build-all,$(CXXFLAGS) $(WX_CXXFLAGS),editor)
	$(call link-all,$(WX_LDFLAGS),editor)

clean:
	rm -r $(OUTPUT)

