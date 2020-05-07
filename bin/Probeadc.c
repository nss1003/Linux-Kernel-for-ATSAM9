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
#include <stdbool.h> 


typedef unsigned long U_L;

#define PIOB_ADDR  0x3600
#define PIOC_ADDR  0x3800
#define PIOC_ODR  0x14
#define PIOC_PER  0x00

#define SYSC_ADDR = 0x1C00
#define PMC_ADDR  0x3C00
#define PMC_PCER 0x10 // PMC Peripheral Clock Enable Register

#define ADC_CR 0x00	//	ADC Control Register
#define ADC_MR 0x04	//	ADC Mode Register
#define ADC_CHER 0x10 // ADC Channel Enable Register
#define ADC_CHSR 0x18 // Channel Status Register
#define ADC_SR 0x1C	//  ADC Status Register
#define ADC_CDR0 0x30 // Channel Data Register 0
#define ADC_CDR1 0x34 // Channel Data Register 1


// Busy wait
void waits(){
	int i;
	for(i = 0; i < 20000000; i++){
	}
}

/*  SYSC address is wrong the size should be 16K Bytes(0x4000)
	According to memory map sysc starts by by 0xFFFF C000 and ends by 0xFFFF FFFF
	but 0xFFFF FFFF - 0xFFFF C0000 = 0x3FFF different as 0x4000 
*/

void initAdc(volatile unsigned* adc_base, volatile unsigned* sysc_base){

	printf("ADC Virtual address: %p \n",adc_base);
	// After convertion ADC Virtual addr = 0x40161000
	// After convertion SYSC Virtual addr = 0x40165000

	// Enable clock for ADC interface page 32 and 217
	// Segmentation fault also with mmap
	// 0x64512 address of pmc + 5 ==> PID5
	*(sysc_base + 0x2800 + PMC_PCER) = 1 << 5; // Enable PID5 ==> ADC

	printf("Init PMC Complete\n");
	// Reset ADC page 709
	*(adc_base + ADC_CR) = 1 << 0;
	//CST(ADC->ADC_CR) = 1 << 1;

	// Set Prescaler, startup time and sample hold time page 710
	/*
	*Prescaler possible values (1, 2, 4, 8, 16, 32, 64)
	*MCK values on page 432 (I choose 4915200 MHz)
	*ADCClock = MCK/((Prescal + 1)*2) = 38400 MHz
	*Startup time = (startup + 1)*8/ADCClock =
	*SHTIM = F
	*STARTUP = 1F page 16
	*PRESCAL = 3F
	*10 Bit resolution
	*Normal Mode
	*/
	*(adc_base + ADC_MR) |= 0x3F << 8;
	*(adc_base + ADC_MR) |= 0x1F << 16;
	*(adc_base + ADC_MR) |= 0xF << 24;

	printf("Init ADC complete\n");
}


U_L readAdc(int channel, volatile unsigned* adc_base){
	U_L result;
	int i = 0;
	printf(" \n");

	// Enable disired channel
	*(adc_base + ADC_CHER) = 1 << channel;

	// Start convertion
	*(adc_base + ADC_CR) = 1 << 1;

	while(!(*(adc_base + ADC_SR) & (1 << channel))){ // Read page 714 for more details
	// Just to check if the conversion remain. Break after 3 steps
	printf("Still in convertion!\n");
		i++;
		if(i == 3)
			break;
	}

	switch(channel){
		case 0:
			result = *(adc_base + ADC_CDR0);
			break;
		case 1:
			result = *(adc_base + ADC_CDR1);
			break;
	}
	return result;
}

int  main(void){

	printf("Start\n");

	int channel;
	U_L readValue = 0;
	float analogVoltage = 0, vRef = 3.3;
	int count = 0;
	int flag = 0;

	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	
	if(fd < 0){
		printf("Can't open /dev/mem \n");
		return -1;
	}
	
	void* adc_vaddr = mmap(NULL, 16384, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0xFFFE0000);
	void* sysc_vaddr = mmap(NULL, 16384, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0xFFFFE800);

	if(adc_vaddr == NULL){
		printf("Maping failed\n");
		return -1;
	}

	volatile unsigned* adc_base = (unsigned*)adc_vaddr;
	volatile unsigned* sysc_base = (unsigned*)sysc_vaddr;

	initAdc(adc_base, sysc_base);

	printf("Wich channel should be read ? \n");
	printf("0 for channel 0 and 1 for channel 1: ");

	scanf("%i",&channel);
	if((channel < 0) || (channel > 1)){
		printf("Wrong channel\n");
		flag = 1;	
	}

	while((count != 5) && (flag == 0)){
		

		readValue = readAdc(channel, adc_base);

		printf("\nReading ADC: %d \n",count);

		if(readValue < 0)
			//printf("Negative result");
			break;
		else{

			analogVoltage = (readValue * vRef) / 1024;
			printf(" \n");
			printf("**************************************************\n");
			printf("* Digital result      ||      Analog Voltage     *\n");
			printf("**************************************************\n");
			printf("*                                                *\n");
			printf("*    %d        ||      %f V   *\n",readValue, analogVoltage);
			printf("*                                                *\n");
			printf("**************************************************\n");
			printf(" \n");
			count++;
			// Wait for next value to be read
			waits();
		}
	}
	flag = 0;

	munmap(adc_vaddr, 16384);
}


