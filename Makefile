CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -g -I.
PARSER = bison
PARSERFLAGS = -Wall -Werror -d
LEXER = flex
LEXERFLAGS =

# Targets
TARGET = compiler
PARSER_SRC = parser.tab.cpp
PARSER_HDR = parser.tab.hpp
LEXER_SRC = lex.yy.c
OBJS = main.o scanner.o parser.o astnode.o semantic_analyzer.o stageprocessor.o compiler.o

# Default build (normal)
all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

parser.tab.cpp parser.tab.hpp: parser.y
	@$(PARSER) $(PARSERFLAGS) -o $(PARSER_SRC) parser.y

lex.yy.c: lexer.l
	@$(LEXER) $(LEXERFLAGS) -o $(LEXER_SRC) lexer.l

parser.o: parser.tab.cpp parser.tab.hpp astnode.hpp
	@$(CXX) $(CXXFLAGS) -c -o $@ $(PARSER_SRC)

scanner.o: lex.yy.c parser.tab.hpp astnode.hpp
	@$(CXX) $(CXXFLAGS) -Wno-deprecated -Wno-sign-compare -c -o $@ $(LEXER_SRC)

astnode.o: astnode.cpp astnode.hpp
	@$(CXX) $(CXXFLAGS) -c -o $@ astnode.cpp

semantic_analyzer.o: semantic_analyzer.cpp semantic_analyzer.hpp astnode.hpp
	@$(CXX) $(CXXFLAGS) -c -o $@ semantic_analyzer.cpp

stageprocessor.o: stageprocessor.cpp stageprocessor.hpp astnode.hpp compiler_context.hpp semantic_analyzer.hpp parser.tab.hpp exception.hpp
	@$(CXX) $(CXXFLAGS) -c -o $@ stageprocessor.cpp

compiler.o: compiler.cpp compiler.hpp compiler_context.hpp stageprocessor.hpp
	@$(CXX) $(CXXFLAGS) -c -o $@ compiler.cpp

main.o: main.cpp compiler.hpp
	@$(CXX) $(CXXFLAGS) -c -o $@ main.cpp

clean:
	@rm -f $(TARGET) $(PARSER_SRC) $(PARSER_HDR) $(LEXER_SRC) *.o parser.output *.S
	@rm -rf test/result

.PHONY: all clean