CXX = g++
CXXFLAGS = -I../headers/

libhandler.a: connection.o
	ar rcs libhandler.a connection.o 

connection.o: connection.cpp

clean:
	rm -f connection.o libhandler.a