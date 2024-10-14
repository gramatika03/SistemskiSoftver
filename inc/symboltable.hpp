#ifndef _SYMBOL_TABLE_HPP
#define _SYMBOL_TABLE_HPP

#include <string>
#include <vector>

using namespace std;

class SymbolTable {
public:
  enum SYMBOL_TABLE_TYPE{NOTYP, SCTN};
  enum SYMBOL_TABLE_BIND{LOCAL, GLOBAL};
private:
  struct SymbolTableElem {
    int num;
    int value;
    int size;
    SYMBOL_TABLE_TYPE type;
    SYMBOL_TABLE_BIND bind;
    int ndx;
    string name;
    bool isExtern;
    bool isDefined;
    bool isSection;

    SymbolTableElem():num(0), value(0), size(0), type(NOTYP), bind(LOCAL), ndx(-1), name(""), isExtern(0), isDefined(0), isSection(0) {}
    SymbolTableElem(int num, int value, int size, SYMBOL_TABLE_TYPE type, SYMBOL_TABLE_BIND bind, int ndx, string name, bool isExtern, bool isDefined, bool isSection):
    num(num), value(value), size(size), type(type), bind(bind), ndx(ndx), name(name), isExtern(isExtern), isDefined(isDefined), isSection(isSection) {}
  };

  vector<SymbolTableElem> table;
  int symbolOrder = 0;
public:
  SymbolTable();

  void addNewElem(int value, int size, SYMBOL_TABLE_TYPE type, SYMBOL_TABLE_BIND bind, int ndx, string name, bool isExtern, bool isDefined, bool isSection);
  void addNewElem(int num, int value, int size, SYMBOL_TABLE_TYPE type, SYMBOL_TABLE_BIND bind, int ndx, string name, bool isExtern, bool isDefined, bool isSection);  
  string getSymbolForRealocation(string symbolName);

  int symbolTableSize();

  string getSymbolName(int i);
  int getSymbolValue(int i);
  int getSymbolNdx(int i);
  int getSymbolType(int i);
  string getSectionName(int i);

  void setDefinition(int num, int value, int ndx);

  void increaseValue(string sectionName, int increase);

  static string getType(bool isSection);
  static string getBind(SYMBOL_TABLE_BIND bind);
  static string getNdx(bool isExtern, bool isEqu, int val);

  SymbolTableElem& operator[](string name);
  //printing out to a txt file
  friend ofstream& operator<<(ofstream& of, const SymbolTable& table);
  //printing out to a binary file
  friend ofstream& operator<<=(ofstream& of, const SymbolTable& table);

  friend ifstream& operator>>=(ifstream& inputFile, SymbolTable& table);
};

#endif