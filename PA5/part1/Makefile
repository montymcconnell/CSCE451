CXX = g++
CXXFLAGS = -std=c++11 -I.


.PHONY: all
all: part1

part1: part1.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

part1.o: part1.cpp

.PHONY: clean
clean:
	rm -rf part1 *.o
