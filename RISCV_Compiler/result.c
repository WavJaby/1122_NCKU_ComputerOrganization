#include<stdio.h>
int main()
{ 
    int f, i, j;
    int h[9] = {0}, x[6] = {0}, y[6] = {0}; 
    FILE *input = fopen("CO2024HW1/input/3.txt", "r");
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
"lw t0, 0(%[p_h])\n\t"
"lw t1, 0(%[p_x])\n\t"
"MUL t3, t0, t1\n\t"
"lw t0, 4(%[p_h])\n\t"
"lw t1, 8(%[p_x])\n\t"
"MUL t4, t0, t1\n\t"
"ADD t3, t3, t4\n\t"
"lw t0, 8(%[p_h])\n\t"
"lw t1, 16(%[p_x])\n\t"
"MUL t4, t0, t1\n\t"
"ADD t3, t3, t4\n\t"
"sw t3, 0(%[p_y])\n\t"
"lw t0, 0(%[p_h])\n\t"
"lw t1, 4(%[p_x])\n\t"
"MUL t3, t0, t1\n\t"
"lw t0, 4(%[p_h])\n\t"
"lw t1, 12(%[p_x])\n\t"
"MUL t4, t0, t1\n\t"
"ADD t3, t3, t4\n\t"
"lw t0, 8(%[p_h])\n\t"
"lw t1, 20(%[p_x])\n\t"
"MUL t4, t0, t1\n\t"
"ADD t3, t3, t4\n\t"
"sw t3, 4(%[p_y])\n\t"
"lw t0, 12(%[p_h])\n\t"
"lw t1, 0(%[p_x])\n\t"
"MUL t3, t0, t1\n\t"
"lw t0, 16(%[p_h])\n\t"
"lw t1, 8(%[p_x])\n\t"
"MUL t4, t0, t1\n\t"
"ADD t3, t3, t4\n\t"
"lw t0, 20(%[p_h])\n\t"
"lw t1, 16(%[p_x])\n\t"
"MUL t4, t0, t1\n\t"
"ADD t3, t3, t4\n\t"
"sw t3, 8(%[p_y])\n\t"
"lw t0, 12(%[p_h])\n\t"
"lw t1, 4(%[p_x])\n\t"
"MUL t3, t0, t1\n\t"
"lw t0, 16(%[p_h])\n\t"
"lw t1, 12(%[p_x])\n\t"
"MUL t4, t0, t1\n\t"
"ADD t3, t3, t4\n\t"
"lw t0, 20(%[p_h])\n\t"
"lw t1, 20(%[p_x])\n\t"
"MUL t4, t0, t1\n\t"
"ADD t3, t3, t4\n\t"
"sw t3, 12(%[p_y])\n\t"
"lw t0, 24(%[p_h])\n\t"
"lw t1, 0(%[p_x])\n\t"
"MUL t3, t0, t1\n\t"
"lw t0, 28(%[p_h])\n\t"
"lw t1, 8(%[p_x])\n\t"
"MUL t4, t0, t1\n\t"
"ADD t3, t3, t4\n\t"
"lw t0, 32(%[p_h])\n\t"
"lw t1, 16(%[p_x])\n\t"
"MUL t4, t0, t1\n\t"
"ADD t3, t3, t4\n\t"
"sw t3, 16(%[p_y])\n\t"
"lw t0, 24(%[p_h])\n\t"
"lw t1, 4(%[p_x])\n\t"
"MUL t3, t0, t1\n\t"
"lw t0, 28(%[p_h])\n\t"
"lw t1, 12(%[p_x])\n\t"
"MUL t4, t0, t1\n\t"
"ADD t3, t3, t4\n\t"
"lw t0, 32(%[p_h])\n\t"
"lw t1, 20(%[p_x])\n\t"
"MUL t4, t0, t1\n\t"
"ADD t3, t3, t4\n\t"
"sw t3, 20(%[p_y])\n\t"
"addi %[i], zero, 3\n\t"
"addi %[j], zero, 2\n\t"
"addi %[f], zero, 3\n\t"
: [f] "+r"(f), [p_y] "+r"(p_y), [i] "+r"(i), [j] "+r"(j)
: [p_x] "r"(p_x), [p_h] "r"(p_h), [h] "r"(h), [x] "r"(x), [y] "r"(y)
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
