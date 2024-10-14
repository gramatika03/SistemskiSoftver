#include "../inc/symboltable.hpp"

#include <iomanip>
#include <iostream>
#include <fstream>

SymbolTable::SymbolTable()  {
  SymbolTableElem newElem = SymbolTableElem();
  table.push_back(newElem);
  symbolOrder++;
}

void SymbolTable::addNewElem(int value, int size, SYMBOL_TABLE_TYPE type, SYMBOL_TABLE_BIND bind, int ndx, string name, bool isExtern, bool isDefined, bool isSection) {
  if(ndx == -2) ndx = symbolOrder;
  SymbolTableElem novi = SymbolTableElem(symbolOrder, value, size, type, bind, ndx, name, isExtern, isDefined, isSection);
  table.push_back(novi);
  symbolOrder++;
}

void SymbolTable::addNewElem(int num, int value, int size, SYMBOL_TABLE_TYPE type, SYMBOL_TABLE_BIND bind, int ndx, string name, bool isExtern, bool isDefined, bool isSection) {
  SymbolTableElem novi = SymbolTableElem(num, value, size, type, bind, ndx, name, isExtern, isDefined, isSection);
  table.push_back(novi);
}

SymbolTable::SymbolTableElem &SymbolTable::operator[](string name) {
  for(int i = 0; i < table.size(); i++) {
    if (table.at(i).name == name) {
      return table.at(i);
    }
  }
  return table.at(0);
}

string SymbolTable::getSymbolForRealocation(string symbolName) {
  if(operator[](symbolName).bind == GLOBAL) return symbolName;
  int section = operator[](symbolName).ndx;
  for (int i = 0; i < table.size(); i++) {
    if(table.at(i).num == section) return table.at(i).name;
  }
  return "";
}

string SymbolTable::getType(bool isSection) {
  if(isSection) return "SCTN";
  else return "NOTYP";
}

string SymbolTable::getBind(SYMBOL_TABLE_BIND bind) {
  if(bind == LOCAL) return "LOCAL";
  else return "GLOBAL";
}

string SymbolTable::getNdx(bool isExtern, bool isEqu, int val) {
  if(isExtern) return "UND";
  else if(isEqu) return "*ABS*";
  else if(val == -1) return "UND";
  else return to_string(val);
}

ofstream &operator<<(ofstream &of, const SymbolTable& table) {
  of << "\nSymbol table" << endl;
  of << left << setw(20) << "================================================================================================================================" << "\n";
  of << left << setw(20) << "Num";
  of << left << setw(20) << "Value[hex]";
  of << left << setw(20) << "Size";
  of << left << setw(20) << "Type";
  of << left << setw(20) << "Bind";
  of << left << setw(20) << "Ndx";
  of << left << setw(20) << "Name" << endl;
  of << "================================================================================================================================" << "\n";
  for(int i = 0; i < table.table.size(); i++) {
      of << left << setw(20) << table.table.at(i).num
              << left << setw(20) << hex << table.table.at(i).value << dec
              << left << setw(20) << table.table.at(i).size
              << left << setw(20) << SymbolTable::getType(table.table.at(i).isSection)
              << left << setw(20) << SymbolTable::getBind(table.table.at(i).bind)
              << left << setw(20) << SymbolTable::getNdx(table.table.at(i).isExtern, false, table.table.at(i).ndx)
              << left << table.table.at(i).name << endl;
      of << "--------------------------------------------------------------------------------------------------------------------------------" << "\n";
  }
  of << dec << setw(0) << setfill(' ') << right;
  return of;
}

ofstream &operator<<=(ofstream &of, const SymbolTable &table) {
  int symbolTableSize = table.table.size();
  int counter = 0;
  for(int i = 0; i < symbolTableSize; i++) {
    if(table.table.at(i).bind == SymbolTable::GLOBAL || table.table.at(i).type == SymbolTable::SCTN)
      counter++;
  }
  of.write((char*)(&counter), sizeof(counter));
  for(int i = 0; i < table.table.size(); i++) {
    if(table.table.at(i).bind != SymbolTable::GLOBAL && table.table.at(i).type != SymbolTable::SCTN) 
      continue;
    int num = table.table.at(i).num;
    of.write((char*)(&num), sizeof(num));
    int value = table.table.at(i).value;
    of.write((char*)(&value), sizeof(value));
    int size = table.table.at(i).size;
    of.write((char*)(&size), sizeof(size));
    int type = table.table.at(i).isSection?1:0;
    of.write((char*)(&type), sizeof(type));
    int bind = table.table.at(i).bind;
    of.write((char*)(&bind), sizeof(bind));
    int ndx = table.table.at(i).ndx;
    of.write((char*)(&ndx), sizeof(ndx));
    int nameLen = table.table.at(i).name.length();
    of.write((char*)(&nameLen), sizeof(nameLen));
    string name = table.table.at(i).name;
    for(int i = 0; i < nameLen; i++) {
      char c = name.at(i);
      of.write((char*)(&c), sizeof(c));
    }
  }
  return of;
}

ifstream& operator>>=(ifstream& inputFile, SymbolTable& table) {
  int symbolTableSize = 0;
  inputFile.read((char*)(&symbolTableSize), sizeof(symbolTableSize));
  for(int i = 0; i < symbolTableSize; i++) {
    int num = 0;
    inputFile.read((char*)(&num), sizeof(num));
    int value = 0;
    inputFile.read((char*)(&value), sizeof(value));
    int size = 0;
    inputFile.read((char*)(&size), sizeof(size));
    int type = 0;
    inputFile.read((char*)(&type), sizeof(type));
    int bind = 0;
    inputFile.read((char*)(&bind), sizeof(bind));
    int ndx = 0;
    inputFile.read((char*)(&ndx), sizeof(ndx));
    int nameLen = 0;
    inputFile.read((char*)(&nameLen), sizeof(nameLen));
    string name = "";
    for(int i = 0; i < nameLen; i++) {
      char c;
      inputFile.read((char*)(&c), sizeof(c));
      name += c;
    }
    if(name != "") table.addNewElem(num, value, size, (SymbolTable::SYMBOL_TABLE_TYPE)type, (SymbolTable::SYMBOL_TABLE_BIND)bind, ndx, name, false, false, type);
  }
  return inputFile;
}

int SymbolTable::symbolTableSize() {
  return table.size();
}

void SymbolTable::increaseValue(string sectionName, int increase) {
  int sectionNum = -1;
  for(int i = 0; i < table.size(); i++) {
    if(table.at(i).name == sectionName) {
      sectionNum = table.at(i).num;
      break;
    }
  }
  if(sectionNum == -1) {
    cout << "Section with name " << sectionName << " not recognized";
  }
  for(int i = 0; i < table.size(); i++) {
    if(table.at(i).ndx == sectionNum) {
      table.at(i).value += increase;
    }
  }
}

string SymbolTable::getSymbolName(int i) {
  return table.at(i).name;
}

int SymbolTable::getSymbolValue(int i) {
  return table.at(i).value;
}

int SymbolTable::getSymbolNdx(int i) {
  return table.at(i).ndx;
}
  
int SymbolTable::getSymbolType(int i) {
  return table.at(i).type;
}

string SymbolTable::getSectionName(int i) {
  int ndx = table.at(i).ndx;
  if(ndx == -1) return "";
  for(int j = 0; j < table.size(); j++) {
    if(table.at(j).num == ndx) return table.at(j).name;
  }
  return "";
}

void SymbolTable::setDefinition(int num, int value, int ndx) {
  table.at(num).value = value;
  table.at(num).ndx = ndx;
}
