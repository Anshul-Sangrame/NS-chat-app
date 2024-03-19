CXX = g++
CXXFLAGS = -I./headers/

all: pre main

pre: 
	cd modules && make
	
main: main.cpp ./modules/libhandler.a
	$(CXX) $(CXXFLAGS) main.cpp -L./modules/ -lhandler -lssl -lcrypto -o main

clean: 
	cd modules && make clean
	rm -f main.o main