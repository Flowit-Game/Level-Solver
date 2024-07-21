all: solver

solver:: main.cpp
	g++ -Wall -g -std=gnu++20 main.cpp -o solver

clean:
	rm solver
