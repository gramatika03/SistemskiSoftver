#include "../inc/realocationtable.hpp"

#include <iomanip>
#include <iostream>
#include <fstream>

using namespace std;

RealocationTable::RealocationTable(string name) {
  sectionName = name;
}

void RealocationTable::addNewElem(int offset, string symbolName, int symbol, int addend) {
  RealocationTableElem newElem = RealocationTableElem(offset, symbolName, symbol, addend);
  table.push_back(newElem);
}

ofstream &operator<<(ofstream &of, const RealocationTable& table) {
  of << "Section " << table.sectionName << endl;
  of << "==============================================\n";
  of << left << setw(20) << "Offset";
  of << left << setw(20) << "Symbol";
  of << left << setw(20) << "Addend" << endl;
  of << "==============================================\n";
  for(int i = 0; i < table.table.size(); i++) {
    of << left << setw(20) << table.table.at(i).offset;
    of << left << setw(20) << table.table.at(i).symbolName;
    of << left << setw(20) << table.table.at(i).addend << endl;
    of << "----------------------------------------------\n";
  }
  of << endl;
  return of;
}

ofstream &operator<<=(ofstream &of, const RealocationTable &table) {
  int tableSize = table.table.size();
  of.write((char*)(&tableSize), sizeof(tableSize));
  for(int i = 0; i < table.table.size(); i++) {
    int offset = table.table.at(i).offset;
    int symbolNameLen = table.table.at(i).symbolName.length();
    string symbolName = table.table.at(i).symbolName;
    int addend = table.table.at(i).addend;
    of.write((char*)(&offset), sizeof(offset));
    of.write((char*)(&symbolNameLen), sizeof(symbolNameLen));
    for(int i = 0; i < symbolNameLen; i++) {
      char c = symbolName.at(i);
      of.write((char*)(&c), sizeof(c));
    }
    of.write((char*)(&addend), sizeof(addend));
  }
  return of;
}

ifstream& operator>>=(ifstream& inputFile, RealocationTable& table) {
  int tableSize = 0;
  inputFile.read((char*)(&tableSize), sizeof(tableSize));
  for(int i = 0; i < tableSize; i++) {
    int offset = 0;
    int symbolNameLen = 0;
    string symbolName = "";
    int addend = 0;
    inputFile.read((char*)(&offset), sizeof(offset));
    inputFile.read((char*)(&symbolNameLen), sizeof(symbolNameLen));
    for(int i = 0; i < symbolNameLen; i++) {
      char c;
      inputFile.read((char*)(&c), sizeof(c));
      symbolName += c;
    }
    inputFile.read((char*)(&addend), sizeof(addend));
    table.addNewElem(offset, symbolName, 0, addend);
  }
  return inputFile;
}

void RealocationTable::increaseOffset(int size) {
  for(int i = 0; i < table.size(); i++) {
    table.at(i).offset += size;
  }
}

void RealocationTable::combine(RealocationTable addTable) {
  for(int i = 0; i < addTable.table.size(); i++) {
    RealocationTableElem addingElem = addTable.table.at(i);
    table.push_back(addingElem);
  }
}

int RealocationTable::realocationTableSize() {
  return table.size();
}

int RealocationTable::getOffset(int i) {
  return table.at(i).offset;
}

string RealocationTable::getSymbolName(int i) {
  return table.at(i).symbolName;
}

int RealocationTable::getAddend(int i) {
  return table.at(i).addend;
}