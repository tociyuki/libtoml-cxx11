DECODERS=yaml-decoder-number.cpp

EXECUTES=lex-number number-test

CXX=clang++ -std=c++11
CXXFLAGS=-Wall

all : $(DECODERS)

lex-number : lex-number.cpp
	$(CXX) $(CXXFLAGS) -o lex-number lex-number.cpp

yaml-decoder-number.cpp : lex-number
	./lex-number > yaml-decoder-number.cpp

number-test : yaml-decoder-number.cpp number-test.cpp ../../value.o ../../setter.o
	$(CXX) $(CXXFLAGS) -o number-test number-test.cpp ../../value.o ../../setter.o

../../value.o : ../../value.cpp
	$(CXX) $(CXXFLAGS) -o ../../value.o -c ../../value.cpp

../../setter.o : ../../setter.cpp
	$(CXX) $(CXXFLAGS) -o ../../setter.o -c ../../setter.cpp

clean:
	rm -fr $(DECODERS) $(EXECUTES)
