OBJS=value.o \
     setter.o \
     json-encoder.o \
     json-decoder.o \
     toml-encoder.o \
     toml-decoder.o \
     encode-utf8.o \
     mustache.o

TESTS=value-test \
      setter-test \
      json-encoder-test \
      json-decoder-test \
      toml-encoder-test \
      toml-decoder-test \
      mustache-test

CXX=clang++ -std=c++11
CXXFLAGS=-Wall -O2

all : $(OBJS)

value.o : value.hpp value.cpp
	$(CXX) $(CXXFLAGS) -o value.o -c value.cpp

setter.o : value.hpp setter.cpp
	$(CXX) $(CXXFLAGS) -o setter.o -c setter.cpp

json-encoder.o : value.hpp json.hpp json-encoder.cpp
	$(CXX) $(CXXFLAGS) -o json-encoder.o -c json-encoder.cpp

json-decoder.o : value.hpp json.hpp json-decoder.cpp
	$(CXX) $(CXXFLAGS) -o json-decoder.o -c json-decoder.cpp

toml-encoenr.o : value.hpp toml.hpp toml-encoenr.cpp
	$(CXX) $(CXXFLAGS) -o toml-encoenr.o -c toml-encoenr.cpp

toml-decoder.o : value.hpp toml.hpp toml-decoder.cpp
	$(CXX) $(CXXFLAGS) -o toml-decoder.o -c toml-decoder.cpp

encode-utf8.o : value.hpp toml.hpp encode-utf8.cpp
	$(CXX) $(CXXFLAGS) -o encode-utf8.o -c encode-utf8.cpp

mustache.o : value.hpp toml.hpp mustache.cpp
	$(CXX) $(CXXFLAGS) -o mustache.o -c mustache.cpp

all-test : $(TESTS)

value-test : value.o setter.o value-test.cpp
	$(CXX) $(CXXFLAGS) -o value-test value-test.cpp value.o setter.o

setter-test: value.o setter.o setter-test.cpp
	$(CXX) $(CXXFLAGS) -o setter-test setter-test.cpp value.o setter.o

json-encoder-test: value.o setter.o json-encoder.o json-encoder-test.cpp
	$(CXX) $(CXXFLAGS) -o json-encoder-test json-encoder-test.cpp value.o setter.o json-encoder.o

json-decoder-test: value.o setter.o json-decoder.o json-decoder-test.cpp
	$(CXX) $(CXXFLAGS) -o json-decoder-test json-decoder-test.cpp value.o setter.o json-decoder.o

toml-encoder-test: value.o setter.o toml-encoder.o toml-encoder-test.cpp
	$(CXX) $(CXXFLAGS) -o toml-encoder-test toml-encoder-test.cpp value.o setter.o toml-encoder.o

toml-decoder-test: value.o setter.o toml-decoder.o toml-decoder-test.cpp
	$(CXX) $(CXXFLAGS) -o toml-decoder-test toml-decoder-test.cpp value.o setter.o toml-decoder.o

mustache-test: value.o setter.o json-decoder.o encode-utf8.o mustache.o mustache-test.cpp
	$(CXX) $(CXXFLAGS) -o mustache-test mustache-test.cpp value.o setter.o json-decoder.o encode-utf8.o mustache.o

clean :
	rm -fr *-test *.o
