#ifndef COMPILER_COMMON_H
#define COMPILER_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_PREFIX "%s:%d:%d: error: "

#define yyerrorf(format, ...)                                                                         \
    {                                                                                                 \
        printf(ERROR_PREFIX format, yyInputFileName, yylineno, yycolumn - yyleng + 1, ##__VA_ARGS__); \
        compileError = true;                                                                          \
        YYABORT;                                                                                      \
    }

typedef enum _objectType {
    OBJECT_TYPE_UNDEFINED,
    OBJECT_TYPE_AUTO,
    OBJECT_TYPE_VOID,
    OBJECT_TYPE_CHAR,
    OBJECT_TYPE_INT,
    OBJECT_TYPE_LONG,
    OBJECT_TYPE_FLOAT,
    OBJECT_TYPE_DOUBLE,
    OBJECT_TYPE_BOOL,
    OBJECT_TYPE_STR,
    OBJECT_TYPE_FUNCTION,
    OBJECT_TYPE_ARRAY,
} ObjectType;

typedef struct SymbolData {
    char* name;
    bool write;
} SymbolData;

typedef struct Object {
    ObjectType type;
    uint64_t value;
    int ptrOffset;
    SymbolData* symbol;
} Object;

#endif /* COMPILER_COMMON_H */