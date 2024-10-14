#ifndef _ASSEMBLER_HPP
#define _ASSEMBLER_HPP

#include <list>
#include <map>
#include <string>
#include <vector>

#include "../inc/symboltable.hpp"
#include "../inc/realocationtable.hpp"
#include "../inc/sectiontable.hpp"

using namespace std;

class Assembler{
public:
  enum INSTRUCTION {
    HALT,
    INT, 
    IRET,
    CALL,
    RET,
    JMP,
    BEQ,
    BNE,
    BGT,
    PUSH,
    POP,
    XCHG,
    ADD,
    SUB,
    MUL,
    DIV,
    NOT,
    AND,
    OR,
    XOR,
    SHL,
    SHR,
    LD,
    ST,
    CSRRD,
    CSRWR,
    NOP
  };

  enum DIRECTIVE {
    GLOBAL,
    EXTERN,
    SECTION,
    WORD,
    SKIP,
    ASCII,
    EQU,
    END,
    LABEL,
    NDI,
    COMMENT
  };

  enum ADDRESSING {
    MEM_DIR,
    MEM_IND,
    IMM,
    REG_DIR,
    REG_IND,
    REG_IND_DISP,
    NAD 
  };

  Assembler(); 

  long literalToValue(string literal);

  void addInstruction(Assembler::INSTRUCTION instr, int regS1, int regS2, int regD, char* nameSymb, char* nameLit, Assembler::ADDRESSING adr);
  void addDirective(Assembler::DIRECTIVE dir, char* nameSymb, char* nameLit, list<char*> listSym);

  void pass();

  //instructions
  bool addHalt();
  bool addInt();
  bool addIret();
  bool addCall(int ls, string name);
  bool addRet();
  bool addJmp(int ls, string name);
  bool addBeq(int regS1, int regS2, int regD, string symbol, string literal);
  bool addBne(int regS1, int regS2, int regD, string symbol, string literal);
  bool addBgt(int regS1, int regS2, int regD, string symbol, string literal);
  bool addPush(int reg);
  bool addPop(int reg);
  bool addXchg(int reg1, int reg2);
  bool addAdd(int regS1, int regS2, int regD);
  bool addSub(int regS1, int regS2, int regD);
  bool addMul(int regS1, int regS2, int regD);
  bool addDiv(int regS1, int regS2, int regD);
  bool addNot(int regS, int regD);
  bool addAnd(int regS1, int regS2, int regD);
  bool addOr(int regS1, int regS2, int regD);
  bool addXor(int regS1, int regS2, int regD);
  bool addShl(int regS1, int regS2, int regD);
  bool addShr(int regS1, int regS2, int regD);
  bool addLd(Assembler::ADDRESSING adr, int regD, int regS, string symbol, string literal);
  bool addSt(Assembler::ADDRESSING adr, int regD, int regS, string symbol, string literal);
  bool addCsrrd(int regS, int regD);
  bool addCsrwr(int regS, int regD);

  //directives
  bool addGlobalSymbol(list<string> symbols);
  bool addExternSymbol(list<string> symbols);
  bool addSection(string name);
  bool addWordSymbol(list<string> symbols);
  bool addLabel(string label);
  bool addSkip(string literal);
  bool addAscii(string text);

  void createMachineInstruction(int opCode, int mode, int regA, int regB, int regC, int disp);
  void addToBackpatch(string section, int line, string literal, string symbol, bool isGlobal);

  bool doBackpatch();
  void setFileName(char* name);

private:
  struct Elem {
    Assembler::INSTRUCTION instr = NOP;
    int regS1 = -1;
    int regS2 = -1;
    int regD = -1;
    string nameSymb = "";
    string nameLit = "";
    Assembler::ADDRESSING adr = NAD;
    Assembler::DIRECTIVE dir = NDI;
    list<string> listSym;

    Elem(Assembler::INSTRUCTION instr, int regS1, int regS2, int regD, char* nameSymb, char* nameLit, Assembler::ADDRESSING adr): instr(instr),
    regS1(regS1), regS2(regS2), regD(regD), adr(adr) {
      if(nameSymb != 0) this->nameSymb = string(nameSymb);
      if(nameLit != 0) this->nameLit = string(nameLit);
    }

    Elem(Assembler::DIRECTIVE dir, char* nameSymb, char* nameLit, list<char*> listSym): dir(dir) {
      if(nameSymb != 0) this->nameSymb = string(nameSymb);
      if(nameLit != 0) this->nameLit = string(nameLit);
      while(!listSym.empty()) {
        this->listSym.push_back(string(listSym.front()));
        listSym.pop_front();
      }
    }
  };

  list<Elem> lines;
  string currentSection = "";
  string message;
  string fileName;
  
  void printToTxtFile();
  void printToBinaryFile();

  SymbolTable symbolTable;
  SectionTable sectionTable;
};

#endif