DECODERS=toml-decoder-decode.cpp \
	 toml-decoder-key.cpp \
	 toml-decoder-value.cpp \
	 toml-decoder-string.cpp \
	 toml-decoder-number.cpp

EXECUTES=grammar lex-key lex-value lex-string lex-number \
	 key-test number-test string-test value-test

CXX=clang++ -std=c++11
CXXFLAGS=-Wall

all : $(DECODERS)

grammar : grammar-define-state.cpp grammar.cpp
	$(CXX) $(CXXFLAGS) -o grammar grammar.cpp

toml-decoder-decode.cpp : grammar
	./grammar > toml-decoder-decode.cpp

lex-key : lex-key.cpp
	$(CXX) $(CXXFLAGS) -o lex-key lex-key.cpp

toml-decoder-key.cpp : lex-key
	./lex-key > toml-decoder-key.cpp

lex-value : lex-value.cpp
	$(CXX) $(CXXFLAGS) -o lex-value lex-value.cpp

toml-decoder-value.cpp : lex-value
	./lex-value > toml-decoder-value.cpp

lex-string : lex-string.cpp
	$(CXX) $(CXXFLAGS) -o lex-string lex-string.cpp

toml-decoder-string.cpp : lex-string
	./lex-string > toml-decoder-string.cpp

lex-number : lex-number.cpp
	$(CXX) $(CXXFLAGS) -o lex-number lex-number.cpp

toml-decoder-number.cpp : lex-number
	./lex-number > toml-decoder-number.cpp

key-test : toml-decoder-key.cpp toml-decoder-string.cpp key-test.cpp ../../value.o ../../setter.o
	$(CXX) $(CXXFLAGS) -o key-test key-test.cpp ../../value.o ../../setter.o

number-test : toml-decoder-number.cpp number-test.cpp ../../value.o ../../setter.o
	$(CXX) $(CXXFLAGS) -o number-test number-test.cpp ../../value.o ../../setter.o

string-test : toml-decoder-string.cpp string-test.cpp ../../value.o ../../setter.o
	$(CXX) $(CXXFLAGS) -o string-test string-test.cpp ../../value.o ../../setter.o

value-test : toml-decoder-value.cpp toml-decoder-string.cpp toml-decoder-number.cpp value-test.cpp ../../value.o ../../setter.o
	$(CXX) $(CXXFLAGS) -o value-test value-test.cpp ../../value.o ../../setter.o

../../value.o : ../../value.cpp
	$(CXX) $(CXXFLAGS) -o ../../value.o -c ../../value.cpp

../../setter.o : ../../setter.cpp
	$(CXX) $(CXXFLAGS) -o ../../setter.o -c ../../setter.cpp

clean:
	rm -fr $(DECODERS) $(EXECUTES)
