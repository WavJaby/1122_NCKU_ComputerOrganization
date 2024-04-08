#include<stdio.h>
int main()
{ 
    int f, i, j;
    int h[9] = {0}, x[6] = {0}, y[6] = {0}; 
    FILE *input = fopen("../input/3.txt", "r");
    for(i = 0; i < 9; i++) fscanf(input, "%d", &h[i]);
    for(i = 0; i < 6; i++) fscanf(input, "%d", &x[i]);
    for(i = 0; i < 6; i++) fscanf(input, "%d", &y[i]);
    fclose(input);
    int *p_x = &x[0] ;
    int *p_h = &h[0] ;
    int *p_y = &y[0] ;
    for (i = 0; i < 3; i++){ 
        for (j = 0; j < 2; j++){        	
            for (f = 0; f < 3; f++)
                asm volatile(
//########## Generate by RISC-V compiler ##########
"addi t0, zero, 3\n\t"  // t0 <= 3
"MUL t3, %[i], t0\n\t"  // i * 3 => _t3
"ADD t3, t3, %[f]\n\t"  // _t3 + f => _t3
"addi t0, zero, 4\n\t"  // t0 <= 4
"MUL t3, t3, t0\n\t"  // _t3 * 4 => _t3
"ADD t4, %[h], t3\n\t"  // h + _t3 => _t4
"mv %[p_h], t4\n\t"  // p_h <= _t4
"addi t0, zero, 2\n\t"  // t0 <= 2
"MUL t3, %[f], t0\n\t"  // f * 2 => _t3
"ADD t4, %[j], t3\n\t"  // j + _t3 => _t4
"addi t0, zero, 4\n\t"  // t0 <= 4
"MUL t3, t4, t0\n\t"  // _t4 * 4 => _t3
"ADD t4, %[x], t3\n\t"  // x + _t3 => _t4
"mv %[p_x], t4\n\t"  // p_x <= _t4
"addi t0, zero, 2\n\t"  // t0 <= 2
"MUL t3, %[i], t0\n\t"  // i * 2 => _t3
"ADD t3, t3, %[j]\n\t"  // _t3 + j => _t3
"addi t0, zero, 4\n\t"  // t0 <= 4
"MUL t3, t3, t0\n\t"  // _t3 * 4 => _t3
"ADD t4, %[y], t3\n\t"  // y + _t3 => _t4
"mv %[p_y], t4\n\t"  // p_y <= _t4
"lw t0, 0(%[p_h])\n\t"  // *p_h
"lw t1, 0(%[p_x])\n\t"  // *p_x
"MUL t3, t0, t1\n\t"  // *p_h * *p_x => _t3
"lw t0, 0(%[p_y])\n\t"  // *p_y
"ADD t4, t0, t3\n\t"  // *p_y + _t3 => _t4
"sw t4, 0(%[p_y])\n\t"  // _t4 => *p_y
  // *p_y <= _t4
: [p_x] "+r"(p_x), [p_h] "+r"(p_h), [p_y] "+r"(p_y)
: [f] "r"(f), [h] "r"(h), [x] "r"(x), [i] "r"(i), [y] "r"(y), [j] "r"(j)
//##########     Compiler by WavJaby     ##########
);
	}
    }
    p_y = &y[0];
    for(i = 0; i < 6; i++)
    printf("%d ", *p_y++);
    printf("\n");
    return 0; 
 
}
