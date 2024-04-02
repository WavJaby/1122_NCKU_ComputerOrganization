#include "main.h"
#define DEBUG
#define WJCL_LINKED_LIST_IMPLEMENTATION
#define WJCL_HASH_MAP_IMPLEMENTATION
#include "../../WJCL/map/wjcl_hash_map.h"

#define RISCV_PREFIX "//########## Generate by RISC-V compiler ##########"
#define RISCV_SUBFIX "//##########     Compiler by WavJaby     ##########"
#define debugPrintInst(inst, instc, a, b) printf("%s: %s%s %s %s%s\n", inst, (a)->value ? "*" : "", (a)->symbol->name, instc, (b)->value ? "*" : "", (b)->symbol->name);

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

void pushFunVar(ObjectType variableType, const char* variableName, bool ptr) {
    Object* obj = malloc(sizeof(Object));
    obj->type = variableType;
    obj->value = !ptr;
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
    if (!value->symbol)
        return NULL;
    return value;
}

void instruction3(const char* inst, Object* a, uint64_t aPtr, Object* b, uint64_t bPtr) {
    if (!aPtr && !bPtr)
        fprintf(tempOut, code("%s t0, %%[%s], %%[%s]"), inst, a->symbol->name, b->symbol->name);
    else {
        if (aPtr)
            fprintf(tempOut, code("lw t1, 0(%%[%s])"), a->symbol->name);
        if (aPtr & VAR_FLAG_INC_ASSIGN) {
            printf("Increase variable: %s%s\n", a->symbol->pointer ? "*" : "", a->symbol->name);
            fprintf(tempOut, code("addi %%[%s], %%[%s], 4"), a->symbol->name, a->symbol->name);
        }
        if (bPtr)
            fprintf(tempOut, code("lw t2, 0(%%[%s])"), b->symbol->name);
        if (bPtr & VAR_FLAG_INC_ASSIGN) {
            printf("Increase variable: %s%s\n", b->symbol->pointer ? "*" : "", b->symbol->name);
            fprintf(tempOut, code("addi %%[%s], %%[%s], 4"), b->symbol->name, b->symbol->name);
        }

        if (aPtr && bPtr)
            fprintf(tempOut, code("%s t0, t1, t2"), inst);
        else if (aPtr)
            fprintf(tempOut, code("%s t0, t1, %%[%s]"), inst, b->symbol->name);
        else
            fprintf(tempOut, code("%s t0, %%[%s], t2"), inst, a->symbol->name);
    }
}

bool objectAdd(Object* a, Object* b) {
    if (!a->symbol || !b->symbol)
        return true;
    debugPrintInst("Add", "+", a, b);
    instruction3("add", a, a->value, b, b->value);
    return false;
}

bool objectDiv(Object* a, Object* b) {
    if (!a->symbol || !b->symbol)
        return true;
    debugPrintInst("Div", "/", a, b);
    printf("%d\n", a->value);
    instruction3("div", a, a->value, b, b->value);
    return false;
}

bool objectAssignVal(Object* a) {
    if (!a->symbol)
        return true;
    a->symbol->write = true;
    printf("Assign variable: %s%s\n", a->value ? "*" : "", a->symbol->name);
    fprintf(tempOut, code("MV %%[%s], t0"), a->symbol->name);
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

    // Write input ouput
    map_entries(&funVar, entries, {
        Object* obj = entries->value;
        if (obj->symbol)
            fprintf(tempOut, ":[%s] \"%s\"(%s)\n", (char*)entries->key, obj->symbol->write ? "+r" : "r", (char*)entries->key);
    });
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