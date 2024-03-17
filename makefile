CXX = g++
CXXFLAGS = -I./headers/ -L./modules/

all: pre main

pre: 
	cd modules && make
	
main: main.cpp ./modules/libhandler.a
	$(CXX) $(CXXFLAGS) main.cpp -lhandler -o main

clean: 
	cd modules && make clean
	rm -f main.o main