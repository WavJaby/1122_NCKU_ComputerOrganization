#ifndef MAIN_H
#define MAIN_H
#include "compiler_common.h"

#define VAR_FLAG_POINTER_VALUE 0b00000001
#define VAR_FLAG_IS_REGISTER 0b00000010

extern FILE* yyin;
extern uint32_t yylineno;
extern bool compileError;
extern char* yyInputFileName;
int yyparse();
int yylex();
int yylex_destroy();

void pushFunVar(ObjectType variableType, const char* variableName, bool ptr);
Object* findVariable(const char* variableName);
bool objectAdd(Object* a, Object* b, Object* out);
bool objectSub(Object* a, Object* b, Object* out);
bool objectMul(Object* a, Object* b, Object* out);
bool objectDiv(Object* a, Object* b, Object* out);
bool objectIncreaseAssign(Object* a);
bool objectDecreaseAssign(Object* a);
bool objectValueAssign(Object* a, Object* b);
bool objectAddAssign(Object* dest, Object* val);
bool objectSubAssign(Object* dest, Object* val);

#endif