#include <iostream>
#include <stdio.h>
#include <string>
#include "../inc/linker.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    bool isHex = false;
    bool isRelocatable = false;
    string outputFile;
    vector<string> inputFiles;
    vector<string> sectionNames;
    vector<int> sectionStart;
    for(int i = 1; i < argc; i++) {
        if(string(argv[i]) == "-hex") isHex = true;
        else if(string(argv[i]) == "-relocatable") isRelocatable = true;
        else if(string(argv[i]).substr(0, 7) == "-place=") {
            string place = string(argv[i]);
            int atPosition = place.find('@');
            string sectionName = place.substr(7, atPosition - 7);
            string sectionPlace = place.substr(atPosition + 3);
            int sectionPlaceValue = stol(sectionPlace, nullptr, 16);
            sectionNames.push_back(sectionName);
            sectionStart.push_back(sectionPlaceValue);
        } else if(string(argv[i]) == "-o") {
            outputFile = string(argv[i + 1]);
            i = i + 1;
        } else {
            inputFiles.push_back(argv[i]);
        }
    }

    if(isHex && isRelocatable) {
        cout << "ONLY ONE OF -hex AND -relocatable OPTIONS CAN BE CHOSEN" << endl;
        return 0;
    }


    Linker linker(inputFiles, outputFile, isHex, isRelocatable, sectionNames, sectionStart);
    linker.doLinking();

    return 0;
}
