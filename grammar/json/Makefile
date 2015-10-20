DECODERS=json-decoder-decode.cpp \
	 json-decoder-next-token.cpp \
	 json-decoder-string.cpp \
	 json-decoder-number.cpp

EXECUTES=grammar lex-next-token lex-string lex-number

CXX=clang++ -std=c++11
CXXFLAGS=-Wall

all : $(DECODERS)

grammar : grammar-define-state.cpp grammar.cpp
	$(CXX) $(CXXFLAGS) -o grammar grammar.cpp

json-decoder-decode.cpp : grammar
	./grammar > json-decoder-decode.cpp

lex-next-token : lex-next-token.cpp
	$(CXX) $(CXXFLAGS) -o lex-next-token lex-next-token.cpp

json-decoder-next-token.cpp : lex-next-token
	./lex-next-token > json-decoder-next-token.cpp

lex-string : lex-string.cpp
	$(CXX) $(CXXFLAGS) -o lex-string lex-string.cpp

json-decoder-string.cpp : lex-string
	./lex-string > json-decoder-string.cpp

lex-number : lex-number.cpp
	$(CXX) $(CXXFLAGS) -o lex-number lex-number.cpp

json-decoder-number.cpp : lex-number
	./lex-number > json-decoder-number.cpp

clean :
	rm -fr $(DECODERS) $(EXECUTES)
