SRC_ASSEMBLER = bison.tab.cpp lex.yy.cpp ./src/mainassembler.cpp ./src/assembler.cpp ./src/sectiontable.cpp ./src/symboltable.cpp ./src/realocationtable.cpp
SRC_LINKER = ./src/linker.cpp ./src/mainlinker.cpp ./src/realocationtable.cpp ./src/sectiontable.cpp ./src/symboltable.cpp 
SRC_EMULATOR = ./src/emulator.cpp ./src/mainemulator.cpp

GPP = g++

all: flexBison assembler linker emulator clean

assembler: $(SRC_ASSEMBLER)
	$(GPP) -g $(SRC_ASSEMBLER) -o assembler

linker: $(SRC_LINKER)
	$(GPP) -g $(SRC_LINKER) -o linker

emulator: $(SRC_EMULATOR)
	$(GPP) -g $(SRC_EMULATOR) -o emulator

flexBison:
	flex -o lex.yy.cpp "./misc/flex.l"
	bison -d -o bison.tab.cpp "./misc/bison.y"

clean:
	rm -f lex.yy.cpp bison.tab.cpp bison.tab.hpp 