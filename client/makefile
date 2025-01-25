CFLAGS := -c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS := -lboost_system -lpthread

all: StompEMIClient

StompEMIClient: bin bin/StompClient.o bin/Event.o bin/Parser.o bin/StompProtocol.o
	g++ -o bin/StompEMIClient bin/StompClient.o bin/Event.o bin/Parser.o bin/StompProtocol.o $(LDFLAGS)

bin:
	mkdir bin

bin/Event.o: src/Event.cpp
	g++ $(CFLAGS) -o bin/Event.o src/Event.cpp

bin/StompClient.o: src/StompClient.cpp
	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

bin/StompProtocol.o: src/StompProtocol.cpp
	g++ $(CFLAGS) -o bin/StompProtocol.o src/StompProtocol.cpp

bin/Parser.o: src/Parser.cpp
	g++ $(CFLAGS) -o bin/Parser.o src/Parser.cpp

# tests

EventParserTest: test/EventParser.cpp src/Event.cpp
	g++ -Iinclude -o bin/EventParserTest test/EventParser.cpp src/Event.cpp

SummaryTest: test/Summary.cpp src/Event.cpp
	g++ -Iinclude -o bin/SummaryTest test/Summary.cpp src/Event.cpp

.PHONY: clean run
clean:
	rm -f bin/*

run:
	./bin/StompEMIClient