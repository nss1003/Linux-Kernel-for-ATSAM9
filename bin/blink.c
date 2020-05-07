#include "AT91SAM9260.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
//#include "at91sam9260_adc.h"
#include <unistd.h>

typedef unsigned long U_L;
#define CST(addr) *((U_L*)(addr))

//#define SYSC_BASE  getVAddr(16384, 0xFFFFC000)
//PIOB_BASE = 0xFFFFF600
#define PIOB_ADDR  0x3600
#define PIO_ODR  0x0014
#define PIO_PER  0x0000
#define PIO_SODR 0x0030
#define PIO_OER  0x0010
#define PIO_OSR  0x0018

void waits(){
	int i;
	for(i = 0; i < 20000000; i++){
	}
}

int *getVAddr(int size, char offset){

	/*
	* First map the memory by converting physical addresses to virtual one
	* this allow the MMU access the control and mode register of the AT91SAM9260
	* 
	* ADC base address = 0xFFFE0000
	* Size = 16 Kbyte = 16384 byte
	* Read man mmap for more information
	*
	*/
	void *vAddr;
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	if(fd < 0){
		printf("Can't open /dev/mem \n");
		return (int*)-1;
	} 
	
	vAddr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset);
	if(vAddr == NULL){
		printf("Can't mmap \n");
		return (int*)-1;
	}
	return vAddr;

}

int main(){

	U_L SYSC_BASE = getVAddr(16384, 0xFFFFC000);

	CST(SYSC_BASE + PIOB_ADDR + PIO_PER) |= 0x0000FFFF; // enable PB0 Page 344
	CST(SYSC_BASE + PIOB_ADDR + PIO_OER) |= 0x000000FF; // configure as output page 345
	
	int i = 0;
	while(i != 5){

		CST(SYSC_BASE + PIOB_ADDR + PIO_SODR) |= 0x00000001; // set PB0 to 1 page 348
		printf("Pin ON \n");
		printf("Status Register = %d\n", PIO_OSR);
		waits();
		CST(SYSC_BASE + PIOB_ADDR + PIO_SODR) &= 0x00000000; // set PB0 to 1 page 348
		printf("Pin OFF \n");
		waits();

		i++;
	}
}
