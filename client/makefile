CFLAGS := -c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS := -lboost_system -lpthread

all: StompEMIClient

StompEMIClient: bin bin/ConnectionHandler.o bin/StompClient.o bin/Event.o bin/Parser.o bin/StompProtocol.o bin/Utils.o
	g++ -o bin/StompEMIClient bin/ConnectionHandler.o bin/StompClient.o bin/Event.o bin/Parser.o bin/StompProtocol.o bin/Utils.o $(LDFLAGS)

bin:
	mkdir bin

bin/ConnectionHandler.o: src/ConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

bin/Event.o: src/Event.cpp
	g++ $(CFLAGS) -o bin/Event.o src/Event.cpp

bin/StompClient.o: src/StompClient.cpp
	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

bin/StompProtocol.o: src/StompProtocol.cpp
	g++ $(CFLAGS) -o bin/StompProtocol.o src/StompProtocol.cpp

bin/Parser.o: src/Parser.cpp
	g++ $(CFLAGS) -o bin/Parser.o src/Parser.cpp

bin/Utils.o: src/Utils.cpp
	g++ $(CFLAGS) -o bin/Utils.o src/Utils.cpp

.PHONY: clean run
clean:
	rm -f bin/*

run:
	./bin/StompEMIClient