#include "../inc/emulator.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sys/mman.h> 
#include <unistd.h> 
#include <termios.h>

using namespace std;

int Emulator::TERM_IN = 0xFFFFFF04;
int Emulator::TERM_OUT = 0xFFFFFF00;
int Emulator::TERMINAL_CAUSE = 3;

long Emulator::MEMORY_SIZE = (1UL << 32);

void Emulator::writeByte(char byte, unsigned int address) {
  if(address == TERM_OUT) {
    cout << (char)byte;
  }
  char* memoryAddress = (char*)(memory) + address;
  *memoryAddress = byte;
}

void Emulator::writeUnsignedInt(unsigned int value, unsigned int address) {
  unsigned int* memoryAddress = (unsigned int*)((char*)(memory) + address);
  *memoryAddress = value;
}

void Emulator::writeInt(int value, unsigned int address) {
  if(address == TERM_OUT) {
    cout << (char)value;
  }
  int* memoryAddress = (int*)((char*)(memory) + address);
  *memoryAddress = value;
}

char Emulator::readByte(unsigned int address) {
  char* memoryAddress = (char*)(memory) + address;
  return *memoryAddress;
}

unsigned int Emulator::readUnsignedInt(unsigned int address) {
  unsigned int* memoryAddress = (unsigned int*)((char*)(memory) + address);
  return *memoryAddress;
}

int Emulator::readInt(unsigned int address) {
  int* memoryAddress = (int*)((char*)(memory) + address);
  return *memoryAddress;
}

void Emulator::readData() {
  ifstream in(inputFile);
  if(!in.is_open()) {
    cout << "FILE " << inputFile << " DOESN'T EXIST" << endl;
    return;
  }
  int numberOfSections;
  in.read((char*)(&numberOfSections), sizeof(numberOfSections));
  for(int i = 0; i < numberOfSections; i++) {
    int sectionStart;
    in.read((char*)(&sectionStart), sizeof(sectionStart));
    int sectionSize;
    in.read((char*)(&sectionSize), sizeof(sectionSize));
    vector<char> lines;
    for(int j = 0; j < sectionSize; j++) {
      char byte;
      in.read((char*)(&byte), sizeof(byte));
      lines.push_back(byte);
    }
    this->lines.push_back(lines);
    this->sectionSize.push_back(sectionSize);
    this->startAddress.push_back(sectionStart);
  }
}

bool Emulator::fillMemory() {
  memory = mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if(memory == MAP_FAILED) {
    cout << "MAPPING MEMORY FAILED" << endl;
    return false;
  } 
  for(int i = 0; i < startAddress.size(); i++) {
    unsigned int startAddress = (unsigned int)this->startAddress.at(i);
    unsigned int size = (unsigned int)sectionSize.at(i);
    for(int j = 0; j < size; j++) {
      writeByte(lines.at(i).at(j), startAddress + j);
    }
  }
  return true;
}

int Emulator::getNextInstruction() {
  char byte0 = *((char*)memory + gp_registers[PC]);
  gp_registers[PC] = gp_registers[PC] + 1;
  char byte1 = *((char*)memory + gp_registers[PC]);
  gp_registers[PC] = gp_registers[PC] + 1;
  char byte2 = *((char*)memory + gp_registers[PC]);
  gp_registers[PC] = gp_registers[PC] + 1;
  char byte3 = *((char*)memory + gp_registers[PC]);
  gp_registers[PC] = gp_registers[PC] + 1;

  return (byte3 << 24 & 0xFF000000) | 
          (byte2 << 16 & 0x00FF0000) | 
          (byte1 << 8 & 0x0000FF00) | 
          (byte0 & 0x000000FF);
}

void Emulator::executeHalt() {
  
}

void Emulator::executeInt() {
  executePush(csr_registers[STATUS]);
  executePush(gp_registers[PC]);
  csr_registers[CAUSE] = 4;
  csr_registers[STATUS] = csr_registers[STATUS] & (~0x1);
  gp_registers[PC] = csr_registers[HANDLER];
}

void Emulator::executePush(int value) {
  gp_registers[SP] -= 4;
  writeInt(value, gp_registers[SP]);
}

void Emulator::executeCall(int mode, int regA, int regB, int regC, int disp) {
  executePush(gp_registers[PC]);
  if(mode == FROM_INSTRUCTION) {
    gp_registers[PC] = gp_registers[regA] + gp_registers[regB] + disp;
  } else if(mode == FROM_POOL) {
    unsigned int address = readUnsignedInt(gp_registers[regA] + gp_registers[regB] + disp);
    gp_registers[PC] = address;
  }
}

void Emulator::executeXchg(int regB, int regC) {
  unsigned int oldB = gp_registers[regB];
  gp_registers[regB] = gp_registers[regC];
  gp_registers[regC] = oldB;
}

void Emulator::executeArithmetic(int mode, int regA, int regB, int regC, int disp) {
  switch(mode) {
    case ADD:
      gp_registers[regA] = gp_registers[regB] + gp_registers[regC];
      break;
    case SUB:
      gp_registers[regA] = gp_registers[regB] - gp_registers[regC];
      break;
    case MUL:
      gp_registers[regA] = gp_registers[regB] * gp_registers[regC];
      break;
    case DIV:
      gp_registers[regA] = gp_registers[regB] / gp_registers[regC];
      break;
  }
}

void Emulator::executeLogic(int mode, int regA, int regB, int regC, int disp) {
  switch (mode) {
    case NOT:
      gp_registers[regA] = ~gp_registers[regB];
      break;
    case AND:
      gp_registers[regA] = gp_registers[regB] & gp_registers[regC];
      break;
    case OR:
      gp_registers[regA] = gp_registers[regB] | gp_registers[regC];
      break;
    case XOR:
      gp_registers[regA] = gp_registers[regB] ^ gp_registers[regC];
      break;
  
  }
}

void Emulator::executeShifting(int mode, int regA, int regB, int regC, int disp) {
  switch(mode) {
    case SHL:
      gp_registers[regA] = gp_registers[regB] << gp_registers[regC];
      break;
    case SHR:
      gp_registers[regA] = gp_registers[regB] >> gp_registers[regC];
      break;
  }
}

void Emulator::executeJmp(int mode, int regA, int regB, int regC, int disp) {
  unsigned int address = 0;
  switch(mode) {
    case JUMP_INSTR:
      gp_registers[PC] = gp_registers[regA] + disp;
      break;
    case JUMP_POOL:
      address = readUnsignedInt(gp_registers[regA] + disp);
      gp_registers[PC] = address;
      break;
    case BEQ_INSTR:
      if(gp_registers[regB] == gp_registers[regC]) {
        gp_registers[PC] = gp_registers[regA] + disp;
      }
      break;
    case BEQ_POOL:
      if(gp_registers[regB] == gp_registers[regC]) {
        address = readUnsignedInt(gp_registers[regA] + disp);
        gp_registers[PC] = address;
      }
      break;
    case BNE_INSTR:
      if(gp_registers[regB] != gp_registers[regC]) {
        gp_registers[PC] = gp_registers[regA] + disp;
      }
      break;
    case BNE_POOL:
      if(gp_registers[regB] != gp_registers[regC]) {
        address = readUnsignedInt(gp_registers[regA] + disp);
        gp_registers[PC] = address;
      }
      break;
    case BGT_INSTR:
      if((int)gp_registers[regB] > (int)gp_registers[regC]) {
        gp_registers[PC] = gp_registers[regA] + disp;
      }
      break;
    case BGT_POOL:
      if((int)gp_registers[regB] > (int)gp_registers[regC]) {
        address = readUnsignedInt(gp_registers[regA] + disp);
        gp_registers[PC] = address;
      }
      break;
  }
}

void Emulator::executeStoring(int mode, int regA, int regB, int regC, int disp) {
  switch(mode) {
    case PUSH:
      gp_registers[regA] = gp_registers[regA] + disp;
      writeInt(gp_registers[regC], gp_registers[regA]);
      break;
    case MEM_DIR_STORE:
      writeInt(gp_registers[regC], gp_registers[regA] + gp_registers[regB] + disp);
      break;
    case MEM_IND_STORE:
      unsigned int address = readUnsignedInt(gp_registers[regA] + gp_registers[regB] + disp);
      writeInt(gp_registers[regC], address);
      break;
  }
}

void Emulator::executeLoading(int mode, int regA, int regB, int regC, int disp) {
  int value = 0;
  switch(mode) {
    case POP:
      value = readInt(gp_registers[regB]);
      gp_registers[regA] = value;
      gp_registers[regB] = gp_registers[regB] + disp;
      break;
    case POP_CSR:
      value = readInt(gp_registers[regB] + gp_registers[regC] + disp);
      csr_registers[regA] = value;
      break;
    case CSRRD:
      gp_registers[regA] = csr_registers[regB];
      break;
    case CSRWR:
      csr_registers[regA] = gp_registers[regB];
      break;
    case MEM_DIR_LOAD:
      gp_registers[regA] = gp_registers[regB] + disp;
      break;
    case MEM_IND_LOAD:
      value = readInt(gp_registers[regB] + gp_registers[regC] + disp);
      gp_registers[regA] = value;
      break;
  }
}

void Emulator::executeTerminalInterrupt() {
  executePush(csr_registers[STATUS]);
  executePush(gp_registers[PC]);
  csr_registers[CAUSE] = 3;
  csr_registers[STATUS] = csr_registers[STATUS] & (~0x1);
  gp_registers[PC] = csr_registers[HANDLER];
}

void Emulator::checkForTerminalInterrupt() {
  char c;
  if (read(STDIN_FILENO, &c, 1) == 1) {
    writeInt((int)c, TERM_IN);
    executeTerminalInterrupt();
  }
}

void Emulator::setInitialTerminalState() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
}


void Emulator::setTerminalState() {
  tcgetattr(STDIN_FILENO, &originalTermios);
  newTermios = originalTermios;
  newTermios.c_lflag &= ~(ICANON | ECHO | ECHONL | IEXTEN);
  newTermios.c_cc[VTIME] = 0;
  newTermios.c_cc[VMIN] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
}


void Emulator::executeFile() {
  gp_registers[PC] = 0x40000000;
  gp_registers[SP] = 0xFFFFFF00;
  bool halt = false;
  while(!halt){
    checkForTerminalInterrupt();
    unsigned int instruction = getNextInstruction();
    if(instruction == 0) break;
    int instructionType = (instruction >> 28) & 0xF;
    int instructionMode = (instruction >> 24) & 0xF;
    int regA = (instruction >> 20) & 0xF;
    int regB = (instruction >> 16) & 0xF;
    int regC = (instruction >> 12) & 0xF;
    int disp = (instruction) & 0xFFF;
    if(disp & 0x800) {
      disp |= 0xFFFFF000;
    }
    switch(instructionType) {
      case HALT:
        executeHalt();
        halt = true;
        break;
      case INT:
        executeInt();
        break;
      case CALL:
        if(instructionMode != FROM_INSTRUCTION && instructionMode != FROM_POOL) {
          cout << "INSTRUCTION MODE IN FUNCTION CALL NOT RECOGNIZED" << endl;
          halt = true;
          break;
        }
        executeCall(instructionMode, regA, regB, regC, disp);
        break;
      case JMP:
        executeJmp(instructionMode, regA, regB, regC, disp);
        break;
      case XCHG:
        executeXchg(regB, regC);
        break;
      case ARITHMETIC:
        executeArithmetic(instructionMode, regA, regB, regC, disp);
        break;
      case LOGIC:
        executeLogic(instructionMode, regA, regB, regC, disp);
        break;
      case SHIFTING:
        executeShifting(instructionMode, regA, regB, regC, disp);
        break;
      case STORING:
        executeStoring(instructionMode, regA, regB, regC, disp);
        break;
      case LOADING:
        executeLoading(instructionMode, regA, regB, regC, disp);
        break;
      default:
        cout << "INSTRUCTION " << hex << instructionType << dec << " NOT RECOGNIZED" << endl;
        break;
    }
  }
}

void Emulator::printOutRegisters() {
  cout << "-----------------------------------------------------------------\n";
  cout << "Emulated processor executed halt instruction" << endl;
  cout << "Emulated processor state:" << endl; 
  for(int i = 0; i < 16; i++) {
    cout << left << "r" << setw(2) << i << right << " = 0x" << setw(8) << setfill('0') << hex << gp_registers[i] << setfill(' ') << dec << " ";
    if((i + 1) % 4 == 0) cout << endl;
  }
}

void Emulator::emptyMemory() {
  munmap(memory, MEMORY_SIZE);
}

void Emulator::emulate(string inputFile) {
  this->inputFile = inputFile;
  for(int i = 0; i < 16; i++) gp_registers[i] = 0;
  for(int i = 0; i < 3; i++) csr_registers[i] = 0;
  
  readData();
  fillMemory();
  setTerminalState();
  executeFile();
  setInitialTerminalState();
  printOutRegisters();
  emptyMemory();
  
}
