CXX=g++
CXXFLAGS=-g --std=c++11 -I../toolbox/include -I../include
LDFLAGS=-lSDL2
LDLIBS=

all: unit-test clean

unit-test: build-all-tests
	./run-tests.py

build-all-tests: $(patsubst %.cpp, %.test, $(wildcard *.cpp))

%.test: %.cpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -f *.test *.output

