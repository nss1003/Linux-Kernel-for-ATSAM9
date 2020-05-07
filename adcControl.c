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
typedef unsigned char U_C;

#define PMC_PCER_OFFSET 0x10 // PMC Peripheral Clock Enable Register
#define ADC_CR_OFFSET 0x00	//	ADC Control Register
#define ADC_MR_OFFSET 0x04	//	ADC Mode Register
#define ADC_CHER_OFFSET 0x10 // ADC Channel Enable Register
#define ADC_CHSR_OFFSET 0x18 // Channel Status Register
#define ADC_SR_OFFSET 0x1C	//  ADC Status Register
#define ADC_CDR0_OFFSET 0x30 // Channel Data Register 0
#define ADC_CDR1_OFFSET 0x34 // Channel Data Register 1

#define CST(addr) *((volatile U_L *)(addr))

// Busy wait
void waits(){
	int i;
	for(i = 0; i < 20000000; i++){
	}
}

U_L* getVAddr(int size, U_C offset){

	/*
	* First map the memory by converting physical addresses to virtual one
	* this allow the MMU access the control and mode register of the AT91SAM9260
	* 
	* ADC base address = 0xFFFE0000
	* Size = 16 Kbyte = 16384 byte
	* Read man mmap for more information
	*
	*/
	U_L* vAddr;
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	if(fd < 0){
		printf("Can't open /dev/mem \n");
		return 1;
	}
	
	vAddr = (U_L *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset);
	if(vAddr == NULL){
		printf("Can't mmap \n");
		return 1;
	}
	return vAddr;
}

void initAdc(){
	int i = 0;
	U_L result;

	U_L *adcVAddr = getVAddr(16384, 0xFFFE0000);
	U_L *pmcVAddr = getVAddr(256, 0xFFFFFC00);
	printf("Memory mapped \n");
	
	printf("ADC Virtual address: %p \n",adcVAddr);
	printf("PMC Virtual address: %p \n",pmcVAddr);
	// Enable clock for ADC interface page 32 and 217
	// Segmentation fault also with mmap
	// 0x64512 address of pmc + 5 ==> PID5
	CST(pmcVAddr + PMC_PCER_OFFSET) = 1 << 5; // Enable PID5 ==> ADC
	
	printf("Init\n");
	// Reset ADC page 709
	CST(adcVAddr + ADC_CR_OFFSET) = 1 << 0;
	CST(adcVAddr + ADC_CR_OFFSET) = 1 << 1;

	// Set Prescaler, startup time and sample hold time page 710
	/*
	*Prescaler possible values (1, 2, 4, 8, 16, 32, 64)
	*MCK values on page 432 (I choose 4915200 MHz)
	*ADCClock = MCK/((Prescal + 1)*2) = 38400 MHz
	*Startup time = (startup + 1)*8/ADCClock = 
	*SHTIM = 0F
	*STARTUP = 1F page 16
	*PRESCAL = 3F	
	*10 Bit resolution
	*Normal Mode
	*/
	CST(adcVAddr + ADC_MR_OFFSET) = 0x0F1F0F00;
}

U_L readAdc(char channel){
	
	U_L result;
	int i = 0;
	printf("Convert \n");
	
	U_L *adcVAddr = getVAddr(16484, 0xFFFE0000);
	// Enable disired channel
	CST(adcVAddr + ADC_CHER_OFFSET) = 1 << channel;
	
	// Start convertion
	//a_pADC->ADC_CR = 0x2;
	CST(adcVAddr + ADC_CR_OFFSET) = 1 << 1; 

	while(!(CST(adcVAddr + ADC_SR_OFFSET) & channel)){ // Read page 714 for more details
	// Just to check if the conversion remain. Break after 3 steps		
	printf("Still in convertion!\n");
		i++;
		if(i == 3)
			break;
	}
	
	U_L status = (U_L)CST(adcVAddr + ADC_CHSR_OFFSET);
	printf("Channel Status = %ld\n", status);

	switch(channel){
		case '0': 
			result = CST(adcVAddr + ADC_CDR0_OFFSET);
			break;
		case '1':
			result = CST(adcVAddr + ADC_CDR1_OFFSET);
			break;
		default:
			printf("Wrong entry\n");
			printf("Code stop!");
			break;
	}
	return result;
}


int  main(){

	char channel;
	U_L readValue = 0;
	float analogVoltage = 0, vRef = 3.3;
	int count = 0;
	U_L *vAddr;

	initAdc();
	printf("Wich channel should be read ? \n");
	printf("0 for channel 0 and 1 for channel 1: ");
	
	scanf("%c",&channel);

	initAdc(vAddr);

	while(count != 10){
	
		printf("\nConvertion starts \n");
		
		readValue = readAdc(channel);

		analogVoltage = (readValue * vRef) / 1024;
		printf(" \n");
		printf("**************************************************\n");
		printf("* Digital result      ||      Analog Voltage     *\n");
		printf("**************************************************\n");
		printf("*                                                *\n");
		printf("*    %ld        ||      %f V   *\n",readValue, analogVoltage);
		printf("*                                                *\n");
		printf("**************************************************\n");
		printf(" \n");
		
		count++;
		// Wait for next value to be read
		waits();
	}
}
