#ifndef MAIN_H
#define MAIN_H
#include "compiler_common.h"

FILE* yyin;
int yyparse();
int yylex();
int yylex_destroy();
uint32_t yylineno;
uint32_t yycolumn;
uint32_t yyleng;

bool compileError;

void pushFunVar(ObjectType variableType, const char* variableName);
Object* findVariable(const char* variableName);
void objectAdd(Object a, Object b);
void objectDiv(Object a, Object b);
void objectAssignVal(Object a);

#endif