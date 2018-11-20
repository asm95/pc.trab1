PROG_NAME=cptb
CXX=g++
CLIBS=-lsfml-graphics -lsfml-window -lsfml-system -lpthread
CFLAGS=--std=c++11
LDFLAGS=-g

all: $(PROG_NAME)

$(PROG_NAME): main.o
	$(CXX) $(LDFLAGS) main.o -o $(PROG_NAME) $(CLIBS)

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

clear:
	rm -f *.o $(PROG_NAME)

pack:
	chmod +x pack.sh
	./pack.sh 'trab1'