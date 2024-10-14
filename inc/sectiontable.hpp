#ifndef _SECTION_TABLE_HPP
#define _SECTION_TABLE_HPP

#include <string>
#include <vector>
#include <map>

#include "realocationtable.hpp"
#include "symboltable.hpp"

using namespace std;

class SectionTable{
private:
  struct BackpatchElem {
    string section;
    int line;
    string literal;
    string simbol;
    bool isGlobal;

    BackpatchElem(): section(""), line(0), literal(""), simbol("") {}
    BackpatchElem(string section, int line, string simbol, string literal, bool isGlobal): section(section), line(line), literal(literal), simbol(simbol), isGlobal(isGlobal) {}
  };

  struct SectionTableElem {
    string name;
    vector<char> instructions;
    int size = 0;
    int order;
    vector<BackpatchElem> backpatch;
    map<string, int> stringPool;
    map<int, string> stringOrder;
    map<string, int> stringNameOrder;
    int stringPoolOrder = 0;
    RealocationTable realocationTable;
    int start;
    
    SectionTableElem(): start(0), realocationTable(RealocationTable()), stringNameOrder(map<string, int>()), stringPoolOrder(0), stringOrder(map<int, string>()), stringPool(map<string, int>()), name(""), instructions(vector<char>()), size(0), order(0), backpatch(vector<BackpatchElem>()) {}
    SectionTableElem(int order, string name): realocationTable(name), name(name), order(order) {}
    SectionTableElem(int order, string name, int start, int size): order(order), name(name), start(start), size(size) {} 
  };

  vector<SectionTableElem> table;
  static SectionTableElem defaultSection;

  int sectionOrder = 0;

  int getStringFromPool(SectionTableElem& table, int i);
public:
  SectionTableElem& getSectionTable(int i) {
    return table.at(i);
  }

  int sectionTableSize() {
    return table.size();
  }

  string getSectionName(int j);
  vector<char> getSectionLines(int j);
  RealocationTable getRealocationTable(int j);

  void addNewSection(string name);
  void addSectionInput(string name, int start, int size);
  void addToBackpatch(string section, int line, string symbol, string literal, bool isGlobal);
  void setRealocationTable(RealocationTable table, int section);

  bool doBackpatch(SymbolTable& symbolTable);
  bool replaceDisplacement(SectionTableElem& sec, int line, int order);
  bool replaceInstruction(SectionTableElem& sec, int symbolValue, long line);
  void addToRealocationTable(SectionTableElem& sec, int offset, string symbolName, int symbol, int addend) ;

  SectionTableElem& operator[](string name);

  friend ofstream& operator<<(ofstream& of, const SectionTable& table);
  friend ofstream& operator<<=(ofstream& of, const SectionTable& table);
  friend ifstream& operator>>=(ifstream& inputFile, SectionTable& table);

  void printLinkerSection(ofstream& of, const SectionTable& table);

  void printOutData(ofstream& of);
  void printOutDataBinary(ofstream& of);
};

#endif