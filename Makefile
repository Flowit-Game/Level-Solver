all: solver

debug:: main.cpp
	g++ -Wall -g -std=gnu++20 main.cpp -o solver

release:: main.cpp
	g++ -Wall -g -O3 -std=gnu++20 main.cpp -o solver

clean:
	rm solver
