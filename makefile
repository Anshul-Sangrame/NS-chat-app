CC = g++
CXX = g++
CXXFLAGS = -I./headers/ -L./modules/ -lhandler 

all: pre main

pre: 
	cd modules && make
	
main: main.o

clean: 
	cd modules && make clean
	rm -f main.o main