all: S C

S: sUDP.cpp
	g++ sUDP.cpp -lssl -lcrypto -o server

C: cUDP.cpp
	g++ cUDP.cpp -lssl -lcrypto -o client
