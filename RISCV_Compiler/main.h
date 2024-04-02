#ifndef MAIN_H
#define MAIN_H
#include "compiler_common.h"

#define VAR_FLAG_PTR 0b00000001
#define VAR_FLAG_INC_ASSIGN 0b00000010
#define code(code) "\"" code "\\n\\t\"\n"

extern FILE* yyin;
extern uint32_t yylineno;
extern bool compileError;
extern char* yyInputFileName;
int yyparse();
int yylex();
int yylex_destroy();

void pushFunVar(ObjectType variableType, const char* variableName, bool ptr);
Object* findVariable(const char* variableName);
bool objectAdd(Object* a, Object* b);
bool objectDiv(Object* a, Object* b);
bool objectAssignVal(Object* a);

#endif