#include "main.h"
#define DEBUG
#define WJCL_LINKED_LIST_IMPLEMENTATION
#define WJCL_HASH_MAP_IMPLEMENTATION
#include "../../WJCL/map/wjcl_hash_map.h"

#define RISCV_PREFIX "//########## Generate by RISC-V compiler ##########"
#define RISCV_SUBFIX "//##########     Compiler by WavJaby     ##########"
#define pointerPrefix(obj) (obj)->symbol->pointer && (obj)->value& VAR_FLAG_POINTER_VALUE ? "*" : ""
#define toInt(val) (*(int*)&val)
#define objectSize(obj) ((obj)->symbol->pointer && !((obj)->value & VAR_FLAG_POINTER_VALUE) /* Is pointer && get pointer value */ \
                             ? objectTypeSize[(obj)->type]                                                                        \
                             : 1)
#define code(code) "\"" code "\\n\\t\"\n"

uint8_t objectTypeSize[] = {
    [OBJECT_TYPE_UNDEFINED] = 0,
    [OBJECT_TYPE_AUTO] = 0,
    [OBJECT_TYPE_VOID] = 0,
    [OBJECT_TYPE_CHAR] = 1,
    [OBJECT_TYPE_INT] = 4,
    [OBJECT_TYPE_LONG] = 8,
    [OBJECT_TYPE_FLOAT] = 4,
    [OBJECT_TYPE_DOUBLE] = 8,
    [OBJECT_TYPE_BOOL] = 1,
    [OBJECT_TYPE_STR] = 0,
    [OBJECT_TYPE_FUNCTION] = 0,
    [OBJECT_TYPE_ARRAY] = 0,
};

FILE *tempOut, *outFile;

char* newStrCat(const char* a, const char* b) {
    char* cache = (char*)malloc(strlen(a) + strlen(b) + 1);
    strcpy(cache, a);
    strcat(cache, b);
    return cache;
}

uint32_t strHash(const char* str) {
    uint32_t hash = 0, seed = 131;
    while (*str)
        hash = hash * seed + (*str++);
    return hash;
}

bool strEquals(void* a, void* b) {
    return strcmp((const char*)a, (const char*)b) == 0;
}

bool objectEquals(void* a, void* b) {
    Object *objA = (Object*)a, *objB = (Object*)b;
    if (objA->type != objA->type)
        return false;
    switch (objA->type) {
    case OBJECT_TYPE_STR:
        return strcmp((const char*)objA->value, (const char*)objA->value) == 0;
    default:
        return objA->value == objB->value;
    }
}
uint32_t objectHash(void* key) {
    Object* obj = (Object*)key;
    if (obj->type == OBJECT_TYPE_STR)
        return obj->type | strHash((const char*)obj->value);
    else
        return obj->type | (obj->value + (obj->value >> 32));
}

void objectFree(void* key, void* value) {
    Object* obj = (Object*)value;
    if (obj->type == OBJECT_TYPE_STR)
        free((char*)obj->value);
    if (obj->symbol) {
        free(obj->symbol->name);
        free(obj->symbol);
    }
}

NodeInfo funVarInfo = {
    .equalsFunction = strEquals,
    .hashFunction = (uint32_t(*)(void*))strHash,
    .freeFlag = WJCL_HASH_MAP_FREE_VALUE,  // Key(variable name) free in objectFree
    .onNodeDelete = objectFree,
};
Map funVar;

void debugPrintInst(const char* instc, Object* a, Object* b) {
    if (a->symbol && b->symbol)
        printf("%s%s %s %s%s\n",
               pointerPrefix(a), (a)->symbol->name, instc,
               pointerPrefix(b), (b)->symbol->name);
    else if (!a->symbol && !b->symbol)
        printf("%d %s %d\n",
               toInt(a->value), instc,
               toInt(a->value));
    else if (!a->symbol && b->symbol)
        printf("%d %s %s%s\n",
               toInt(a->value), instc,
               pointerPrefix(b), (b)->symbol->name);
    else
        printf("%s%s %s %d\n",
               pointerPrefix(a), (a)->symbol->name, instc,
               toInt(a->value));
}

void pushFunVar(ObjectType variableType, const char* variableName, bool ptr) {
    Object* obj = malloc(sizeof(Object));
    obj->type = variableType;
    obj->value = 0;
    SymbolData* symbol = obj->symbol = malloc(sizeof(SymbolData));
    symbol->index = 0;
    symbol->name = (char*)variableName;
    symbol->addr = 0;
    symbol->pointer = ptr;
    symbol->write = false;
    printf("Function variable(%lu): %s\n", funVar.size, variableName);
    map_putpp(&funVar, (void*)variableName, obj);
}

Object* findVariable(const char* variableName) {
    Object* value = (Object*)map_get(&funVar, (void*)variableName);
    printf("Find variable: %s\n", variableName);
    if (!value || !value->symbol)
        return NULL;
    return value;
}

bool objectIncreaseAssignNum(Object* a, int num) {
    if (!a->symbol)
        return true;

    a->symbol->write = true;
    printf("Increase variable '%s%s' by %d\n", pointerPrefix(a), a->symbol->name, num);

    if (a->symbol->pointer && a->value & VAR_FLAG_POINTER_VALUE)  // Is pointer && get pointer value
        fprintf(tempOut, code("addi 0(%%[%s]), 0(%%[%s]), %d"), a->symbol->name, a->symbol->name, num);
    else
        fprintf(tempOut, code("addi %%[%s], %%[%s], %d"), a->symbol->name, a->symbol->name, num);
    return false;
}

// ++
bool objectIncreaseAssign(Object* a) {
    objectIncreaseAssignNum(a, objectSize(a));
}

// --
bool objectDecreaseAssign(Object* a) {
    objectIncreaseAssignNum(a, -objectSize(a));
}

#define VAR "%s%s%s"
#define setVAR(var) ((var)->value & VAR_FLAG_IS_REGISTER)          \
                        ? (((var)->value & VAR_FLAG_POINTER_VALUE) \
                               ? "0("                              \
                               : "")                               \
                        : (((var)->value & VAR_FLAG_POINTER_VALUE) \
                               ? "0(%["                            \
                               : "%["),                            \
                    (var)->symbol->name,                           \
                    ((var)->value & VAR_FLAG_IS_REGISTER)          \
                        ? (((var)->value & VAR_FLAG_POINTER_VALUE) \
                               ? ")"                               \
                               : "")                               \
                        : (((var)->value & VAR_FLAG_POINTER_VALUE) \
                               ? "])"                              \
                               : "]")

// =
bool objectValueAssign(Object* dest, Object* value) {
    if (!dest->symbol)
        return true;

    dest->symbol->write = true;
    // Assign variable
    if (value->symbol) {
        // From register
        fprintf(tempOut, code("mv " VAR ", " VAR), setVAR(dest), setVAR(value));

        printf("%s%s = %s%s\n", pointerPrefix(dest), dest->symbol->name, pointerPrefix(value), value->symbol->name);

        return false;
    }

    // Assign value
    switch (value->type) {
    case OBJECT_TYPE_INT:
        printf("%s%s = %d\n", pointerPrefix(dest), dest->symbol->name, toInt(value->value));
        fprintf(tempOut, dest->value & VAR_FLAG_POINTER_VALUE  // Get pointer value
                             ? code("lui 0(%%[%s]), %d")
                             : code("lui %%[%s], %d"),
                dest->symbol->name, toInt(value->value));
        break;
    default:
        return false;
    }
    return false;
}

bool objectAssign(Object* dest, Object* val, bool sub) {
    dest->symbol->write = true;
    if (dest->type != val->type)
        return false;

    // Variable value
    if (val->symbol)
        return false;

    // Normal value
    switch (dest->type) {
    case OBJECT_TYPE_INT:
        if (sub)
            objectIncreaseAssignNum(dest, -objectSize(dest) * toInt(val->value));
        else
            objectIncreaseAssignNum(dest, objectSize(dest) * toInt(val->value));
        break;
    default:
        return false;
    }
    return false;
}

// +=
bool objectAddAssign(Object* dest, Object* val) {
    if (!dest->symbol)
        return true;

    return objectAssign(dest, val, false);
}

// -=
bool objectSubAssign(Object* dest, Object* val) {
    if (!dest->symbol)
        return true;

    return objectAssign(dest, val, true);
}

void variableValueInst3(const char* inst, Object* a, uint64_t aFlag, Object* value, Object* out) {
    // Two variable
    fprintf(tempOut, code("lw t1, " VAR), setVAR(a));
    fprintf(tempOut, a->symbol->pointer && aFlag & VAR_FLAG_POINTER_VALUE  // Is pointer && get pointer value
                         ? code("%si 0(%%[%s]), 0(%%[%s]), %d")
                         : code("%si %%[%s], %%[%s], %d"),
            inst, a->symbol->name, a->symbol->name,
            a->symbol->pointer ? toInt(value->value) * objectTypeSize[OBJECT_TYPE_INT] : toInt(value->value));
}

void variableInst3(const char* inst, Object* a, uint64_t aFlag, Object* b, uint64_t bFlag, Object* out) {
    // Two variable add
    if (aFlag & VAR_FLAG_POINTER_VALUE)  // Store variable pointer value to t1
        fprintf(tempOut, code("lw t1, 0(%%[%s])"), a->symbol->name);
    else if (aFlag & ~VAR_FLAG_IS_REGISTER)  // Store variable value to t1
        fprintf(tempOut, code("lw t1, %%[%s]"), a->symbol->name);

    if (bFlag & VAR_FLAG_POINTER_VALUE)  // Store variable pointer value to t2
        fprintf(tempOut, code("lw t2, 0(%%[%s])"), b->symbol->name);
    else if (bFlag & ~VAR_FLAG_IS_REGISTER)  // Store variable value to t2
        fprintf(tempOut, code("lw t2, %%[%s]"), b->symbol->name);

    if (aFlag && bFlag)
        fprintf(tempOut, code("%s t0, %s, %s"), inst,
                (aFlag & VAR_FLAG_IS_REGISTER) ? a->symbol->name : "t1",
                (bFlag & VAR_FLAG_IS_REGISTER) ? b->symbol->name : "t2");
    else if (!aFlag && !bFlag)
        fprintf(tempOut, code("%s t0, %%[%s], %%[%s]"), inst, a->symbol->name, b->symbol->name);
    else if (aFlag)
        fprintf(tempOut, code("%s t0, %s, %%[%s]"), inst, b->symbol->name,
                (aFlag & VAR_FLAG_IS_REGISTER) ? a->symbol->name : "t1");
    else
        fprintf(tempOut, code("%s t0, %%[%s], %s"), inst, a->symbol->name,
                (bFlag & VAR_FLAG_IS_REGISTER) ? b->symbol->name : "t2");
}

bool objectAdd(Object* a, Object* b, Object* out) {
    // Two value add
    if (!a->symbol && !b->symbol) {
        switch (a->type) {
        case OBJECT_TYPE_INT:
            switch (b->type) {
            case OBJECT_TYPE_INT:
                out->type = OBJECT_TYPE_INT;
                out->value = toInt(a->value) + toInt(b->value);
                out->symbol = NULL;
                printf("%d + %d = %d\n", toInt(a->value), toInt(b->value), toInt(out->value));
                return false;
            default:
                return true;
            }
            break;
        default:
            return true;
        }
        return false;
    }

    SymbolData* symbol = malloc(sizeof(SymbolData));  // TODO: Need free
    symbol->name = "t0";
    out->type = OBJECT_TYPE_INT;
    out->value = VAR_FLAG_IS_REGISTER;
    out->symbol = symbol;

    // Two variable add
    if (a->symbol && b->symbol) {
        debugPrintInst("+", a, b);
        variableInst3("add", a, a->value, b, b->value, out);
        return false;
    }

    // Variable and Value
    if (b->symbol) {
        Object* swap = a;
        a = b;
        b = swap;
    }
    printf("%s%s %s %d\n",
           pointerPrefix(a), (a)->symbol->name, "+",
           toInt(b->value));
    variableValueInst3("add", a, a->value, b, out);
    return false;
}

bool objectSub(Object* a, Object* b, Object* out) {
}

bool objectMul(Object* a, Object* b, Object* out) {
}

bool objectDiv(Object* a, Object* b, Object* out) {
    if (!a->symbol || !b->symbol)
        return true;
    debugPrintInst("/", a, b);
    variableInst3("div", a, a->value, b, b->value, out);
    return false;
}

int main(int argc, char* argv[]) {
    char *tempOutputFileName = NULL, *outputFileName = NULL, *outputName = NULL;
    if (argc == 3) {
        yyin = fopen(yyInputFileName = argv[1], "r");
        outputName = argv[2];
    } else if (argc == 2) {
        yyin = stdin;
        outputName = argv[1];
    } else {
        yyin = stdin;
        outputName = "output";
    }
    if (!yyin) {
        printf("file `%s` doesn't exists or cannot be opened\n", yyInputFileName);
        exit(1);
    }
    outputFileName = newStrCat(outputName, ".c");
    outFile = fopen(outputFileName, "r");
    if (!outFile) {
        printf("file `%s` cannot be opened\n", outputFileName);
        exit(1);
    }

    tempOutputFileName = newStrCat(outputName, "_.c");
    tempOut = fopen(tempOutputFileName, "w");
    if (!tempOut) {
        printf("file `%s` cannot be opened\n", tempOutputFileName);
        exit(1);
    }

    // Find start
    const char startStr[] = "asm volatile(";
    const char endStr[] = ");";
    char cache[sizeof(startStr)] = "";
    cache[sizeof(startStr) - 1] = 0;
    char c;
    bool findStart = true;
    while (fread(&c, 1, 1, outFile)) {
        if (c != '\n' && c != '\r' && c != '\t') {
            for (size_t i = 0; i < sizeof(startStr) - 2; i++)
                cache[i] = cache[i + 1];
            cache[sizeof(startStr) - 2] = c;
        }
        if (findStart) {
            fwrite(&c, 1, 1, tempOut);
            // Find start
            if (!strcmp(cache, startStr))
                findStart = false;
        } else {
            // Find end
            if (!strncmp(cache + (sizeof(startStr) - sizeof(endStr)), endStr, sizeof(endStr) - 1))
                break;
        }
    }
    fprintf(tempOut, "\n" RISCV_PREFIX "\n");

    // Start parsing
    funVar = map_create(funVarInfo);

    yyparse();
    printf("Total lines: %d\n", yylineno);
    fclose(yyin);

    // Read/Write variable
    bool first = true;
    map_entries(&funVar, entries, {
        Object* obj = entries->value;
        if (obj->symbol->write) {
            fprintf(tempOut, "%s [%s] \"+r\"(%s)", first ? ":" : ",", (char*)entries->key, (char*)entries->key);
            first = false;
        }
    });
    fprintf(tempOut, first ? ":\n" : "\n");
    // Read only variable
    first = true;
    map_entries(&funVar, entries, {
        Object* obj = entries->value;
        if (!obj->symbol->write) {
            fprintf(tempOut, "%s [%s] \"r\"(%s)", first ? ":" : ",", (char*)entries->key, (char*)entries->key);
            first = false;
        }
    });
    fprintf(tempOut, first ? ":\n" : "\n");
    map_free(&funVar);

    if (compileError) {
        fclose(tempOut);
        fclose(outFile);
        remove(tempOutputFileName);
    } else {
        // Finish read file
        fprintf(tempOut, RISCV_SUBFIX "\n);");
        size_t len;
        while ((len = fread(cache, 1, sizeof(cache), outFile))) {
            fwrite(cache, 1, len, tempOut);
        }
        fclose(tempOut);
        fclose(outFile);
        rename(tempOutputFileName, outputFileName);
    }

    free(outputFileName);
    free(tempOutputFileName);
    yylex_destroy();
    return 0;
}