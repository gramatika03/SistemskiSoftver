#ifndef _RELOCATION_TABLE_HPP
#define _RELOCATION_TABLE_HPP

#include <string>
#include <vector>

using namespace std;

class RealocationTable {
private:
  struct RealocationTableElem {
    int offset;
    string symbolName;
    int symbol;
    int addend;

    RealocationTableElem(): offset(0), symbolName(""), symbol(0), addend(0) {}
    RealocationTableElem(int offset, string symbolName, int symbol, int addend):
    offset(offset), symbolName(symbolName), symbol(symbol), addend(addend) {}
  };

  vector<RealocationTableElem> table;
  string sectionName;
public:
  RealocationTable(string name = "");

  void addNewElem(int offset, string symbolName, int symbol, int addend);

  void increaseOffset(int size);

  void combine(RealocationTable addTable);

  int realocationTableSize();

  int getOffset(int i);
  string getSymbolName(int i);
  int getAddend(int i);

  //printing out to a txt file
  friend ofstream& operator<<(ofstream& of, const RealocationTable& table);
  //printing out to a binary file
  friend ofstream& operator<<=(ofstream& of, const RealocationTable& table);

  friend ifstream& operator>>=(ifstream& inputFile, RealocationTable& table);
};

#endif