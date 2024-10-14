#include <iostream>
#include <stdio.h>
#include <string>

extern int yyparse();
extern FILE* yyin;
char* outFile;

using namespace std;

int main(int argc, char* argv[]) {
    string outputOption = string(argv[1]);
    if(outputOption != "-o") {
        cout << "OUTPUT FILE NOT SPECIFIED" << endl;
        return 0;
    }

    yyin = fopen(argv[3], "r"); // Open input file
    outFile = argv[2];

    if (!yyin) {
        cout << "INPUT FILE NOT RECOGNIZED" << endl;
        return 1;
    }

    yyparse(); // Start parsing

    fclose(yyin); // Close file
    return 0;
}
