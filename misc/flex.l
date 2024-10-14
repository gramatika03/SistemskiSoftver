%{
  #include "bison.tab.hpp"
  #include <cstring>
  int linenum = 0;

  extern int listNumber;
  extern int indir;
%}

%option noyywrap

%%

%r([0-9]|10|11|12|13|14|15) {yylval.num = atoi(yytext + 2); return GP_REGISTER;}
%sp {yylval.num = 14; return GP_REGISTER;}
%pc {yylval.num = 15; return GP_REGISTER;}

%status {yylval.num = 0; return CSR_REGISTER;}
%handler {yylval.num = 1; return CSR_REGISTER;}
%cause {yylval.num = 2; return CSR_REGISTER;}

".global" {indir = 1; return GLOBAL;}
".extern" {indir = 1; return EXTERN;}
".section" {indir = 1; return SECTION;}
".word" {indir = 1; return WORD;}
".skip" {indir = 1; return SKIP;}
".ascii" {indir = 1; return ASCII;}
".equ" {indir = 1; return EQU;}
".end" {return END;}

halt {indir = 1; return HALT;}
int {indir = 1; return INT;}
iret {indir = 1; return IRET;}
call {indir = 1; return CALL;}
ret {indir = 1; return RET;}
jmp {indir = 1; return JMP;}
beq {indir = 1; return BEQ;}
bne {indir = 1; return BNE;}
bgt {indir = 1; return BGT;}
push {indir = 1; return PUSH;}
pop {indir = 1; return POP;}
xchg {indir = 1; return XCHG;}
add {indir = 1; return ADD;}
sub {indir = 1; return SUB;}
mul {indir = 1; return MUL;}
div {indir = 1; return DIV;}
not {indir = 1; return NOT;}
and {indir = 1; return AND;}
or {indir = 1; return OR;}
xor {indir = 1; return XOR;}
shl {indir = 1; return SHL;}
shr {indir = 1; return SHR;}
ld {indir = 1; return LD;}
st {indir = 1; return ST;}
csrrd {indir = 1; return CSRRD;}
csrwr {indir = 1; return CSRWR;}

([0-9]+|0[xX][0-9a-fA-F]+) {
  if(listNumber == 0 && indir == 0) return ERROR;
  yylval.num = listNumber; 
  yylval.str = strdup(yytext); 
  return LITERAL;
}

[a-zA-Z.][a-zA-Z0-9_]*: {
  size_t length = yyleng - 1;
  yylval.str = (char*)malloc(length + 1);
  strncpy(yylval.str, yytext, length);
  yylval.str[length] = '\0';
  return LABEL;  
}

"\""([^"\n]|\\.)*"\"" {yylval.str = strdup(yytext); return STRING;}

"[" {return LS_BRACKET;}
"]" {return RS_BRACKET;}
"," {return COMMA;}
"+" {return PLUS;}
"-" {return MINUS;}
";" {return SEMICOLON;}
":" {return COLON;}
"%" {return PERCENT;}
"$" {return DOLLAR;}

[a-zA-Z_][a-zA-Z0-9_]* {
  if(listNumber == 0 && indir == 0) return ERROR;
  yylval.num = listNumber; 
  yylval.str = strdup(yytext); 
  return SYMBOL;
}
\n {linenum = linenum + 1; return ENDL;}
"#"[^\n\r]* {yylval.str = strdup(yytext); return COMMENT;}

[ \t]+ {;}

. { printf("ERROR: Unrecognized token '%s'\n", yytext); }

%%