CXX=clang++ -std=c++11
CXXFLAGS=-Wall

all : decoder-scan-number.cpp decoder-scan-string.cpp decoder-parse.cpp

decoder-scan-number.cpp : lex-number
	./lex-number > decoder-scan-number.cpp

decoder-scan-string.cpp : lex-string
	./lex-string > decoder-scan-string.cpp

decoder-parse.cpp : grammar
	./grammar > decoder-parse.cpp

lex-number : lex-number.cpp layoutable.hpp check-builder.hpp
	$(CXX) $(CXXFLAGS) -o lex-number lex-number.cpp

lex-string : lex-string.cpp layoutable.hpp check-builder.hpp
	$(CXX) $(CXXFLAGS) -o lex-string lex-string.cpp

grammar : grammar.cpp grammar-define-state.cpp layoutable.hpp check-builder.hpp
	$(CXX) $(CXXFLAGS) -o grammar grammar.cpp

scan-number-test : scan-number-test.cpp decoder-scan-number.cpp
	$(CXX) $(CXXFLAGS) -o scan-number-test scan-number-test.cpp

scan-string-test : scan-string-test.cpp decoder-scan-string.cpp
	$(CXX) $(CXXFLAGS) -o scan-string-test scan-string-test.cpp

clean:
	rm -fr decoder-scan-number.cpp decoder-scan-string.cpp \
		decoder-parse.cpp lex-number lex-string grammar \
		scan-number-test scan-string-test

