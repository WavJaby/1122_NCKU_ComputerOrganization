#include <stdio.h>
int main(){
	int a, b;
	FILE *input = fopen("../input/1.txt", "r");
	fscanf(input, "%d %d", &a, &b);
	fclose(input);
	//a = a / b;
	asm volatile(
//########## Generate by RISC-V compiler ##########
"div t0, %[a], %[b]\n\t"
"MV %[a], t0\n\t"
:[a] "+r"(a)
:[b] "r"(b)
//##########     Compiler by WavJaby     ##########
);
	printf("%d\n", a);
	return 0;
}


