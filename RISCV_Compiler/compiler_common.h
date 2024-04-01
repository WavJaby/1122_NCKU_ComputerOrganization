#ifndef COMPILER_COMMON_H
#define COMPILER_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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

typedef struct _symbolData {
    uint32_t index;
    char* name;
    uint64_t addr;
    bool write;
} SymbolData;

typedef struct Object {
    ObjectType type;
    uint64_t value;
    SymbolData* symbolData;
} Object;

#endif /* COMPILER_COMMON_H */