#include<stdio.h>
int main()
{ 
    int i = 0;
    int h[9] = {0}, x[6] = {0}, y[6] = {0}; 
    FILE *input = fopen("../input/4.txt", "r");
    for(i = 0; i < 9; i++) fscanf(input, "%d", &h[i]);
    for(i = 0; i < 6; i++) fscanf(input, "%d", &x[i]);
    for(i = 0; i < 6; i++) fscanf(input, "%d", &y[i]);
    fclose(input);
    
    int *p_x = &x[0];
    int *p_h = &h[0];
    int *p_y = &y[0];
    
    asm volatile(
//########## Generate by RISC-V compiler ##########
"addi %[i], zero, 0\n\t"  // i <= 0
"j EL0\n\t"  // for loop (0) jump to 'ENTRY'
"L0:\n\t"
// for loop (0) CONTNET
    "addi t0, zero, 0\n\t"
    "sw t0, -20(s0)\n\t"  // *s0 <= 0
    "j EL1\n\t"  // for loop (1) jump to 'ENTRY'
    "L1:\n\t"
// for loop (1) CONTNET
        "addi t0, zero, 0\n\t"
        "sw t0, -24(s0)\n\t"  // *s0 <= 0
        "j EL2\n\t"  // for loop (2) jump to 'ENTRY'
        "L2:\n\t"
// for loop (2) CONTNET
            "addi t0, zero, 3\n\t"
            "MUL t0, %[i], t0\n\t"  // i * 3 => _t0
            "lw t1, -24(s0)\n\t"  // *s0
            "ADD t0, t0, t1\n\t"  // _t0 + *s0 => _t0
            "addi t1, zero, 4\n\t"
            "MUL t0, t0, t1\n\t"  // _t0 * 4 => _t0
            "ADD t0, %[h], t0\n\t"  // h + _t0 => _t0
            "mv %[p_h], t0\n\t"  // p_h <= _t0
            "addi t0, zero, 2\n\t"
            "lw t1, -24(s0)\n\t"  // *s0
            "MUL t0, t1, t0\n\t"  // *s0 * 2 => _t0
            "lw t1, -20(s0)\n\t"  // *s0
            "ADD t0, t1, t0\n\t"  // *s0 + _t0 => _t0
            "addi t1, zero, 4\n\t"
            "MUL t0, t0, t1\n\t"  // _t0 * 4 => _t0
            "ADD t0, %[x], t0\n\t"  // x + _t0 => _t0
            "mv %[p_x], t0\n\t"  // p_x <= _t0
            "addi t0, zero, 2\n\t"
            "MUL t0, %[i], t0\n\t"  // i * 2 => _t0
            "lw t1, -20(s0)\n\t"  // *s0
            "ADD t0, t0, t1\n\t"  // _t0 + *s0 => _t0
            "addi t1, zero, 4\n\t"
            "MUL t0, t0, t1\n\t"  // _t0 * 4 => _t0
            "ADD t0, %[y], t0\n\t"  // y + _t0 => _t0
            "mv %[p_y], t0\n\t"  // p_y <= _t0
            "lw t0, 0(%[p_h])\n\t"  // *p_h
            "lw t1, 0(%[p_x])\n\t"  // *p_x
            "MUL t0, t0, t1\n\t"  // *p_h * *p_x => _t0
            "lw t1, 0(%[p_y])\n\t"  // *p_y
            "ADD t0, t1, t0\n\t"  // *p_y + _t0 => _t0
            "sw t0, 0(%[p_y])\n\t"  // *p_y <= _t0
// for loop (2) CONTNET END
            "addi t0, zero, 1\n\t"
            "lw t1, -24(s0)\n\t"  // *s0
            "ADD t0, t1, t0\n\t"  // *s0 + 1 => _t0
            "sw t0, -24(s0)\n\t"  // *s0 <= _t0
        "EL2:\n\t"  // for loop (2) ENTRY
            "addi t0, zero, 3\n\t"
            "lw t1, -24(s0)\n\t"  // *s0
            "SLT t0, t1, t0\n\t"  // *s0 < 3 => _t0
            "bne t0, zero, L2\n\t"  // for loop (2) jump to 'CONTNET'

// for loop (1) CONTNET END
        "addi t0, zero, 1\n\t"
        "lw t1, -20(s0)\n\t"  // *s0
        "ADD t0, t1, t0\n\t"  // *s0 + 1 => _t0
        "sw t0, -20(s0)\n\t"  // *s0 <= _t0
    "EL1:\n\t"  // for loop (1) ENTRY
        "addi t0, zero, 2\n\t"
        "lw t1, -20(s0)\n\t"  // *s0
        "SLT t0, t1, t0\n\t"  // *s0 < 2 => _t0
        "bne t0, zero, L1\n\t"  // for loop (1) jump to 'CONTNET'

// for loop (0) CONTNET END
    "addi t0, zero, 1\n\t"
    "ADD t0, %[i], t0\n\t"  // i + 1 => _t0
    "mv %[i], t0\n\t"  // i <= _t0
"EL0:\n\t"  // for loop (0) ENTRY
    "addi t0, zero, 3\n\t"
    "SLT t0, %[i], t0\n\t"  // i < 3 => _t0
    "bne t0, zero, L0\n\t"  // for loop (0) jump to 'CONTNET'

: [p_x] "+r"(p_x), [p_h] "+r"(p_h), [p_y] "+r"(p_y), [i] "+r"(i)
: [h] "r"(h), [x] "r"(x), [y] "r"(y)
: "t0", "t1"
//##########     Compiler by WavJaby     ##########
);

    p_y = &y[0];
    for(i = 0; i < 6; i++)
        printf("%d ", *p_y++);
    printf("\n");
    return 0; 
 
}
