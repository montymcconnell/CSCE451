CXX = g++
CXXFLAGS = -Wall -g -O3 -std=c++11 -pedantic -pthread

.PHONY: all
all: part2

part2: part2.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

part2.o: part2.cpp

.PHONY: clean
clean:
	rm -rf part2 *.o