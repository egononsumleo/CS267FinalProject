CXX=g++
#CXXFLAGS=-g -std=c++11 -Wall -Wformat=2 -Wshift-overflow=2 -fsanitize=address -lgmp -lgmpxx -lmpfr
CXXFLAGS= -std=c++11 -lmpfr -fopenmp -O3 
LDFLAGS=-lmpfr -fopenmp

test: test.o
	g++ test.o -o test $(LDFLAGS)
test.o: main.cpp RBS.h mpreal.h makefile
	$(CXX) $(CXXFLAGS) -c main.cpp -o test.o -lmpfr
#
