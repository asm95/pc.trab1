PROG_NAME=cptb
CXX=g++
CLIBS=-lsfml-graphics -lsfml-window -lsfml-system
CFLAGS=--std=c++11

all: $(PROG_NAME)

$(PROG_NAME): main.o
	$(CXX) main.o -o $(PROG_NAME) $(CFLAGS) $(CLIBS)

main.o: main.cpp
	$(CXX) -c main.cpp

clear:
	rm -f *.o $(PROG_NAME)