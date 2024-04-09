#include "compiler_common.h"

#define getRegIndex(arr, index, useReg, maxIndex, name) \
    if (!arr[index]) {                                  \
        arr[index] = useReg;                            \
        if (index > maxIndex) maxIndex = index;         \
        return name[index];                             \
    }

bool usedTmpReg[7];
char tmpRegName[][3] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "ERR"};
int maxTmpRegIndex = -1;

void freeTmpReg(Object* regObj) {
    if (regObj->symbol->name[1] < '0' || regObj->symbol->name[1] > '6')
        fprintf(stderr, "Failed to free temp register '%s'\n", regObj->symbol->name);
    else
        usedTmpReg[regObj->symbol->name[1] - '0'] = false;
}

char* getTmpReg(bool useReg) {
    getRegIndex(usedTmpReg, 0, useReg, maxTmpRegIndex, tmpRegName);
    getRegIndex(usedTmpReg, 1, useReg, maxTmpRegIndex, tmpRegName);
    getRegIndex(usedTmpReg, 2, useReg, maxTmpRegIndex, tmpRegName);
    getRegIndex(usedTmpReg, 3, useReg, maxTmpRegIndex, tmpRegName);
    getRegIndex(usedTmpReg, 4, useReg, maxTmpRegIndex, tmpRegName);
    getRegIndex(usedTmpReg, 5, useReg, maxTmpRegIndex, tmpRegName);
    getRegIndex(usedTmpReg, 6, useReg, maxTmpRegIndex, tmpRegName);
    fprintf(stderr, "Temp register out of range\n");
    return tmpRegName[7];
}