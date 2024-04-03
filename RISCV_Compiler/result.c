#include <stdio.h>
int main() {
    int f, i, j;
    int h[9] = {0}, x[6] = {0}, y[6] = {0};
    FILE *input = fopen("CO2024HW1/input/3.txt", "r");
    for (i = 0; i < 9; i++) fscanf(input, "%d", &h[i]);
    for (i = 0; i < 6; i++) fscanf(input, "%d", &x[i]);
    for (i = 0; i < 6; i++) fscanf(input, "%d", &y[i]);
    fclose(input);
    int *p_x = &x[0];
    int *p_h = &h[0];
    int *p_y = &y[0];
    i = 10;
    asm volatile(
//########## Generate by RISC-V compiler ##########
"mv 0(%[p_y]), %[i]\n\t"
: [p_y] "+r"(p_y)
: [p_x] "r"(p_x), [p_h] "r"(p_h), [f] "r"(f), [i] "r"(i), [j] "r"(j)
//##########     Compiler by WavJaby     ##########
);
    // p_y = &y[0];
    // for(i = 0; i < 6; i++)
    // printf("%d ", *p_y++);
    // printf("\n");
    printf("%d\n", x[0]);
    return 0;
}
