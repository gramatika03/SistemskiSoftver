#include "../inc/assembler.hpp"

#include <iostream>
#include <list>
#include <cstring>
#include <bitset>
#include <iomanip>
#include <fstream>

Assembler::Assembler() {}

void Assembler::setFileName(char* name) {
  fileName = string(name);
}


long Assembler::literalToValue(string literal) {
  long literalValue = 0;
  if(literal.size() > 2 && literal.at(0) == '0' && literal.at(1) == 'x') literalValue = stol(literal, nullptr, 16);
  else literalValue = stol(literal, nullptr, 10);
  return literalValue;
}

void Assembler::addInstruction(Assembler::INSTRUCTION instr, int regS1, int regS2, int regD, char* nameSymb, char* nameLit, Assembler::ADDRESSING adr) {
  Elem novi(instr, regS1, regS2, regD, nameSymb, nameLit, adr);
  lines.push_back(novi);
}

void Assembler::addDirective(Assembler::DIRECTIVE dir, char* nameSymb, char* nameLit, std::list<char*> listSym) {
  Elem novi(dir, nameSymb, nameLit, listSym);
  lines.push_back(novi);
}

void Assembler::createMachineInstruction(int opCode, int mode, int regA, int regB, int regC, int disp) {
  char byte3 = 0;
  char byte2 = 0;
  char byte1 = 0;
  char byte0 = 0;
  byte3 = ((opCode << 4) & 0xF0)   | (mode & 0xF);
  byte2 = ((regA << 4) & 0xF0)     | (regB & 0xF);
  byte1 = ((regC << 4) & 0xF0)     | ((disp >> 8) & 0xF);
  byte0 = ((disp >> 4) & 0xF) << 4 | ((disp) & 0xF);
  sectionTable[currentSection].instructions.push_back(byte0);
  sectionTable[currentSection].instructions.push_back(byte1);
  sectionTable[currentSection].instructions.push_back(byte2);
  sectionTable[currentSection].instructions.push_back(byte3);  
  sectionTable[currentSection].size += 4;
}

void Assembler::addToBackpatch(string section, int line, string symbol, string literal, bool isGlobal) {
  sectionTable.addToBackpatch(section, line, symbol, literal, isGlobal);
}

bool Assembler::addHalt() {
  createMachineInstruction(0b0000, 0b0000, 0, 0, 0, 0);
  return true;
}

bool Assembler::addInt() {
  createMachineInstruction(0b0001, 0b0000, 0, 0, 0, 0);
  return true;
}

bool Assembler::addIret() {
  createMachineInstruction(0b1001, 0b0110, 0, 14, 0, 4);
  createMachineInstruction(0b1001, 0b0011, 15, 14, 0, 8);
  return true;
}

bool Assembler::addCall(int ls, string name) {
  if(ls == 0) {//
    long literalValue = literalToValue(name);
    if(literalValue >= (1UL << 32)) {
      message = "GIVEN ADDRESS IS OUT OF ADDRESS SPACE";
      return false;
    }
    if(literalValue < (1 << 12)) {
      createMachineInstruction(0b0010, 0b0000, 0, 0, 0, literalValue);
      return true;
    } else {
      createMachineInstruction(0b0010, 0b0001, 15, 0, 0, 0);
      addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), "", name, false);
      return true;
    }
  } else if(ls == 1) {
    if(symbolTable[name].name != "") {
      if(symbolTable[name].ndx == symbolTable[sectionTable[currentSection].name].value) {
        if(symbolTable[name].isDefined) {
          createMachineInstruction(0b0010, 0b0000, 15, 0, 0, symbolTable[name].value - sectionTable[currentSection].size);
          return true;
        } else {
          createMachineInstruction(0b0010, 0b0001, 15, 0, 0, 0);
          addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), name, "", symbolTable[name].bind);
          return true;
        }
      } else {
        createMachineInstruction(0b0010, 0b0001, 15, 0, 0, 0);
        addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), name, "", symbolTable[name].bind);
        return true;
      }
    } else {
      createMachineInstruction(0b0010, 0b0001, 15, 0, 0, 0);
      addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), name, "", symbolTable[name].bind);
      return true;
    }
  }
  return true;
}

bool Assembler::addRet() {
  createMachineInstruction(0b1001, 0b0011, 15, 14, 0, 4);
  return true;
}

bool Assembler::addJmp(int ls, string name) {
  if(ls == 0) {//
    long literalValue = literalToValue(name);
    if(literalValue >= (1UL << 32)) {
      message = "GIVEN ADDRESS IS OUT OF ADDRESS SPACE";
      return false;
    }
    if(literalValue < (1 << 12)) {
      createMachineInstruction(0b0011, 0b0000, 0, 0, 0, literalValue);
      return true;
    } else {
      createMachineInstruction(0b0011, 0b1000, 15, 0, 0, 0);
      addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), "", name, false);
      return true;
    }
  } else if(ls == 1) {
    if(symbolTable[name].name != "") {
      if(symbolTable[name].ndx == symbolTable[sectionTable[currentSection].name].value) {
        if(symbolTable[name].isDefined) {
          createMachineInstruction(0b0011, 0b0000, 15, 0, 0, symbolTable[name].value - sectionTable[currentSection].size);
          return true;
        } else {
          createMachineInstruction(0b0011, 0b1000, 15, 0, 0, 0);
          addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), name, "", symbolTable[name].bind);
          return true;
        }
      } else {
        createMachineInstruction(0b0011, 0b1000, 15, 0, 0, 0);
        addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), name, "", symbolTable[name].bind);
        return true;
      }
    } else {
      createMachineInstruction(0b0011, 0b1000, 15, 0, 0, 0);
      addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), name, "", symbolTable[name].bind);
      return true;
    }
  }
  return true;
}

bool Assembler::addBeq(int regS1, int regS2, int regD, string symbol, string literal) {
  int operation = 0b0011;
  int modeDir = 0b0001;
  int modeInd = 0b1001;
  if(literal != "") {
    long literalValue = literalToValue(literal);
    if(literalValue >= (1UL << 32)) {
      message = "GIVEN ADDRESS IS OUT OF ADDRESS SPACE";
      return false;
    }
    if(literalValue < (1 << 12)) {
      createMachineInstruction(operation, modeDir, 0, regS1, regS2, literalValue);
      return true;
    } else {
      createMachineInstruction(operation, modeInd, 15, regS1, regS2, 0);
      addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), "", literal, false);
      return true;
    }
  } else if (symbol != "") {
      if(symbolTable[symbol].name != "") {
        if(symbolTable[symbol].ndx == symbolTable[sectionTable[currentSection].name].value) {
          if(symbolTable[symbol].isDefined) {
            createMachineInstruction(operation, modeDir, 15, regS1, regS2, symbolTable[symbol].value - sectionTable[currentSection].size);
            return true;
          } else {
            createMachineInstruction(operation, modeInd, 15, regS1, regS2, 0);
            addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
            return true;
          }
        } else {
          createMachineInstruction(operation, modeInd, 15, regS1, regS2, 0);
          addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
          return true;
        }
      } else {
        createMachineInstruction(operation, modeInd, 15, regS1, regS2, 0);
        addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
        return true;
      }
    }
  return true;
}

bool Assembler::addBne(int regS1, int regS2, int regD, string symbol, string literal) {
  int operation = 0b0011;
  int modeDir = 0b0010;
  int modeInd = 0b1010;
  if(literal != "") {
    long literalValue = literalToValue(literal);
    if(literalValue >= (1UL << 32)) {
      message = "GIVEN ADDRESS IS OUT OF ADDRESS SPACE";
      return false;
    }
    if(literalValue < (1 << 12)) {
      createMachineInstruction(operation, modeDir, 0, regS1, regS2, literalValue);
      return true;
    } else {
      createMachineInstruction(operation, modeInd, 15, regS1, regS2, 0);
      addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), "", literal, false);
      return true;
    }
  } else if (symbol != "") {
      if(symbolTable[symbol].name != "") {
        if(symbolTable[symbol].ndx == symbolTable[sectionTable[currentSection].name].value) {
          if(symbolTable[symbol].isDefined) {
            createMachineInstruction(operation, modeDir, 15, regS1, regS2, symbolTable[symbol].value - sectionTable[currentSection].size);
            return true;
          } else {
            createMachineInstruction(operation, modeInd, 15, regS1, regS2, 0);
            addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
            return true;
          }
        } else {
          createMachineInstruction(operation, modeInd, 15, regS1, regS2, 0);
          addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
          return true;
        }
      } else {
        createMachineInstruction(operation, modeInd, 15, regS1, regS2, 0);
        addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
        return true;
      }
    }
  return true;
}

bool Assembler::addBgt(int regS1, int regS2, int regD, string symbol, string literal) {
  int operation = 0b0011;
  int modeDir = 0b0011;
  int modeInd = 0b1011;
  if(literal != "") {
    long literalValue = literalToValue(literal);
    if(literalValue >= (1UL << 32)) {
      message = "GIVEN ADDRESS IS OUT OF ADDRESS SPACE";
      return false;
    }
    if(literalValue < (1 << 12)) {
      createMachineInstruction(operation, modeDir, 0, regS1, regS2, literalValue);
      return true;
    } else {
      createMachineInstruction(operation, modeInd, 15, regS1, regS2, 0);
      addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), "", literal, false);
      return true;
    }
  } else if (symbol != "") {
      if(symbolTable[symbol].name != "") {
        if(symbolTable[symbol].ndx == symbolTable[sectionTable[currentSection].name].value) {
          if(symbolTable[symbol].isDefined) {
            createMachineInstruction(operation, modeDir, 15, regS1, regS2, symbolTable[symbol].value - sectionTable[currentSection].size);
            return true;
          } else {
            createMachineInstruction(operation, modeInd, 15, regS1, regS2, 0);
            addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
            return true;
          }
        } else {
          createMachineInstruction(operation, modeInd, 15, regS1, regS2, 0);
          addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
          return true;
        }
      } else {
        createMachineInstruction(operation, modeInd, 15, regS1, regS2, 0);
        addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
        return true;
      }
    }
  return true;
}

bool Assembler::addPush(int reg) {
  createMachineInstruction(0b1000, 0b0001, 14, 0, reg, -4);
  return true;
}

bool Assembler::addPop(int reg) {
  createMachineInstruction(0b1001, 0b0011, reg, 14, 0, 4);
  return true;
}

bool Assembler::addXchg(int reg1, int reg2) {
  createMachineInstruction(0b0100, 0, 0, reg1, reg2, 0);
  return true;
}

bool Assembler::addAdd(int regS1, int regS2, int regD) {
  createMachineInstruction(0b0101, 0b0000, regD, regS1, regS2, 0);
  return true;
}

bool Assembler::addSub(int regS1, int regS2, int regD) {
  createMachineInstruction(0b0101, 0b0001, regD, regS1, regS2, 0);
  return true;
}

bool Assembler::addMul(int regS1, int regS2, int regD) {
  createMachineInstruction(0b0101, 0b0010, regD, regS1, regS2, 0);
  return true;
}

bool Assembler::addDiv(int regS1, int regS2, int regD) {
  createMachineInstruction(0b0101, 0b0011, regD, regS1, regS2, 0);
  return true;
}

bool Assembler::addNot(int regS, int regD) {
  createMachineInstruction(0b0110, 0b0000, regD, regS, 0, 0);
  return true;
}

bool Assembler::addAnd(int regS1, int regS2, int regD) {
  createMachineInstruction(0b0110, 0b0001, regD, regS1, regS2, 0);
  return true;
}

bool Assembler::addOr(int regS1, int regS2, int regD) {
  createMachineInstruction(0b0110, 0b0010, regD, regS1, regS2, 0);
  return true;
}

bool Assembler::addXor(int regS1, int regS2, int regD) {
  createMachineInstruction(0b0110, 0b0011, regD, regS1, regS2, 0);
  return true;
}

bool Assembler::addShl(int regS1, int regS2, int regD) {
  createMachineInstruction(0b0111, 0b0000, regD, regS1, regS2, 0);
  return true;
}

bool Assembler::addShr(int regS1, int regS2, int regD) {
  createMachineInstruction(0b0111, 0b0001, regD, regS1, regS2, 0);
  return true;
}

bool Assembler::addLd(Assembler::ADDRESSING adr, int regD, int regS, string symbol, string literal) {
  int operation;
  int modeDir;
  int modeInd;
  int mode;
  switch(adr) {
    case MEM_DIR:
      operation = 0b1001;
      modeDir = 0b0001;
      modeInd = 0b0010;
      if(literal != "") {
        long literalValue = literalToValue(literal);
        if(literalValue >= (1UL << 32)) {
          message = "GIVEN ADDRESS IS OUT OF ADDRESS SPACE";
          return false;
        }
        if(literalValue < (1 << 12)) {
          createMachineInstruction(operation, modeDir, regD, 0, 0, literalValue);
          return true;
        } else {
          createMachineInstruction(operation, modeInd, regD, 15, 0, 0);
          addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), "", literal, false);
          return true;
        }
      } else if(symbol != "") {
          if(symbolTable[symbol].name != "") {
            if(symbolTable[symbol].ndx == symbolTable[sectionTable[currentSection].name].value) {
              if(symbolTable[symbol].isDefined) {
                createMachineInstruction(operation, modeDir, 15, 0, 0, symbolTable[symbol].value - sectionTable[currentSection].size);
                return true;
              } else {
                createMachineInstruction(operation, modeInd, regD, 15, 0, 0);
                addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
                return true;
              }
            } else {
              createMachineInstruction(operation, modeInd, regD, 15, 0, 0);
              addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
              return true;
            }
          } else {
            createMachineInstruction(operation, modeInd, regD, 15, 0, 0);
            addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
            return true;
          }
      }
      break;
    case MEM_IND:
      operation = 0b1001;
      mode = 0b0010;
      addLd(MEM_DIR, regD, regS, symbol, literal);
      createMachineInstruction(operation, mode, regD, regD, 0, 0);
      break;
    case REG_DIR:
      createMachineInstruction(0b1001, 0b0001, regD, regS, 0, 0);
      break;
    case REG_IND:
      createMachineInstruction(0b1001, 0b0010, regD, regS, 0, 0);
      break;
    case REG_IND_DISP:
      if(literal != "") {
        long literalValue = literalToValue(literal);
        if(literalValue >= (1UL << 12)) {
          message = "LITERAL VALUE CAN'T BE LARGER THAN 12b";
          return false;
        }
        createMachineInstruction(0b1001, 0b0010, regD, regS, 0, literalValue);
      } else if(symbol != "") {
        return true;
        message = "SYMBOL MUST BE DEFINED DURING ASSEMBLING";
        return false;
      }
      break;
 
  };
  return true;
}

bool Assembler::addSt(Assembler::ADDRESSING adr, int regD, int regS, string symbol, string literal) {
  switch(adr) {
    case MEM_DIR:
      if(literal != "") {
        long literalValue = literalToValue(literal);
        if(literalValue >= (1UL << 32)) {
          message = "GIVEN ADDRESS IS OUT OF ADDRESS SPACE";
          return false;
        }
        if(literalValue < (1 << 12)) {
          createMachineInstruction(0b1000, 0b0000, 0, 0, regS, literalValue);
          return true;
        } else {
          createMachineInstruction(0b1000, 0b0000, 15, 0, regS, 0);
          addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), "", literal, false);
          return true;
        }
      } else if(symbol != "") {
          if(symbolTable[symbol].name != "") {
            if(symbolTable[symbol].ndx == symbolTable[sectionTable[currentSection].name].value) {
              if(symbolTable[symbol].isDefined) {
                createMachineInstruction(0b1000, 0b0000, 15, 0, regS, symbolTable[symbol].value - sectionTable[currentSection].size);
                return true;
              } else {
                createMachineInstruction(0b1000, 0b0000, 15, 0, regS, 0);
                addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
                return true;
              }
            } else {
              createMachineInstruction(0b1000, 0b0000, 15, 0, regS, 0);
              addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
              return true;
            }
          } else {
            createMachineInstruction(0b1000, 0b0000, 15, 0, regS, 0);
            addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
            return true;
          }
      }
      break;
    case MEM_IND:
      if(literal != "") {
        long literalValue = literalToValue(literal);
        if(literalValue >= (1UL << 32)) {
          message = "GIVEN ADDRESS IS OUT OF ADDRESS SPACE";
          return false;
        }
        if(literalValue < (1 << 12)) {
          createMachineInstruction(0b1000, 0b0010, 0, 0, regS, literalValue);
          return true;
        } else {
          createMachineInstruction(0b1000, 0b0010, 15, 0, regS, 0);
          addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), "", literal, false);
          return true;
        }
      } else if(symbol != "") {
          if(symbolTable[symbol].name != "") {
            if(symbolTable[symbol].ndx == symbolTable[sectionTable[currentSection].name].value) {
              if(symbolTable[symbol].isDefined) {
                createMachineInstruction(0b1000, 0b0010, 15, 0, regS, symbolTable[symbol].value - sectionTable[currentSection].size);
                return true;
              } else {
                createMachineInstruction(0b1000, 0b0010, 15, 0, regS, 0);
                addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
                return true;
              }
            } else {
              createMachineInstruction(0b1000, 0b0010, 15, 0, regS, 0);
              addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
              return true;
            }
          } else {
            createMachineInstruction(0b1000, 0b0010, 15, 0, regS, 0);
            addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), symbol, "", symbolTable[symbol].bind);
            return true;
          }
      }
      break;
    case REG_DIR:
      addLd(adr, regS, regD, symbol, literal);
      break;
    case REG_IND:
      createMachineInstruction(0b1000, 0b0000, regD, 0, regS, 0);
      break;
    case REG_IND_DISP:
      if(literal != "") {
        long literalValue = literalToValue(literal);
        if(literalValue >= (1UL << 12)) {
          message = "LITERAL VALUE CAN'T BE LARGER THAN 12b";
          return false;
        }
        createMachineInstruction(0b1000, 0b0000, regD, 0, regS, literalValue);
      } else if(symbol != "") {
        createMachineInstruction(-1, -1, -1, -1, -1, -1);
        return true;
        message = "SYMBOL MUST BE DEFINED DURING ASSEMBLING";
        return false;
      }
      break;
 
  };
  return true;
}

bool Assembler::addCsrrd(int regS, int regD) {
  createMachineInstruction(0b1001, 0b0000, regD, regS, 0, 0);
  return true;
}

bool Assembler::addCsrwr(int regS, int regD) {
  createMachineInstruction(0b1001, 0b0100, regD, regS, 0, 0);
  return true;
}

bool Assembler::addGlobalSymbol(list<string> symbols) {
  while(!symbols.empty()) {
    string symbol = symbols.front();
    symbols.pop_front();
    if(symbolTable[symbol].name == "") {
      symbolTable.addNewElem(0, 0, SymbolTable::NOTYP, SymbolTable::GLOBAL, currentSection != ""?symbolTable[currentSection].num:-1, symbol, false, false, false);
    } else {
      symbolTable[symbol].bind = SymbolTable::GLOBAL;
    }
  }
  return true;
}

bool Assembler::addExternSymbol(list<string> symbols) {
  while(!symbols.empty()) {
    string symbol = symbols.front();
    symbols.pop_front();
    if(symbolTable[symbol].name == "") {
      symbolTable.addNewElem(0, 0, SymbolTable::NOTYP, SymbolTable::GLOBAL, -1, symbol, true, false, false);
    } else {
      if(!(symbolTable[symbol].bind == SymbolTable::GLOBAL) && !symbolTable[symbol].isExtern) {
        message = "ERROR WHILE ADDING EXTERN SYMBOL TO SYMBOL TABLE";
        return false;
      }
    }
  }
  return true;
}

bool Assembler::addSection(string name) {
  if(symbolTable[name].name == "") {
    symbolTable.addNewElem(0, 0, SymbolTable::SCTN, SymbolTable::LOCAL, -2, name, false, true, true);
    sectionTable.addNewSection(name);
    currentSection = name;
  } else {
    if(!symbolTable[name].isSection) {
      message = "SECTION NAME ALREADY DEFINED";
      return false;
    } else {
      currentSection = name;
    }
  }
  return true;
}

bool Assembler::addWordSymbol(list<string> symbols) {
  while(!symbols.empty()) {
    string tek = symbols.front();
    symbols.pop_front();
    if(tek.at(0) == '0' || tek.at(0) == '1' || tek.at(0) == '2' || tek.at(0) == '3' || 
      tek.at(0) == '4' || tek.at(0) == '5' || tek.at(0) == '6' || tek.at(0) == '7' || 
      tek.at(0) == '8' || tek.at(0) == '9'){
        long literalValue = literalToValue(tek);
        if(literalValue > (1UL << 32)) {
          message = "GIVEN LITERAL IS LARGER THAN EIGHT BYTES";
          return false;
        }
        char byte0;
        char byte1;
        char byte2;
        char byte3;
        byte0 = (literalValue & 0xFF);
        byte1 = (literalValue >>  8 & 0xFF);
        byte2 = (literalValue >> 16 & 0xFF);
        byte3 = (literalValue >> 24 & 0xFF);
        sectionTable[currentSection].instructions.push_back(byte0);
        sectionTable[currentSection].instructions.push_back(byte1);
        sectionTable[currentSection].instructions.push_back(byte2);
        sectionTable[currentSection].instructions.push_back(byte3);
        sectionTable[currentSection].size += 4;
        return true;
    } else {
      createMachineInstruction(0b0000, 0b0000, 0, 0, 0, 0);
      addToBackpatch(currentSection, sectionTable[currentSection].instructions.size(), tek, "", symbolTable[tek].bind);
      return true;
    }
  }
  return true;
}

bool Assembler::addLabel(string label) {
  if(symbolTable[label].name == "") {
    symbolTable.addNewElem(sectionTable[currentSection].size, 0, SymbolTable::NOTYP, SymbolTable::LOCAL, currentSection != ""?symbolTable[currentSection].num:-1, label, false, true, false);
  } else {
    if(symbolTable[label].isDefined) {
      message = "SYMBOL CAN'T BE DEFINED MORE THAT ONCE IN THE SAME FILE";
      return false;
    }
    symbolTable.setDefinition(symbolTable[label].num, sectionTable[currentSection].size, symbolTable[currentSection].num);
  }
  return true;
}

bool Assembler::addSkip(string literal) {
  long literalValue = literalToValue(literal);
  while(literalValue >= 4) {
    createMachineInstruction(0, 0, 0, 0, 0, 0);
    literalValue -= 4;
  }
  while(literalValue != 0) {
    sectionTable[currentSection].instructions.push_back(0);
    sectionTable[currentSection].size += 1;
    literalValue -= 1;
  }
  return true;
}

bool Assembler::addAscii(string text) {
  for(int i = 1; i < text.size() - 1; i++) {
    char c = text.at(i);
    sectionTable[currentSection].instructions.push_back(c);
    sectionTable[currentSection].size += 1;
  }
  return true;
}

void Assembler::pass() {
  int lineCounter = 1;
  bool done = true;
  bool backpatch = false;
  while(!lines.empty()) {
    Elem tek = lines.front();
    lines.pop_front();
    if(tek.instr != NOP) {
      if(currentSection == "") {
        cout << "INSTRUCTIONS MUST BE INSIDE OF A SECTION\n";
        return;
      }
      switch(tek.instr){
        case HALT:
          done = addHalt();
          break;
        case INT:
          done = addInt();
          break;
        case IRET:
          done = addIret();
          break;
        case CALL:
          if(tek.nameLit != "") done = addCall(0, tek.nameLit);
          else if(tek.nameSymb != "") done = addCall(1, tek.nameSymb);
          break;
        case RET:
          done = addRet();
          break;
        case JMP:
          if(tek.nameLit != "") done = addJmp(0, tek.nameLit);
          else if(tek.nameSymb != "") done = addJmp(1, tek.nameSymb);
          break;
        case BEQ:
          done = addBeq(tek.regS1, tek.regS2, tek.regD, tek.nameSymb, tek.nameLit);
          break;
        case BNE:
          done = addBne(tek.regS1, tek.regS2, tek.regD, tek.nameSymb, tek.nameLit);
          break;
        case BGT:
          done = addBgt(tek.regS1, tek.regS2, tek.regD, tek.nameSymb, tek.nameLit);
          break;
        case PUSH:
          done = addPush(tek.regS1);
          break;
        case POP:
          done = addPop(tek.regD);
          break;
        case XCHG:
          done = addXchg(tek.regS1, tek.regS2);
          break;
        case ADD:
          done = addAdd(tek.regS1, tek.regS2, tek.regD);
          break;
        case SUB:
          done = addSub(tek.regS1, tek.regS2, tek.regD);
          break;
        case MUL:
          done = addMul(tek.regS1, tek.regS2, tek.regD);
          break;
        case DIV:
          done = addDiv(tek.regS1, tek.regS2, tek.regD);
          break;
        case NOT:
          done = addNot(tek.regS1, tek.regD);
          break;
        case AND:
          done = addAnd(tek.regS1, tek.regS2, tek.regD);
          break;
        case OR:
          done = addOr(tek.regS1, tek.regS2, tek.regD);
          break;
        case XOR:
          done = addXor(tek.regS1, tek.regS2, tek.regD);
          break;
        case SHL:
          done = addShl(tek.regS1, tek.regS2, tek.regD);
          break;
        case SHR:
          done = addShr(tek.regS1, tek.regS2, tek.regD);
          break;
        case LD:
          done = addLd(tek.adr, tek.regD, tek.regS1, tek.nameSymb, tek.nameLit);
          break;
        case ST:
          done = addSt(tek.adr, tek.regD, tek.regS1, tek.nameSymb, tek.nameLit);
          break;
        case CSRRD:
          done = addCsrrd(tek.regS1, tek.regD);
          break;
        case CSRWR:
          done = addCsrwr(tek.regS1, tek.regD);
          break;
        case NOP:
        default:
          std::cout << "INSTRUCTION NOT RECOGNIZED" << std::endl;
          break;
      }
      if(!done) {
        cout << message << ": " << "\n\tERROR ON INSTRUCTION " << lineCounter << endl;
        return;
      }
    } else if (tek.dir != NDI) {
      switch(tek.dir) {
        case GLOBAL:
          done = addGlobalSymbol(tek.listSym);
          break;
        case EXTERN:
          done = addExternSymbol(tek.listSym);
          break;
        case SECTION:
          done = addSection(tek.nameSymb);
          break;
        case WORD:
          if(currentSection == "") {
            done = false;
            message = "WORD DIRECTIVE CAN'T BE USED OUTSIDE OF THE SECTION";
            break;
          }
          done = addWordSymbol(tek.listSym);
          break;
        case SKIP:
          if(currentSection == "") {
            done = false;
            message = "SKIP DIRECTIVE CAN'T BE USED OUTSIDE OF THE SECTION";
            break;
          }
          done = addSkip(tek.nameLit);
          break;
        case ASCII:
          addAscii(tek.nameLit);
          break;
        case EQU:
          break;
        case END:
          backpatch = true;
          done = doBackpatch();
          printToTxtFile();
          printToBinaryFile();
          break;
        case LABEL:
          if(currentSection == "") {
            done = false;
            message = "LABEL CAN'T BE DEFINED OUTSIDE OF THE SECTION";
            break;
          }
          done = addLabel(tek.nameSymb);
          break;
        case COMMENT:
          break;
        case NDI:
        default:
          std::cout << "DIRECTIVE NOT RECOGNIZED" << std::endl;
          break;
      } 
      if(!done) {
        if(!backpatch) cout << message << ": " << "\n\tERROR ON INSTRUCTION " << lineCounter << endl;
        else cout << message;
        return;
      }
    }
    lineCounter = lineCounter + 1;
  }
}

bool Assembler::doBackpatch() {
  bool ret = sectionTable.doBackpatch(symbolTable);
  if(ret == false) {
    message = ""; 
    return false;
  }
  return true;
}

void Assembler::printToTxtFile() {
  string txtFileName = fileName.substr(0, fileName.length() - 2);
  txtFileName += ".txt";
  //printing out section table
  std::ofstream outFile(txtFileName);
  outFile << sectionTable;

  //printing out symbol table
  outFile << symbolTable;

  //printing out sections
  sectionTable.printOutData(outFile);

  //printing relocation tables
  outFile << "================Realocation tables================\n";
  for (int i = 0; i < sectionTable.sectionTableSize(); i++) {
    outFile << sectionTable.getSectionTable(i).realocationTable;
  }
  outFile.close();
}

void Assembler::printToBinaryFile() {

  ofstream binaryFile(fileName, ios::binary | ios::out);
  
  //printing out section table
  binaryFile <<= sectionTable;

  //printing out symbol table
  binaryFile <<= symbolTable;

  //printing out realocation tables
  int sectionTableSize = sectionTable.sectionTableSize();
  binaryFile.write((char*)(&sectionTableSize), sizeof(sectionTableSize));
  for (int i = 0; i < sectionTable.sectionTableSize(); i++) {
    binaryFile <<= sectionTable.getSectionTable(i).realocationTable;
  }
  
  //printing out data of all sections
  sectionTable.printOutDataBinary(binaryFile);
  
  binaryFile.close();
}