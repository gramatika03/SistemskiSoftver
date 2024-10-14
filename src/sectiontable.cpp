#include "../inc/sectiontable.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

SectionTable::SectionTableElem SectionTable::defaultSection = SectionTable::SectionTableElem();

long literalToValue(string literal) {
  long literalValue = 0;
  if(literal.size() > 2 && literal.at(0) == '0' && literal.at(1) == 'x') literalValue = stol(literal, nullptr, 16);
  else literalValue = stol(literal, nullptr, 10);
  return literalValue;
}

void SectionTable::addNewSection(string name) {
  SectionTableElem newElem = SectionTableElem(sectionOrder, name);
  table.push_back(newElem);
  sectionOrder++;
}

void SectionTable::addSectionInput(string name, int start, int size) {
  SectionTableElem newElem = SectionTableElem(sectionOrder, name, start, size);
  table.push_back(newElem);
  sectionOrder++;
}


SectionTable::SectionTableElem &SectionTable::operator[](string name) {
  for(int i = 0; i < table.size(); i++) {
    if(table.at(i).name == name) return table.at(i);
  }
  return defaultSection;
}

void SectionTable::addToBackpatch(string section, int line, string symbol, string literal, bool isGlobal)
{
  operator[](section).backpatch.push_back(BackpatchElem(section, line, symbol, literal, isGlobal));
}

bool SectionTable::replaceDisplacement(SectionTableElem& sec, int line, int order) {
  vector<char>& instructions = sec.instructions;
  int displacement = instructions.size() - line + order * 4;
  if(displacement > (1UL << 12)) {
    cout << "DISPLACEMENT TO STRING POOL LARGER THAN 12b";
    return false;
  }
  char byte1 = (displacement >> 8) & 0xF;
  char byte0 = (displacement) & 0xFF;
  instructions[line - 4] = byte0;
  instructions[line - 3] = (instructions[line - 3] & 0xF0) | byte1;
  return true;
}

bool SectionTable::replaceInstruction(SectionTableElem& sec, int symbolValue, long line) {
  vector<char>& instructions = sec.instructions;
  if(!((instructions[line - 1] & 0b00100000) == 0b00100000 || (instructions[line - 1] & 0b00110000) == 0b00110000)) return false;
  int displacement = symbolValue - line;
  char byte0; char byte1; char byte2; char byte3;
  if((instructions[line - 1] & 0b00100000) == 0b00100000) {
    byte3 = 0b00100000;
    byte2 = 0b11110000;
    byte1 = 0b00000000 | ((displacement >> 8) & 0xF);
    byte0 = displacement & 0xFF; 
  }
  if((instructions[line - 1] & 0b00110000) == 0b00110000) {
    byte3 = instructions[line - 1] & 0b11110111;
    byte2 = instructions[line - 2] | 0b11110000;
    byte1 = (instructions[line - 3] & 0xF0) | ((displacement >> 8) & 0xF);
    byte0 = displacement & 0xFF; 
  }
  instructions[line - 4] = byte0;
  instructions[line - 3] = byte1;
  instructions[line - 2] = byte2;
  instructions[line - 1] = byte3;
  return true;
}

void SectionTable::addToRealocationTable(SectionTableElem& sec, int offset, string symbolName, int symbol, int addend) {
  sec.realocationTable.addNewElem(offset, symbolName, symbol, addend);
}

bool SectionTable::doBackpatch(SymbolTable& symbolTable) {
  for (int i = 0; i < table.size(); i++) {
    SectionTableElem& currSection = table.at(i);
    vector<BackpatchElem>& backpatch = currSection.backpatch;
    for(int j = 0; j < backpatch.size(); j++) {
      BackpatchElem currBackpatch = backpatch.at(j);
      string currLiteral = currBackpatch.literal;
      string currSymbol = currBackpatch.simbol;
      if(currLiteral != "") {
        long literalValue = literalToValue(currLiteral);
        if(literalValue >= (1UL << 32)) {
          cout << "LITERAL VALUE CAN'T BE LARGER THAN 32b";
          return false;
        }
        int literalToPut = literalValue & 0xFFFFFFFF;
        if(currSection.stringPool.find(currLiteral) == currSection.stringPool.end()) {
          currSection.stringPool[currLiteral] = literalToPut;
          currSection.stringOrder[currSection.stringPoolOrder] = currLiteral;
          currSection.stringNameOrder[currLiteral] = currSection.stringPoolOrder;
          currSection.stringPoolOrder += 1;
        }
        replaceDisplacement(currSection, currBackpatch.line, currSection.stringNameOrder[currLiteral]);
      } else if (currSymbol != "") {
        if(symbolTable[currSymbol].name == "") {
          cout << "SYMBOL " << currSymbol << " DOESN'T EXIST IN SYMBOL TABLE";
          return false;
        }
        bool defined = false;
        if(symbolTable[currSymbol].isDefined && symbolTable[currSymbol].ndx == symbolTable[currSection.name].num) {
          defined = replaceInstruction(currSection, symbolTable[currSymbol].value, currBackpatch.line);
        } 
        if(!defined && currSection.stringPool.find(currSymbol) == currSection.stringPool.end()) {
          currSection.stringPool[currSymbol] = symbolTable[currSymbol].value;
          currSection.stringOrder[currSection.stringPoolOrder] = currSymbol;
          currSection.stringNameOrder[currSymbol] = currSection.stringPoolOrder;
          string symbolRelocation = symbolTable.getSymbolForRealocation(currSymbol);
          if(currBackpatch.isGlobal) {
            addToRealocationTable(currSection, currSection.instructions.size() + currSection.stringPoolOrder * 4, symbolRelocation, symbolTable[symbolRelocation].num, 0);
          } else {
            addToRealocationTable(currSection, currSection.instructions.size() + currSection.stringPoolOrder * 4, symbolRelocation, symbolTable[symbolRelocation].num, symbolTable[currSymbol].value);
          }
          currSection.stringPoolOrder += 1;
        }
        if(!defined) {
          replaceDisplacement(currSection, currBackpatch.line, currSection.stringNameOrder[currSymbol]);
        }
      }
    }
  }
  return true;
}
ofstream &operator<<(ofstream &of, const SectionTable &table) {
  of << "Section Table" << endl;
  of << left << setw(20) << "==================================================" << "\n";
  of << left << setw(20) << "Name";
  of << left << setw(20) << "Start address[hex]";
  of << left << setw(20) << "Size[hex]" << endl;
  of << left << setw(20) << "==================================================" << "\n";
  int sizeCounter = 0;
  for(int i = 0; i < table.table.size(); i++) {
      of << left << setw(20) << table.table.at(i).name 
              << hex << left << setw(20) << sizeCounter 
              << hex << left << setw(20) << table.table.at(i).size + table.table.at(i).stringPool.size() * 4 << endl 
              << dec;
      sizeCounter += table.table.at(i).size + table.table.at(i).stringPool.size() * 4;
      of << "--------------------------------------------------\n";
  }
  return of;
}

void SectionTable::printLinkerSection(ofstream &of, const SectionTable &table) {
  of << "Section Table" << endl;
  of << left << setw(20) << "==================================================" << "\n";
  of << left << setw(20) << "Name";
  of << left << setw(20) << "Start address[hex]";
  of << left << setw(20) << "Size[hex]" << endl;
  of << left << setw(20) << "==================================================" << "\n";
  for(int i = 0; i < table.table.size(); i++) {
      of << left << setw(20) << table.table.at(i).name 
              << hex << left << setw(20) << table.table.at(i).start
              << hex << left << setw(20) << table.table.at(i).size<< endl 
              << dec;
      of << "--------------------------------------------------\n";
  }
}

void SectionTable::printOutData(ofstream& of) {
  for(int i = 0; i < table.size(); i++) {
    vector<char> codedInstructions = table.at(i).instructions;
    int counter = 0;
    of << "\n===================Section " << table.at(i).name << "===================\n";
    of << "                   Size: " << table.at(i).size + table.at(i).stringPool.size() << "\n";
    while(!codedInstructions.empty()) {
      if(counter % 8 == 0) of << hex << setfill('0') << setw(8) << counter << ": ";
      char val = codedInstructions.front();
      codedInstructions.erase(codedInstructions.begin());
      of << hex << setw(2) << static_cast<int>(static_cast<unsigned char>(val)) << " ";
      counter++;
      if(counter % 8 == 0) of << endl;
    }
    for(int j = 0; j < table.at(i).stringPool.size(); j++) {
      if(counter % 8 == 0) of << hex << setfill('0') << setw(8) << counter << ": ";
      int value = getStringFromPool(table.at(i), j);
      char byte0 = (value >> 24) & 0xFF;
      char byte1 = (value >> 16) & 0xFF;
      char byte2 = (value >> 8) & 0xFF;
      char byte3 = (value >> 24) & 0xFF;
      of << hex << setw(2) << static_cast<int>(static_cast<unsigned char>(byte0)) << " ";
      counter++;
      if(counter % 8 == 0) of << hex << setfill('0') << setw(8) << counter << ": ";

      of << hex << setw(2) << static_cast<int>(static_cast<unsigned char>(byte1)) << " ";
      counter++;
      if(counter % 8 == 0) of << hex << setfill('0') << setw(8) << counter << ": ";

      of << hex << setw(2) << static_cast<int>(static_cast<unsigned char>(byte2)) << " ";
      counter++;
      if(counter % 8 == 0) of << hex << setfill('0') << setw(8) << counter << ": ";

      of << hex << setw(2) << static_cast<int>(static_cast<unsigned char>(byte3)) << " ";
      counter++;
      if(counter % 8 == 0) of << hex << setfill('0') << setw(8) << counter << ": ";

    }
    while(counter % 8 != 0) {
      of << hex << setw(2) << 0 << " ";
      counter++;
    }
  }
  of << dec << setfill(' ') << endl;
}

ofstream &operator<<=(ofstream &of, const SectionTable &table) {
  int sizeCounter = 0;
  int sectionTableSize = table.table.size();
  of.write((char*)(&sectionTableSize), sizeof(sectionTableSize));
  for(int i = 0; i < sectionTableSize; i++) {
    int stringPoolSize = table.table.at(i).stringPool.size();
    int nameLen = table.table.at(i).name.length();
    of.write((char*)(&nameLen), sizeof(nameLen));
    for(int j = 0; j < table.table.at(i).name.length(); j++) {
      char c = table.table.at(i).name.at(j);
      of.write((char*)(&c), sizeof(c));
    }
    of.write((char*)(&sizeCounter), sizeof(sizeCounter));
    int sectionSize = table.table.at(i).size + stringPoolSize * 4;
    sizeCounter += sectionSize;
    of.write((char*)(&sectionSize), sizeof(sectionSize));
    table.table.at(i).size + stringPoolSize * 4;
  }
  return of;
}

ifstream &operator>>=(ifstream &inputFile, SectionTable &table) {
  int sectionTableSize;
  inputFile.read((char*)(&sectionTableSize), sizeof(sectionTableSize));
  int nameLen;
  string name = "";
  int start;
  int size;
  for(int i = 0; i < sectionTableSize; i++) {  
    nameLen = 0;
    name = "";
    start = size = 0;
    inputFile.read((char*)(&nameLen), sizeof(nameLen));
    for(int j = 0; j < nameLen; j++) {
      char c;
      inputFile.read((char*)(&c), sizeof(c));
      name += c;
    }
    inputFile.read((char*)(&start), sizeof(start));
    inputFile.read((char*)(&size), sizeof(size));
    table.addSectionInput(name, start, size);
  }
  return inputFile;
}

int SectionTable::getStringFromPool(SectionTableElem& section, int i) {
  string name = section.stringOrder[i];
  int value = section.stringPool[name];
  return value;
}

void SectionTable::printOutDataBinary(ofstream &of) {
  for(int i = 0; i < table.size(); i++) {
    vector<char> codedInstructions = table.at(i).instructions;
    int counter = 0;
    while(!codedInstructions.empty()) {
      char val = codedInstructions.front();
      codedInstructions.erase(codedInstructions.begin());
      of.write((char*)(&val), sizeof(val));
    }
    for(int j = 0; j < table.at(i).stringPool.size(); j++) {
      int value = getStringFromPool(table.at(i), j);
      of.write((char*)(&value), sizeof(value));
    }
  }
}

void SectionTable::setRealocationTable(RealocationTable table, int section) {
  this->table.at(section).realocationTable = table;
}

string SectionTable::getSectionName(int i) {
  return getSectionTable(i).name;
}

vector<char> SectionTable::getSectionLines(int i) {
  return getSectionTable(i).instructions;
}

RealocationTable SectionTable::getRealocationTable(int i) {
  return getSectionTable(i).realocationTable;
}