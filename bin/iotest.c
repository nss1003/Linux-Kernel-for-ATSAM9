#include <sys/io.h>
#include <stdio.h>

int main(){

	long base = 0x40161000;
	int value = 5;
	
	printf("Base & value ok\n");
	ioperm(base, 1, 1);

	printf("Permission ok\n");
	outl(value, base);

	printf("Write ok");
	ioperm(base, 1, 0);

	return 0;
}
