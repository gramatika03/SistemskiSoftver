#ifndef _EMULATOR_HPP
#define _EMULATOR_HPP

#include <vector>
#include <string>
#include <termios.h>

using namespace std;

class Emulator {
private:
  vector<vector<char>> lines;
  vector<int> sectionSize;
  vector<int> startAddress;

  string inputFile;

  static long MEMORY_SIZE;

  void* memory;

  void writeByte(char byte, unsigned int address);
  void writeUnsignedInt(unsigned int value, unsigned int address);
  void writeInt(int value, unsigned int address);

  char readByte(unsigned int address);
  unsigned int readUnsignedInt(unsigned int address);
  int readInt(unsigned int address);

  int getNextInstruction();

  void readData();
  bool fillMemory();
  void emptyMemory();
  void executeFile();

  void setInitialTerminalState();
  void setTerminalState();
  void checkForTerminalInterrupt();

  unsigned int gp_registers[16];
  unsigned int csr_registers[3];

  enum GP_REGISTERS {R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, SP, PC};
  enum CSR_REGISTERS {STATUS, HANDLER, CAUSE};

  enum INSTRUCTION {
    HALT = 0b000,
    INT = 0b0001,
    CALL = 0b0010,
    JMP = 0b0011,
    XCHG = 0b0100,
    ARITHMETIC = 0b0101,
    LOGIC = 0b0110,
    SHIFTING = 0b0111,
    STORING = 0b1000,
    LOADING = 0b1001
  };

  enum CALL_TYPE {
    FROM_INSTRUCTION = 0b0000,
    FROM_POOL = 0b0001
  };

  enum ARITHMETIC_TYPE {
    ADD = 0b0000,
    SUB = 0b0001,
    MUL = 0b0010,
    DIV = 0b0011
  };

  enum LOGIC_TYPE {
    NOT = 0b0000,
    AND = 0b0001,
    OR = 0b0010,
    XOR = 0b0011
  };

  enum SHIFT_TYPE {
    SHL = 0b0000,
    SHR = 0b0001
  };

  enum JMP_TYPE {
    JUMP_INSTR = 0b0000,
    JUMP_POOL = 0b1000,
    BEQ_INSTR = 0b0001,
    BEQ_POOL = 0b1001,
    BNE_INSTR = 0b0010,
    BNE_POOL = 0b1010,
    BGT_INSTR = 0b0011,
    BGT_POOL = 0b1011
  };

  enum STORE_TYPE {
    PUSH = 0b0001,
    MEM_DIR_STORE = 0b0000,
    MEM_IND_STORE = 0b0010
  };

  enum LOAD_TYPE {
    POP = 0b0011,
    POP_CSR = 0b0110,
    CSRRD = 0b0000,
    CSRWR = 0b0100,
    MEM_DIR_LOAD = 0b0001,
    MEM_IND_LOAD = 0b0010
  };

  void executeTerminalInterrupt();

  void executeHalt();
  void executeInt();
  void executePush(int value);
  void executeCall(int mode, int regA, int regB, int regC, int disp);
  void executeXchg(int regB, int regC);
  void executeArithmetic(int mode, int regA, int regB, int regC, int disp);
  void executeLogic(int mode, int regA, int regB, int regC, int disp);
  void executeShifting(int mode, int regA, int regB, int regC, int disp);
  void executeJmp(int mdoe, int regA, int regB, int regC, int disp);
  void executeLoading(int mode, int regA, int regB, int regC, int disp);
  void executeStoring(int mode, int regA, int regB, int regC, int disp);

  void printOutRegisters();

  struct termios originalTermios, newTermios;

  static int TERM_IN;
  static int TERM_OUT;
  static int TERMINAL_CAUSE;

  int counter = 0;

public:

  void emulate(string inputFile);

};

#endif