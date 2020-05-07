#include "AT91SAM9260.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

typedef unsigned long U_L;

#define CST(addr) *((U_L*)(addr))

#define ADC_BASE 
#define SYS_BASE 

//Busy wait
void busyWait(){
	int i;
	for(i = 0; i < 20000000; i++){}
}

// Convert physical addresses to Virtual addresses
int *getVirtualAddr(int size, int offset){

	void *virtualAddr;
	int fd;
	fd = open("/dev/mem", O_RDWR|O_SYNC);
	if(fd < 0){
		printf("Can't open /dev/mem \n");
		return (int*)-1;
	}

	virtualAddr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset);

	if(virtualAddr == NULL){
		printf("Can't mmap \n");
		return (int*)-1;
	}
	return virtualAddr;
}


int main(){
	
	//ADC block size: 16K bytes, offset: 0xFFFE 0000 
	//SYSC block size: 16K bytes, offser: 0xFFFF C000 

	int *adcAddr = getVirtualAddr(16384, 0xFFFE0000);
	int *pmcAddr = getVirtualAddr(16384, 0xFFFFC000);

	printf("ADC Virtual address: %p\n", adcAddr);
	printf("PMC Virtual address: %p\n", pmcAddr);

	return 0;
}
