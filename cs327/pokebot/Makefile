CXX = g++
CXXFLAGS = -Wall -Werror -ggdb -funroll-loops
LIBS = -lcurl -ljson-c -lncurses

pokebot: pokebot.o
	$(CXX) -g -o $@ $< $(LIBS)

pokebot.o: pokebot.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f pokebot.o pokebot