#ifndef MAIN_H
#define MAIN_H
#include "compiler_common.h"

#define VAR_FLAG_PTR_VALUE 0b00000001
#define VAR_FLAG_REG 0b00000010
#define VAR_FLAG_CREATED 0b00000100

extern FILE* yyin;
extern uint32_t yylineno;
extern bool compileError;
extern char* yyInputFileName;
int yyparse();
int yylex();
int yylex_destroy();

void pushFunVar(ObjectType variableType, const char* variableName, bool ptr);
Object* findVariable(const char* variableName);
Object* createVariable(ObjectType type, const char* variableName, Object* value);
bool objectAdd(Object* a, Object* b, Object* out);
bool objectSub(Object* a, Object* b, Object* out);
bool objectMul(Object* a, Object* b, Object* out);
bool objectDiv(Object* a, Object* b, Object* out);
bool objectLes(Object* a, Object* b, Object* out);
bool objectGtr(Object* a, Object* b, Object* out);
bool objectIncAssign(Object* a, Object* out);
bool objectDecAssign(Object* a, Object* out);
bool objectValueAssign(Object* dest, Object* val);
bool objectAddAssign(Object* dest, Object* val);
bool objectSubAssign(Object* dest, Object* val);
bool getPointerValue(Object* exp, Object* out);
bool forBegin();
bool forConditionEnd(Object* result);
bool forHeaderEnd();
bool forEnd();

#endif