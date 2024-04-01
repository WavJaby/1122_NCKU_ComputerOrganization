#include "main.h"
#define DEBUG
#define WJCL_LINKED_LIST_IMPLEMENTATION
#define WJCL_HASH_MAP_IMPLEMENTATION
#include "../../WJCL/map/wjcl_hash_map.h"

#define RISCV_PREFIX "//########## Generate by RISC-V compiler ##########"
#define RISCV_SUBFIX "//##########     Compiler by WavJaby     ##########"
#define code(code) "\"" code "\\n\\t\"\n"

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
    if (obj->symbolData) {
        free(obj->symbolData->name);
        free(obj->symbolData);
    }
}

NodeInfo funVarInfo = {
    .equalsFunction = strEquals,
    .hashFunction = (uint32_t(*)(void*))strHash,
    .freeFlag = WJCL_HASH_MAP_FREE_VALUE,  // Key(variable name) free in objectFree
    .onNodeDelete = objectFree,
};
Map funVar;

void pushFunVar(ObjectType variableType, const char* variableName) {
    Object* obj = malloc(sizeof(Object));
    SymbolData* symbolData = obj->symbolData = malloc(sizeof(SymbolData));
    symbolData->index = 0;
    symbolData->name = (char*)variableName;
    symbolData->addr = 0;
    symbolData->write = false;
    printf("Function variable(%lu): %s\n", funVar.size, variableName);
    map_putpp(&funVar, (void*)variableName, obj);
}

Object* findVariable(const char* variableName) {
    Object* value = (Object*)map_get(&funVar, (void*)variableName);
    printf("Find variable: %s\n", variableName);
    return value;
}

void objectAdd(Object a, Object b) {
    if (a.symbolData && b.symbolData) {
        printf("Add object: %s + %s\n", a.symbolData->name, b.symbolData->name);
        fprintf(tempOut, code("add t0, %%[%s], %%[%s]"), a.symbolData->name, b.symbolData->name);
    } else {
        printf("Add object failed\n");
    }
}

void objectDiv(Object a, Object b) {
    if (a.symbolData && b.symbolData) {
        printf("Div object: %s + %s\n", a.symbolData->name, b.symbolData->name);
        fprintf(tempOut, code("div t0, %%[%s], %%[%s]"), a.symbolData->name, b.symbolData->name);
    } else {
        printf("Div object failed\n");
    }
}

void objectAssignVal(Object a) {
    if (a.symbolData) {
        a.symbolData->write = true;
        printf("Assign object: %s\n", a.symbolData->name);
        fprintf(tempOut, code("MV %%[%s], t0"), a.symbolData->name);
    } else {
        printf("Assign object failed\n");
    }
}

int main(int argc, char* argv[]) {
    char *tempOutputFileName = NULL, *outputFileName = NULL, *outputName = NULL, *inputFileName = NULL;
    if (argc == 3) {
        yyin = fopen(inputFileName = argv[1], "r");
        outputName = argv[2];
    } else if (argc == 2) {
        yyin = stdin;
        outputName = argv[1];
    } else {
        yyin = stdin;
        outputName = "output";
    }
    if (!yyin) {
        printf("file `%s` doesn't exists or cannot be opened\n", inputFileName);
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
        if (obj->symbolData)
            fprintf(tempOut, ":[%s] \"%s\"(%s)\n", (char*)entries->key, obj->symbolData->write ? "+r" : "r", (char*)entries->key);
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