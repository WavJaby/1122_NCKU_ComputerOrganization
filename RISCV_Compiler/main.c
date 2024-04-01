#include "main.h"
#define DEBUG
#define WJCL_LINKED_LIST_IMPLEMENTATION
#define WJCL_HASH_MAP_IMPLEMENTATION
#include "../../WJCL/map/wjcl_hash_map.h"

uint32_t strHash(const char* str) {
    uint32_t hash = 0, seed = 131;
    while (*str)
        hash = hash * seed + (*str++);
    return hash;
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
    Object* obj = (Object*)key;
    if (obj->type == OBJECT_TYPE_STR)
        free((char*)obj->value);
}

NodeInfo funVarInfo = {
    .equalsFunction = objectEquals,
    .hashFunction = objectHash,
    .freeFlag = WJCL_HASH_MAP_FREE_KEY | WJCL_HASH_MAP_FREE_VALUE,
    .onNodeDelete = objectFree,
};
Map funVar;

char* newStrCat(const char* a, const char* b) {
    char* cache = (char*)malloc(strlen(a) + strlen(b) + 1);
    strcpy(cache, a);
    strcat(cache, b);
    return cache;
}

void pushFunVar(ObjectType variableType, const char* variableName) {
}

int main(int argc, char* argv[]) {
    char *outputFileName = NULL, *inputFileName = NULL;
    if (argc == 3) {
        yyin = fopen(inputFileName = argv[1], "r");
        outputFileName = argv[2];
    } else if (argc == 2) {
        yyin = stdin;
        outputFileName = argv[1];
    } else {
        yyin = stdin;
        outputFileName = "output";
    }
    if (!yyin) {
        printf("file `%s` doesn't exists or cannot be opened\n", inputFileName);
        exit(1);
    }
    outputFileName = newStrCat(outputFileName, ".h");
    fout = fopen(outputFileName, "w");
    if (!fout) {
        printf("file `%s` cannot be opened\n", outputFileName);
        exit(1);
    }
    funVar = map_create(funVarInfo);

    fclose(fout);

    if (compileError)
        remove(outputFileName);
    free(outputFileName);
    yylex_destroy();
    return 0;
}