#ifndef _LINKER_HPP
#define _LINKER_HPP

#include "../inc/sectiontable.hpp"
#include "../inc/symboltable.hpp"

#include <fstream>
#include <vector>

using namespace std;

class Linker {
private:
  struct CombinedSectionsElem {
    string name;
    int start = 0;
    int size;
    RealocationTable realocationTable;
    vector<char> lines;
    bool placed;

    CombinedSectionsElem(): placed(false), name(""), start(0), size(0), realocationTable(RealocationTable()), lines(vector<char>()) {}
    CombinedSectionsElem(string name, int start, int size, RealocationTable realocationTable, vector<char> lines)
      :placed(false), name(name), start(start), size(size), realocationTable(realocationTable), lines(lines) {}
  };

  struct CombinedSymbolsElem {
    string name;
    bool isSection;
    bool isDefined;
    int value;
    int ndx;
    int type;

    CombinedSymbolsElem(): name(""), isSection(false), isDefined(false), value(0), type(0) {};
    CombinedSymbolsElem(string name, bool isSection, bool isDefined, int value, int ndx, int type):
      name(name), isSection(isSection), isDefined(isDefined), value(value), ndx(ndx), type(type) {}
  };

  vector<SectionTable> sectionTables;
  vector<SymbolTable> symbolTables;
  vector<CombinedSectionsElem> combinedSections;
  vector<CombinedSymbolsElem> combinedSymbols;

  void readSectionTable(ifstream& inputFile, int i);
  void readSymbolTable(ifstream& inputFile, int i);
  void readRelocationTables(ifstream& inputFile, int i);
  void readSectionsData(ifstream& inputFile, int i);

  CombinedSectionsElem& getCombinedSection(string name);

  static CombinedSectionsElem defaultCombinedSection;

  int sectionAdded(string name);
  int getSymbolIndex(string name);
  int getSectionStart(string name);

  int symbolValue(string name);
  vector<string> placedNames;
  vector<int> placedValues;
  vector<string> inputFiles;
  string outputFile;
  bool isHex;
  bool isRelocatable;

  void writeHex();
  void writeToRelocatable();
public:
  Linker(vector<string> inputFiles, string outputFile, bool isHex, bool isRelocatable, vector<string> placedNames, vector<int> placedValues);

  bool loadData(int numOfFiles, vector<string> fileNames);
  void combineSections();
  void setStartOfSections(vector<int> startAddress, vector<string> nameOfSection);
  bool checkForIntersection();
  bool combineTables();
  bool combineTablesRealocation();
  bool fillRealocations();

  void doLinking();
};

#endif