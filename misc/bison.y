%{
  #include <stdio.h>
  #include <stdlib.h>
  #include "./inc/assembler.hpp"
  #include <list>

  Assembler* assembler = new Assembler();

  extern int yylex();
  extern int linenum;
  void yyerror(char* s);

  extern char* outFile;

  int listNumber = 0;
  int indir = 0;

  struct Elem {
    char* symbol;
    struct Elem* next;
  };

  std::list<char*> globalList;
  std::list<char*> externList;
  std::list<char*> wordList;
  
  void addSymbol(char* symbol, int listNumber);
%}

%union {
  int num;
  char* str;
}

/* registers */
%token<num> GP_REGISTER CSR_REGISTER

/* general symbols */
%token LS_BRACKET RS_BRACKET COMMA PLUS MINUS SEMICOLON COLON PERCENT DOLLAR

/* operations */
%token HALT INT IRET CALL RET JMP BEQ BNE BGT PUSH POP XCHG ADD SUB MUL DIV NOT AND OR XOR SHL SHR LD ST CSRRD CSRWR

/* directives */
%token GLOBAL EXTERN SECTION WORD SKIP ASCII EQU END

/* symbols */
%token<str> SYMBOL

/* literals */
%token<str> LITERAL

/* label */
%token<str> LABEL

/* new line */
%token ENDL

/* comments */
%token COMMENT

%token ERROR

%token<str> STRING

%%
LINE:|
LINE OPERATIONS|
LINE END_LINE|
LINE COMMENTS|
LINE DIRECTIVES|
LINE LABELS|
LINE SYMBOLS|
LINE LITERALS|
LINE COMMA SYMBOLS|
LINE COMMA LITERALS

OPERATIONS: 
  HALT {
    assembler->addInstruction(Assembler::HALT, -1, -1, -1, 0, 0, Assembler::NAD);
  }|
  INT {
    assembler->addInstruction(Assembler::INT, -1, -1, -1, 0, 0, Assembler::NAD);
  }|
  IRET {
    assembler->addInstruction(Assembler::IRET, -1, -1, -1, 0, 0, Assembler::NAD);
  }|
  CALL LITERAL {
    assembler->addInstruction(Assembler::CALL, -1, -1, -1, 0, $2, Assembler::MEM_DIR);
  }|
  CALL SYMBOL {
    assembler->addInstruction(Assembler::CALL, -1, -1, -1, $2, 0, Assembler::MEM_DIR);
  }|
  RET {
    assembler->addInstruction(Assembler::RET, -1, -1, -1, 0, 0, Assembler::NAD);
  }|
  JMP LITERAL {
    assembler->addInstruction(Assembler::JMP, -1, -1, -1, 0, $2, Assembler::MEM_DIR);
  }|
  JMP SYMBOL {
    assembler->addInstruction(Assembler::JMP, -1, -1, -1, $2, 0, Assembler::MEM_DIR);
  }|
  BEQ GP_REGISTER COMMA GP_REGISTER COMMA LITERAL {
    assembler->addInstruction(Assembler::BEQ, $2, $4, -1, 0, $6, Assembler::MEM_IND);
  }|
  BEQ GP_REGISTER COMMA GP_REGISTER COMMA SYMBOL {
    assembler->addInstruction(Assembler::BEQ, $2, $4, -1, $6, 0, Assembler::MEM_IND);
  }|
  BNE GP_REGISTER COMMA GP_REGISTER COMMA LITERAL {
    assembler->addInstruction(Assembler::BNE, $2, $4, -1, 0, $6, Assembler::MEM_IND);
  }|
  BNE GP_REGISTER COMMA GP_REGISTER COMMA SYMBOL {
    assembler->addInstruction(Assembler::BNE, $2, $4, -1, $6, 0, Assembler::MEM_IND);
  }|
  BGT GP_REGISTER COMMA GP_REGISTER COMMA LITERAL {
    assembler->addInstruction(Assembler::BGT, $2, $4, -1, 0, $6, Assembler::MEM_IND);
  }|
  BGT GP_REGISTER COMMA GP_REGISTER COMMA SYMBOL {
    assembler->addInstruction(Assembler::BGT, $2, $4, -1, $6, 0, Assembler::MEM_IND);
  }|
  PUSH GP_REGISTER {
    assembler->addInstruction(Assembler::PUSH, $2, 0, 0, 0, 0, Assembler::REG_DIR);
  }|
  POP GP_REGISTER {
    assembler->addInstruction(Assembler::POP, 0, 0, $2, 0, 0, Assembler::REG_DIR);
  }|
  XCHG GP_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::XCHG, $2, $4, 0, 0, 0, Assembler::REG_DIR);
  }|
  ADD GP_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::ADD, $4, $2, $4, 0, 0, Assembler::REG_DIR);
  }|
  SUB GP_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::SUB, $4, $2, $4, 0, 0, Assembler::REG_DIR);
  }|
  MUL GP_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::MUL, $4, $2, $4, 0, 0, Assembler::REG_DIR);
  }|
  DIV GP_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::DIV, $4, $2, $4, 0, 0, Assembler::REG_DIR);
  }|
  NOT GP_REGISTER {
    assembler->addInstruction(Assembler::NOT, $2, 0, $2, 0, 0, Assembler::REG_DIR);
  }|
  AND GP_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::AND, $4, $2, $4, 0, 0, Assembler::REG_DIR);
  }|
  OR GP_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::OR, $4, $2, $4, 0, 0, Assembler::REG_DIR);
  }|
  XOR GP_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::XOR, $4, $2, $4, 0, 0, Assembler::REG_DIR);
  }|
  SHL GP_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::SHL, $4, $2, $4, 0, 0, Assembler::REG_DIR);
  }|
  SHR GP_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::SHR, $4, $2, $4, 0, 0, Assembler::REG_DIR);
  }|
  LD DOLLAR LITERAL COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::LD, 0, 0, $5, 0, $3, Assembler::MEM_DIR);
  }|
  LD DOLLAR SYMBOL COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::LD, 0, 0, $5, $3, 0, Assembler::MEM_DIR);
  }|
  LD LITERAL COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::LD, 0, 0, $4, 0, $2, Assembler::MEM_IND);
  }|
  LD SYMBOL COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::LD, 0, 0, $4, $2, 0, Assembler::MEM_IND);
  }|
  LD GP_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::LD, $2, 0, $4, 0, 0, Assembler::REG_DIR);
  }|
  LD LS_BRACKET GP_REGISTER RS_BRACKET COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::LD, $3, 0, $6, 0, 0, Assembler::REG_IND);
  }|
  LD LS_BRACKET GP_REGISTER PLUS LITERAL RS_BRACKET COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::LD, $3, 0, $8, 0, $5, Assembler::REG_IND_DISP);
  }|
  LD LS_BRACKET GP_REGISTER PLUS SYMBOL RS_BRACKET COMMA GP_REGISTER{
    assembler->addInstruction(Assembler::LD, $3, 0, $8, $5, 0, Assembler::REG_IND_DISP);
  }|
  ST GP_REGISTER COMMA DOLLAR LITERAL {
    assembler->addInstruction(Assembler::ST, $2, 0, 0, 0, $5, Assembler::MEM_DIR);
  }|
  ST GP_REGISTER COMMA DOLLAR SYMBOL {
    assembler->addInstruction(Assembler::ST, $2, 0, 0, $5, 0, Assembler::MEM_DIR);
  }|
  ST GP_REGISTER COMMA LITERAL {
    assembler->addInstruction(Assembler::ST, $2, 0, 0, 0, $4, Assembler::MEM_IND);
  }|
  ST GP_REGISTER COMMA SYMBOL {
    assembler->addInstruction(Assembler::ST, $2, 0, 0, $4, 0, Assembler::MEM_IND);
  }|
  ST GP_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::ST, $2, 0, $4, 0, 0, Assembler::REG_DIR);
  }|
  ST GP_REGISTER COMMA LS_BRACKET GP_REGISTER RS_BRACKET {
    assembler->addInstruction(Assembler::ST, $2, 0, $5, 0, 0, Assembler::REG_IND);
  }|
  ST GP_REGISTER COMMA LS_BRACKET GP_REGISTER PLUS LITERAL RS_BRACKET {
    assembler->addInstruction(Assembler::ST, $2, 0, $5, 0, $7, Assembler::REG_IND_DISP);
  }|
  ST GP_REGISTER COMMA LS_BRACKET GP_REGISTER PLUS SYMBOL RS_BRACKET {
    assembler->addInstruction(Assembler::ST, $2, 0, $5, $7, 0, Assembler::REG_IND_DISP);
  }|
  CSRRD CSR_REGISTER COMMA GP_REGISTER {
    assembler->addInstruction(Assembler::CSRRD, $2, 0, $4, 0, 0, Assembler::REG_DIR);
  }|
  CSRWR GP_REGISTER COMMA CSR_REGISTER {
    assembler->addInstruction(Assembler::CSRWR, $2, 0, $4, 0, 0, Assembler::REG_DIR);
  }

DIRECTIVES:
  GLOBAL {
    listNumber = 1;
  }|
  EXTERN {
    listNumber = 2;
  }|
  SECTION SYMBOL{
    assembler->addDirective(Assembler::SECTION, $2, 0, std::list<char*>());
  }|
  WORD {
    listNumber = 3;
  }|
  SKIP LITERAL {
    assembler->addDirective(Assembler::SKIP, 0, $2, std::list<char*>());
  }|
  ASCII STRING{
    assembler->addDirective(Assembler::ASCII, 0, $2, std::list<char*>());
  }|
  EQU SYMBOL COMMA LITERAL {
    assembler->addDirective(Assembler::EQU, $2, $4, std::list<char*>());
  }|
  END {
    assembler->addDirective(Assembler::END, 0, 0, std::list<char*>());
    assembler->setFileName(outFile);
    assembler->pass();
  }

LABELS:
  LABEL {assembler->addDirective(Assembler::LABEL, $1, 0, std::list<char*>());}

COMMENTS:
  COMMENT {
    assembler->addDirective(Assembler::COMMENT, 0, 0, std::list<char*>());
  }

SYMBOLS:
  SYMBOL {
    addSymbol(yylval.str, listNumber);
  }

LITERALS:
  LITERAL{
    addSymbol(yylval.str, listNumber);
  }


END_LINE:
  ENDL {
    if(listNumber != 0) {
      if(listNumber == 1) {
        assembler->addDirective(Assembler::GLOBAL, 0, 0, globalList);
      } else if (listNumber == 2) {
        assembler->addDirective(Assembler::EXTERN, 0, 0, externList);
      } else if (listNumber == 3) {
        assembler->addDirective(Assembler::WORD, 0, 0, wordList);
      } 
      globalList.clear(); 
      externList.clear();
      wordList.clear();
    }
    indir = 0;
    listNumber = 0;
  }
%%

void yyerror(char* s) {
  printf("\n------------------\nERROR ON LINE: %d\n", linenum + 1);
}

void addSymbol(char* symbol, int listNumber) {

  if(listNumber == 1) {
    globalList.push_back(symbol);
  } else if (listNumber == 2) {
    externList.push_back(symbol);

  } else if (listNumber == 3) {
    wordList.push_back(symbol);
  }
}