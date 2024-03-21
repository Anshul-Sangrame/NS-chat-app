CXX = g++
CXXFLAGS = -I../headers/
OBJS = connection.o message.o client_connection.o server_connection.o

libhandler.a: $(OBJS)
	ar rcs libhandler.a $(OBJS)

message.o: message.cpp
connection.o: connection.cpp
client_connection.o: client_connection.cpp
server_connection.o: server_connection.cpp

clean:
	rm -f $(OBJS) libhandler.a