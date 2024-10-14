#include "../inc/emulator.hpp"

#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
  if(argc == 1) {
    cout << "FILE TO BE EMULATED NOT SPECIFIED" << endl;
    return 0;
  } else if (argc > 2) {
    cout << "ONLY ONE FILE CAN BE EMULATED AT ONE TIME" << endl;
  }

  Emulator emulator;
  emulator.emulate(string(argv[1]));
}