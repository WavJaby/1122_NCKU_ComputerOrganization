#include "main.h"
#define DEBUG_OUT
#define WJCL_LINKED_LIST_IMPLEMENTATION
#define WJCL_HASH_MAP_IMPLEMENTATION
#include "../../WJCL/map/wjcl_hash_map.h"
#include "register_stack.h"
#include "string_cache.h"
#include "value_operation.h"

#define RISCV_PREFIX "//########## Generate by RISC-V compiler ##########"
#define RISCV_SUBFIX "//##########     Compiler by WavJaby     ##########"
#define DISPVAR "%s%s"
#define setDISPVAR(obj) (obj)->symbol                                \
                            ? (obj)->value &VAR_FLAG_PTR_VALUE ? "*" \
                              : (obj)->value &VAR_FLAG_REG     ? "_" \
                                                               : ""  \
                            : "",                                    \
                        (obj)->symbol ? (obj)->symbol->name : ""

#define VAR "%s%s%s"
#define setVAR(var) ((var)->value & VAR_FLAG_REG)              \
                        ? (((var)->value & VAR_FLAG_PTR_VALUE) \
                               ? getVariablePtrOffsetStr(var)  \
                               : "")                           \
                        : (((var)->value & VAR_FLAG_PTR_VALUE) \
                               ? getVariablePtrOffsetStr(var)  \
                               : "%["),                        \
                    (var)->symbol->name,                       \
                    ((var)->value & VAR_FLAG_REG)              \
                        ? (((var)->value & VAR_FLAG_PTR_VALUE) \
                               ? ")"                           \
                               : "")                           \
                        : (((var)->value & VAR_FLAG_PTR_VALUE) \
                               ? "])"                          \
                               : "]")

#define isReg(obj) (obj)->symbol && (obj)->value& VAR_FLAG_REG
#define isPtrVal(obj) (obj)->symbol && (obj)->value& VAR_FLAG_PTR_VALUE

#define debug printf("%s:%d: ############### debug\n", __FILE__, __LINE__)
#ifdef DEBUG_OUT
#define codeOut(format, ...)      \
    if (useCacheCodeStream != -1) \
    ssprintf(cacheCodeStream[useCacheCodeStream], "%*s\"" format "\\n\\t\"", indent << 2, "", __VA_ARGS__) else fprintf(tempOut, "%*s\"" format "\\n\\t\"", indent << 2, "", __VA_ARGS__)
#define debugOut(format, ...)                                                                                                                    \
    if (useCacheCodeStream != -1)                                                                                                                \
        ssprintf(cacheCodeStream[useCacheCodeStream], "  // " format "\n", __VA_ARGS__) else fprintf(tempOut, "  // " format "\n", __VA_ARGS__); \
    printf(format "\n", __VA_ARGS__)
#define debugOutRaw(...)                                                                               \
    if (useCacheCodeStream != -1)                                                                      \
        ssprintf(cacheCodeStream[useCacheCodeStream], __VA_ARGS__) else fprintf(tempOut, __VA_ARGS__); \
    printf(__VA_ARGS__)
#else
#define codeOut(format, ...)      \
    if (useCacheCodeStream != -1) \
    ssprintf(cacheCodeStream[useCacheCodeStream], "\"" format "\\n\\t\"\n", __VA_ARGS__) else fprintf(tempOut, "\"" format "\\n\\t\"\n", __VA_ARGS__)
#define debugOut(...)
#define debugOutRaw(...)
#endif

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
int useCacheCodeStream = -1;
StringStream* cacheCodeStream[20];
int indent = 0;
int createdVariableIndex = -20;

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
        if (!(obj->value & VAR_FLAG_CREATED))
            free(obj->symbol->name);
        free(obj->symbol);
    }
}

NodeInfo variableInfo = {
    .equalsFunction = strEquals,
    .hashFunction = (uint32_t(*)(void*))strHash,
    .freeFlag = WJCL_HASH_MAP_FREE_VALUE,  // Key(variable name) free in objectFree
    .onNodeDelete = objectFree,
};
Map funVar, createdVar;

void debugPrintInst(char instc, Object* a, Object* b, Object* out) {
    if (a->symbol) {
        debugOut(DISPVAR " %c %d => " DISPVAR "", setDISPVAR(a), instc, toInt(b->value), setDISPVAR(out));
    } else {
        debugOut("%d %c " DISPVAR " => " DISPVAR "", toInt(a->value), instc, setDISPVAR(b), setDISPVAR(out));
    }
}

char* getVariablePtrOffsetStr(Object* obj) {
    static char cache1[12], cache2[12];
    static bool sw = false;
    char* cache = (sw = !sw) ? cache1 : cache2;
    if (obj->value & VAR_FLAG_REG)
        sprintf(cache, "%d(", obj->ptrOffset);
    else
        sprintf(cache, "%d(%%[", obj->ptrOffset * objectTypeSize[obj->type]);
    return cache;
}

// =
bool objectValueAssign(Object* dest, Object* val) {
    if (!dest->symbol)
        return true;

    dest->symbol->write = true;
    // Assign from variable
    if (val->symbol) {
        if (isPtrVal(dest)) {
            if (isPtrVal(val)) {
                char* cacheReg = getTmpReg(false);
                codeOut("lw %s, " VAR, cacheReg, setVAR(val));
                debugOutRaw("\n");
                codeOut("sw %s, " VAR, cacheReg, setVAR(dest));
            } else
                codeOut("sw " VAR ", " VAR, setVAR(val), setVAR(dest));
        } else {
            codeOut("mv " VAR ", " VAR, setVAR(dest), setVAR(val));
        }
        debugOut(DISPVAR " <= " DISPVAR, setDISPVAR(dest), setDISPVAR(val));
        if (isReg(dest))
            freeTmpReg(dest);
        if (isReg(val))
            freeTmpReg(val);
        return false;
    }

    // Assign from value
    switch (val->type) {
    case OBJECT_TYPE_INT:
        if (dest->symbol && dest->value & VAR_FLAG_PTR_VALUE) {
            char* cacheReg = getTmpReg(false);
            codeOut("addi %s, zero, %d", cacheReg, toInt(val->value));
            debugOutRaw("\n");
            codeOut("sw %s, " VAR, cacheReg, setVAR(dest));
        } else {
            codeOut("addi " VAR ", zero, %d", setVAR(dest), toInt(val->value));
        }
        debugOut(DISPVAR " <= %d", setDISPVAR(dest), toInt(val->value));
        if (isReg(dest))
            freeTmpReg(dest);
        if (isReg(val))
            freeTmpReg(val);
        return false;
    }
    return true;
}

void pushFunVar(ObjectType variableType, const char* variableName, bool ptr) {
    Object* obj = malloc(sizeof(Object));
    obj->type = variableType;
    obj->value = 0;
    obj->ptrOffset = 0;
    SymbolData* symbol = obj->symbol = malloc(sizeof(SymbolData));
    symbol->name = (char*)variableName;
    symbol->write = false;
    // printf("Function variable(%lu): %s\n", funVar.size, variableName);
    map_putpp(&funVar, (void*)variableName, obj);
}

Object* createVariable(ObjectType type, const char* variableName, Object* value) {
    Object* obj = malloc(sizeof(Object));
    obj->type = type;
    obj->value = VAR_FLAG_REG | VAR_FLAG_PTR_VALUE | VAR_FLAG_CREATED;
    obj->ptrOffset = createdVariableIndex;
    createdVariableIndex -= objectTypeSize[type];
    SymbolData* symbol = obj->symbol = malloc(sizeof(SymbolData));
    symbol->name = "s0";
    symbol->write = false;
    printf("Create variable(%lu): %s\n", funVar.size, variableName);
    objectValueAssign(obj, value);
    map_putpp(&createdVar, (void*)variableName, obj);
    return obj;
}

Object* findVariable(const char* variableName) {
    Object* value = (Object*)map_get(&funVar, (void*)variableName);
    if (!value || !value->symbol)
        value = (Object*)map_get(&createdVar, (void*)variableName);
    if (!value || !value->symbol)
        return NULL;
    printf("Find variable '%s'\n", variableName);
    return value;
}

void variableInst3(const char* inst, Object* a, Object* b, Object* out) {
    Object cacheA = {a->type, VAR_FLAG_REG, 0, &(SymbolData){}},
           cacheB = {b->type, VAR_FLAG_REG, 0, &(SymbolData){}};
    if (a->symbol && a->value & VAR_FLAG_PTR_VALUE) {  // Load variable pointer
        cacheA.symbol->name = getTmpReg(true);
        codeOut("lw %s, " VAR, cacheA.symbol->name, setVAR(a));
        debugOut(DISPVAR, setDISPVAR(a));
        a = &cacheA;
    }
    if (b->symbol && b->value & VAR_FLAG_PTR_VALUE) {  // Load variable pointer
        cacheB.symbol->name = getTmpReg(true);
        codeOut("lw %s, " VAR, cacheB.symbol->name, setVAR(b));
        debugOut(DISPVAR, setDISPVAR(b));
        b = &cacheB;
    }
    if (isReg(a))
        freeTmpReg(a);
    if (isReg(b))
        freeTmpReg(b);
    out->symbol->name = getTmpReg(true);
    codeOut("%s " VAR ", " VAR ", " VAR, inst, setVAR(out), setVAR(a), setVAR(b));
}

// *ptr
bool getPointerValue(Object* exp, Object* out) {
    out->type = exp->type;
    out->value = exp->value | VAR_FLAG_PTR_VALUE;
    out->ptrOffset = exp->ptrOffset;
    out->symbol = exp->symbol;
    return false;
}

bool objectExpression(char op, Object * a, Object* b, Object* out) {
    // Two value
    /** TODO: Fix here */
    if (!a->symbol && !b->symbol)
        return valueOperation(op, a, b, out);

    char* opInst;
    switch (op) {
    case '+':
        opInst = "ADD";
        break;
    case '-':
        opInst = "SUB";
        break;
    case '*':
        opInst = "MUL";
        break;
    case '/':
        opInst = "DIV";
        break;
    case '<':
        opInst = "SLT";
        break;
    default:
        return true;
    }

    // Create output reg
    out->type = OBJECT_TYPE_INT;
    out->value = VAR_FLAG_REG;
    out->ptrOffset = 0;
    out->symbol = malloc(sizeof(SymbolData)); /** TODO: Need free */

    // Two variable
    if (a->symbol && b->symbol) {
        variableInst3(opInst, a, b, out);
        debugOut(DISPVAR " %c " DISPVAR " => " DISPVAR "", setDISPVAR(a), op, setDISPVAR(b), setDISPVAR(out));
        return false;
    }

    // Variable and Value
    SymbolData symbol = {getTmpReg(true)};
    Object cacheVar = {OBJECT_TYPE_INT, VAR_FLAG_REG, 0, &symbol};

    cacheVar.type = OBJECT_TYPE_INT;
    codeOut("addi %s, zero, %d", symbol.name, toInt((a->symbol ? b : a)->value));
    debugOutRaw("\n");
    if (a->symbol)
        variableInst3(opInst, a, &cacheVar, out);
    else
        variableInst3(opInst, &cacheVar, b, out);
    debugPrintInst(op, a, b, out);

    // Free cache value if used
    if (out->symbol->name != cacheVar.symbol->name)
        freeTmpReg(&cacheVar);
    return false;
}

bool objectAdd(Object* a, Object* b, Object* out) {
    return objectExpression('+', a, b, out);
}

bool objectSub(Object* a, Object* b, Object* out) {
    return objectExpression('-', a, b, out);
}

bool objectMul(Object* a, Object* b, Object* out) {
    return objectExpression('*', a, b, out);
}

bool objectDiv(Object* a, Object* b, Object* out) {
    return objectExpression('/', a, b, out);
}

bool objectLes(Object* a, Object* b, Object* out) {
    return objectExpression('<', a, b, out);
}

bool objectGtr(Object* a, Object* b, Object* out) {
    return objectExpression('<', b, a, out);
}

// +=
bool objectAddAssign(Object* dest, Object* val) {
    Object cache;
    if (objectAdd(dest, val, &cache))
        return true;
    return objectValueAssign(dest, &cache);
}

// -=
bool objectSubAssign(Object* dest, Object* val) {
    Object cache;
    if (objectSub(dest, val, &cache))
        return true;
    return objectValueAssign(dest, &cache);
}

// ++
bool objectIncAssign(Object* a, Object* out) {
}

// --
bool objectDecAssign(Object* a, Object* out) {
}

int offset = 0;
int indentStack[10], indentStackIndex = 0;

bool forBegin() {
    codeOut("j EL%d", offset);
    debugOut("for loop (%d) jump to 'ENTRY'", offset);
    codeOut("L%d:", offset);
    debugOutRaw("\n// for loop (%d) CONTNET\n", offset);
    indentStack[indentStackIndex++] = offset;
    useCacheCodeStream = indent << 1;
    cacheCodeStream[useCacheCodeStream] = newStreamStream();
    ++indent;
}

bool forConditionEnd(Object* result) {
    // result != 0
    codeOut("bne " VAR ", zero, L%d", setVAR(result), offset);
    debugOut("for loop (%d) jump to 'CONTNET'\n", offset);
    if (isReg(result))
        freeTmpReg(result);
    useCacheCodeStream = (indent << 1) - 1;
    cacheCodeStream[useCacheCodeStream] = newStreamStream();
}

bool forHeaderEnd() {
    useCacheCodeStream = -1;
    ++offset;
}

bool forEnd() {
    int offset = indentStack[--indentStackIndex];
    debugOutRaw("// for loop (%d) CONTNET END\n", offset);
    int conditionIndex = ((indent - 1) << 1);
    // Loop action code
    fprintf(tempOut, "%s", cacheCodeStream[conditionIndex + 1]->data);
    // Loop end
    --indent;
    codeOut("EL%d:", offset);
    ++indent;
    debugOut("for loop (%d) ENTRY", offset);
    // Loop condition code
    fprintf(tempOut, "%s", cacheCodeStream[conditionIndex]->data);
    freeStreamStream(cacheCodeStream[conditionIndex]);
    freeStreamStream(cacheCodeStream[conditionIndex + 1]);
    --indent;
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
    funVar = map_create(variableInfo);
    createdVar = map_create(variableInfo);

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
    map_free(&createdVar);
    // Register
    first = true;
    for (size_t i = 0; i < maxTmpRegIndex + 1; i++) {
        fprintf(tempOut, "%c \"%s\"", first ? ':' : ',', tmpRegName[i]);
        first = false;
    }
    fprintf(tempOut, first ? ":\n" : "\n");

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