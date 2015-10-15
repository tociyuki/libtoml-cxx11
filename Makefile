OBJECTS=doc.o encode.o decode.o

CXX=clang++ -std=c++11
CXXFLAGS=-Wall -O2

all: doc.o encode.o decode.o

doc.o: toml.hpp doc.cpp
	$(CXX) $(CXXFLAGS) -o doc.o -c doc.cpp

encode.o: toml.hpp encode.cpp
	$(CXX) $(CXXFLAGS) -o encode.o -c encode.cpp

decode.o: toml.hpp decode.cpp
	$(CXX) $(CXXFLAGS) -o decode.o -c decode.cpp

example: example.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o example example.cpp $(OBJECTS)

clean:
	rm -fr $(OBJECTS) example
