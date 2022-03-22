CXX = g++
CXXFLAGS = -Wall -g -O3 -std=c++11 -pedantic -pthread

.PHONY: all
all: part1

part1: part1.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

part1.o: part1.cpp

.PHONY: clean
clean:
	rm -rf part1 *.o