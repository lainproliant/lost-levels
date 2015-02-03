CXX=g++
CXXFLAGS=-g -rdynamic --std=c++11 -DLAIN_ENABLE_STACKTRACE -DLAIN_STACKTRACE_IN_DESCRIPTION -I../toolbox/include -I../include
LDFLAGS=-lSDL2 -lSDL2_image
LDLIBS=

all: build-all-demos

build-all-demos: $(patsubst %.cpp, %.demo, $(wildcard *.cpp))

%.demo: %.cpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -f *.demo

