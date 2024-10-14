#include "../inc/linker.hpp"

#include <iostream>
#include <iomanip>

using namespace std;

Linker::CombinedSectionsElem Linker::defaultCombinedSection = Linker::CombinedSectionsElem();

Linker::Linker(vector<string> inputFiles, string outputFile, bool isHex, bool isRelocatable, vector<string> placedNames, vector<int> placedValues) {
  this->inputFiles = inputFiles;
  this->outputFile = outputFile;
  this->isHex = isHex;
  this->isRelocatable = isRelocatable;
  this->placedNames = placedNames;
  this->placedValues = placedValues;
}


Linker::CombinedSectionsElem& Linker::getCombinedSection(string name) {
  for(int i = 0; i < combinedSections.size(); i++) {
    CombinedSectionsElem& currSection = combinedSections.at(i);
    if(currSection.name == name) return currSection;
  }
  return defaultCombinedSection;
}

void Linker::readSectionTable(ifstream& inputFile, int i) {
  SectionTable newSectionTable;
  inputFile >>= newSectionTable;
  sectionTables.push_back(newSectionTable);
}

void Linker::readSymbolTable(ifstream& inputFile, int i) {
  SymbolTable newSymbolTable;
  inputFile >>= newSymbolTable;
  symbolTables.push_back(newSymbolTable);
}

void Linker::readRelocationTables(ifstream& inputFile, int i) {
  int numOfRealocationTables;
  inputFile.read((char*)(&numOfRealocationTables), sizeof(numOfRealocationTables));
  for(int j = 0; j < numOfRealocationTables; j++) {
    RealocationTable newRealocationTable;
    inputFile >>= newRealocationTable;
    sectionTables.at(i).setRealocationTable(newRealocationTable, j);
  }
}

void Linker::readSectionsData(ifstream& inputFile, int i) {
    for(int k = 0; k < sectionTables.at(i).sectionTableSize(); k++) {
      int size = sectionTables.at(i).getSectionTable(k).size;
      vector<char> lines;
      for(int l = 0; l < size; l++) {
        char c;
        inputFile.read((char*)(&c), sizeof(c));
        lines.push_back(c);
      }
      sectionTables.at(i).getSectionTable(k).instructions = lines;
    }
  
}

int Linker::sectionAdded(string name) {
  for(int i = 0; i < combinedSections.size(); i++) {
    if(combinedSections.at(i).name == name) return i;
  }
  return -1;
}

bool Linker::loadData(int numOfFiles, vector<string> fileNames) {
  for(int i = 0; i < numOfFiles; i++) {
    ifstream inputFile(fileNames[i], ios::binary);
    if(!inputFile.is_open()) { 
      cout << "FILE " << fileNames.at(i) << " DOESN'T EXIST" << endl;
      return false;
    } 
    readSectionTable(inputFile, i);
    readSymbolTable(inputFile, i);
    readRelocationTables(inputFile, i);
    readSectionsData(inputFile, i);
    inputFile.close();
  }
  return true;
}

int Linker::getSymbolIndex(string name) {
  for(int i = 0; i < combinedSymbols.size(); i++) {
    if(combinedSymbols.at(i).name == name) return i;
  }
  return -1;
}

int Linker::getSectionStart(string name) {
  for(int i = 0; i < combinedSections.size(); i++) {
    if(combinedSections.at(i).name == name) return combinedSections.at(i).start;
  }
  return 0;
}

bool Linker::combineTables() {
  enum SYMBOL_TABLE_TYPE{NOTYP, SCTN};
  for(int i = 0; i < symbolTables.size(); i++) {
    SymbolTable& table = symbolTables.at(i);
    for(int j = 0; j < table.symbolTableSize(); j++) {
      string name = table.getSymbolName(j);
      int value = table.getSymbolValue(j);
      int ndx = table.getSymbolNdx(j);
      int type = table.getSymbolType(j);

      

      bool isSection = ((SYMBOL_TABLE_TYPE)type == SCTN);
      bool isDefined = (ndx != -1);

      int indexInTable = getSymbolIndex(name);
      string sectionName = table.getSectionName(j);

      for(int k = 0; k < combinedSections.size(); k++) {
          if(combinedSections.at(k).name == sectionName) ndx = k;
      }

      if(indexInTable == -1) {
        if(isSection) {
          value = getSectionStart(name);
        }
        CombinedSymbolsElem symbol(name, isSection, isDefined, value, ndx, type);
        combinedSymbols.push_back(symbol);
      } else {
        CombinedSymbolsElem& symbol = combinedSymbols.at(indexInTable);
        if(symbol.isDefined && isDefined && !symbol.isSection) {
          cout << "MULTIPLE DEFINITIONS OF: " << name << endl;
          return false;;
        }
        if(symbol.isSection) continue;
        if(!isDefined) continue;
        symbol.name = name;
        symbol.value = value;
        symbol.type = type;
        symbol.isSection  = isSection;
        symbol.isDefined = isDefined;
        symbol.ndx = ndx;
      }
    }
  }
  if(isHex) {
    for(int i = 0; i < combinedSymbols.size(); i++) {
      if(combinedSymbols.at(i).name != "" && (!combinedSymbols.at(i).isDefined || combinedSymbols.at(i).ndx == -1)) {
        cout << "SYMBOL " << combinedSymbols.at(i).name << " NOT DEFINED" << endl;
        return false;
      }
    }
  }
  return true;
}

void Linker::combineSections() {
  int numOfFiles = sectionTables.size();
  for(int i = 0; i < numOfFiles; i++) {
    SectionTable& sectionTable = sectionTables.at(i);
    for(int j = 0; j < sectionTable.sectionTableSize(); j++) {
      string name = sectionTable.getSectionName(j);
      vector<char> lines = sectionTable.getSectionLines(j);
      RealocationTable realocationTable = sectionTable.getRealocationTable(j);
      int exists = sectionAdded(name);
      if(exists == -1) {
        CombinedSectionsElem newCombinedSectionsElem = CombinedSectionsElem(name, 0, lines.size(), realocationTable, lines);
        combinedSections.push_back(newCombinedSectionsElem);
      } else {
        CombinedSectionsElem& section = combinedSections.at(exists);
        realocationTable.increaseOffset(section.size);
        symbolTables.at(i).increaseValue(name, section.size);
        section.realocationTable.combine(realocationTable);
        for(int k = 0; k < lines.size(); k++) {
          char line = lines.at(k);
          section.lines.push_back(line);
        }
        section.size += lines.size();
      }
    }
  }
}

void Linker::setStartOfSections(vector<int> startAddress, vector<string> nameOfSection) {
  unsigned int lastPlaced = 0;
  for(int i = 0; i < startAddress.size(); i++) {
    string sectionName = nameOfSection.at(i);
    unsigned int sectionStart = startAddress.at(i);
    CombinedSectionsElem& section = getCombinedSection(sectionName);
    if(section.name == "") {
      cout << "Section " << sectionName << " placed at address 0x" << hex << sectionStart << " does not exist" << endl;
      continue;
    }
    section.start = sectionStart;
    section.placed = true;
    if((unsigned int)section.start > lastPlaced) lastPlaced = sectionStart + section.size;
  }

  int size = 0;
  for(int i = 0; i < combinedSections.size(); i++) {
    CombinedSectionsElem& section = combinedSections.at(i);
    if(section.placed) continue;
    unsigned int startAddress = lastPlaced + size;
    section.start = startAddress;
    size = size + section.size;
    section.placed = true;
  }
}

bool Linker::checkForIntersection() {
  for(int i = 0; i < combinedSections.size(); i++) {
    CombinedSectionsElem& section1 = combinedSections.at(i); 
    for(int j = i + 1; j < combinedSections.size(); j++) {
      CombinedSectionsElem& section2 = combinedSections.at(j);
      if(section1.start < section2.start && section1.start + section1.size > section2.start) {
        cout << "SECTIONS: " << section1.name << " AND " << section2.name << " ARE INTERSECTED";
        return true;
      }
    }
  }
  return false;
}

int Linker::symbolValue(string name) {
  for(int i = 0; i < combinedSymbols.size(); i++) {
    CombinedSymbolsElem& symbol = combinedSymbols.at(i);
    if(symbol.name == name) {
      if(symbol.isSection) return symbol.value;
      int ndx = symbol.ndx;
      unsigned int sectionValue = combinedSections.at(ndx).start;
      return sectionValue + symbol.value;
    }
  }
  return 0;
}

bool Linker::fillRealocations() {
  for(int i = 0; i < combinedSections.size(); i++) {
    CombinedSectionsElem& currSection = combinedSections.at(i);
    RealocationTable& realocTable = currSection.realocationTable;
    for(int j = 0; j < realocTable.realocationTableSize(); j++) {
      string symbolName = realocTable.getSymbolName(j);
      int offset = realocTable.getOffset(j);
      int addend = realocTable.getAddend(j);
      int sValue = symbolValue(symbolName);
      sValue += addend;
      char byte3 = sValue >> 24 & 0xFF;
      char byte2 = sValue >> 16 & 0xFF;
      char byte1 = sValue >> 8 & 0xFF;
      char byte0 = sValue & 0xFF;
      vector<char>& lines = currSection.lines;
      lines[offset] = byte0;
      lines[offset + 1] = byte1;
      lines[offset + 2] = byte2;
      lines[offset + 3] = byte3;
    }
  }
  return true;
}

void Linker::writeHex() {
  ofstream outFile(outputFile);
  int numberOfSections = combinedSections.size();
  outFile.write((char*)(&numberOfSections), sizeof(numberOfSections));
  for(int i = 0; i < combinedSections.size(); i++) {
    CombinedSectionsElem& section = combinedSections.at(i);
    unsigned int start = section.start;
    outFile.write((char*)(&start), sizeof(start));
    int size = section.size;
    outFile.write((char*)(&size), sizeof(size));
    for(int j = 0; j < section.lines.size(); j++) {
      char byte = section.lines.at(j);
      outFile.write((char*)(&byte), sizeof(byte));
    }
  }
  outFile.close();

}

void Linker::writeToRelocatable() {
  ofstream of(outputFile);
  int sizeCounter = 0;
  int sectionTableSize = combinedSections.size();
  of.write((char*)(&sectionTableSize), sizeof(sectionTableSize));
  for(int i = 0; i < sectionTableSize; i++) {
    int nameLen = combinedSections.at(i).name.length();
    of.write((char*)(&nameLen), sizeof(nameLen));
    for(int j = 0; j < nameLen; j++) {
      char c = combinedSections.at(i).name.at(j);
      of.write((char*)(&c), sizeof(c));
    }
    of.write((char*)(&sizeCounter), sizeof(sizeCounter));
    int sectionSize = combinedSections.at(i).size;
    sizeCounter += sectionSize;
    of.write((char*)(&sectionSize), sizeof(sectionSize));
  }

  int symbolTableSize = combinedSymbols.size();
  of.write((char*)(&symbolTableSize), sizeof(symbolTableSize));
  for(int i = 0; i < combinedSymbols.size(); i++) {
    int num = i;
    of.write((char*)(&num), sizeof(num));
    int value = combinedSymbols.at(i).value;
    of.write((char*)(&value), sizeof(value));
    int size = 0;
    of.write((char*)(&size), sizeof(size));
    int type = combinedSymbols.at(i).isSection?1:0;
    of.write((char*)(&type), sizeof(type));
    int bind = combinedSymbols.at(i).isSection;
    of.write((char*)(&bind), sizeof(bind));
    int indexInSectionTable = combinedSymbols.at(i).ndx;
    int ndx;
    if(indexInSectionTable >= 0) {
      string sectionName = combinedSections.at(indexInSectionTable).name;
      ndx = getSymbolIndex(sectionName);
    } else {
      ndx = -1;
    }
    of.write((char*)(&ndx), sizeof(ndx));
    int nameLen = combinedSymbols.at(i).name.length();
    of.write((char*)(&nameLen), sizeof(nameLen));
    string name = combinedSymbols.at(i).name;
    for(int i = 0; i < nameLen; i++) {
      char c = name.at(i);
      of.write((char*)(&c), sizeof(c));
    }
  }

  sectionTableSize = combinedSections.size();
  of.write((char*)(&sectionTableSize), sizeof(sectionTableSize));
  
  for (int i = 0; i < sectionTableSize; i++) {
    of <<= combinedSections.at(i).realocationTable;
  }

  for(int i = 0; i < combinedSections.size(); i++) {
    vector<char> codedInstructions = combinedSections.at(i).lines;
    int counter = 0;
    while(!codedInstructions.empty()) {
      char val = codedInstructions.front();
      codedInstructions.erase(codedInstructions.begin());
      of.write((char*)(&val), sizeof(val));
    }
  }
}

void Linker::doLinking() {
  bool dataLoaded = false;
  dataLoaded = loadData(inputFiles.size(), inputFiles);
  if(!dataLoaded) return;
  combineSections();
  if(isHex) {
    setStartOfSections(placedValues, placedNames);
    bool intersected = checkForIntersection();
    if(intersected) return;
    bool madeSymbolTable = combineTables();
    if(!madeSymbolTable) return;
    fillRealocations();
    writeHex();
  } else if(isRelocatable) {
    setStartOfSections(vector<int>(), vector<string>());
    bool madeSymbolTable = combineTables();
    if(!madeSymbolTable) return;
    writeToRelocatable();
  }
}


