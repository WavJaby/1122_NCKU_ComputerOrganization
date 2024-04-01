#ifndef MAIN_H
#define MAIN_H
#include "compiler_common.h"

FILE* yyin;
FILE* fout;
int yyparse();
int yylex();
int yylex_destroy();
uint32_t yylineno;
uint32_t yyleng;

bool compileError;

void pushFunVar(ObjectType variableType, const char* variableName);

#endif