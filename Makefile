# this will be the binary name generated
PROG_NAME=cptb
# well, compile it using g++
CXX=g++
# you must install sfml source code (see readme)
CLIBS=-lsfml-graphics -lsfml-window -lsfml-system -lpthread
# should be compiled with C++11
CFLAGS=--std=c++11
# enable debugging
LDFLAGS=-g
# add external static libs search path
LDFLAGS+=-Letc/lib
# add inih
CLIBS+=-linih

# peforming some system checks
ifeq ($(OS),Windows_NT)
	echo This program was not degined for Windows. We're sorry.
	exit 1
else
endif

# if the user only type make then everything will be compiled
all: $(PROG_NAME) inith

inith:
	$(MAKE) -C etc/ -f inih.mk static

$(PROG_NAME): main.o
	$(CXX) $(LDFLAGS) main.o -o $(PROG_NAME) $(CLIBS)

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

clear:
	rm -f *.o $(PROG_NAME)

pack:
	chmod +x pack.sh
	./pack.sh 'trab1'