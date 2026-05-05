CC      = gcc
CFLAGS  = -Wall -Wextra -g
TARGETS = dslc

all: $(TARGETS)

# Generate C from Flex/Bison
lex.yy.c: lexer.l parser.tab.h
	flex lexer.l

parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

# Compiler binary
dslc: main.c ast.c codegen.c lex.yy.c parser.tab.c
	$(CC) $(CFLAGS) -o $@ $^ -lfl

# Run a DSL file end-to-end
run: dslc
	@echo "── Compiling DSL ──────────────"
	./dslc $(SRC) -o _out.c -ast
	@echo "── Generated C ────────────────"
	@cat _out.c
	@echo "── GCC compile + run ──────────"
	$(CC) -o _out _out.c && ./_out

clean:
	rm -f dslc lex.yy.c parser.tab.c parser.tab.h _out.c _out *.o

.PHONY: all clean run
