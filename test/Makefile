CXX=g++
CXXFLAGS=-g --std=c++11 -I../toolbox/include -I../include
LDFLAGS=-lSDL2 -lSDL2_image
LDLIBS=

all: unit-test

unit-test: build-all-tests
	./run-tests.py

build-all-tests: $(patsubst %.cpp, %.test, $(wildcard *.cpp))

%.test: %.cpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -f *.test *.output

