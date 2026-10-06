all: release

debug:: main.cpp
	g++ -Wall -g -std=gnu++20 -pthread main.cpp -o solver

release:: main.cpp
	g++ -Wall -g -O3 -std=gnu++20 -pthread main.cpp -o solver

clean:
	rm solver
