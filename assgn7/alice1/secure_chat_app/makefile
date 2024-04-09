CXX = g++
CXXFLAGS = -I./headers/
LDFLAGS = -L./modules/ -lhandler -lssl -lcrypto -lpthread -lncurses

.PHONY: main pre T test all clean

all: pre main

test: pre T

pre: 
	cd modules && make
	
T: test.cpp ./modules/libhandler.a
	$(CXX) $(CXXFLAGS) test.cpp $(LDFLAGS) -o main

main: main.cpp ./modules/libhandler.a
	$(CXX) $(CXXFLAGS) main.cpp $(LDFLAGS) -o main

clean: 
	cd modules && make clean
	rm -f main.o main