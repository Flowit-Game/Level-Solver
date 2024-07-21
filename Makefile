all: solver

solver:: main.cpp
	g++ -Wall -g -O2 -std=gnu++20 main.cpp -o solver

clean:
	rm solver
