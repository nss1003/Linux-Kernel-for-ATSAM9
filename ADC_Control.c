
/*
LED ansteuern


PIOB_BASE = 0xFFFFF600

PIOB_BASE + PIO_PER |= 0x00000001 // enable PB0 Page 344
PIOB_BASE + PIO_OER |= 0x00000001 // configure as output page 345
PIOB_BASE + PIO_SODR |= 0x00000001 // set PB0 to 1 page 348


*/


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


#define ADC_BASE  getVAddr(16384, 0xFFFE0000)
#define SYSC_BASE  getVAddr(16384, 0xFFFFC000)


#define PIOB_ADDR  0x3600
#define PIOC_ADDR  0x3800
#define PIOC_ODR  0x14
#define PIOC_PER  0x00

#define PMC_ADDR  0x3C00
#define PMC_PCER 0x10 // PMC Peripheral Clock Enable Register

#define ADC_CR 0x00	//	ADC Control Register
#define ADC_MR 0x04	//	ADC Mode Register
#define ADC_CHER 0x10 // ADC Channel Enable Register
#define ADC_CHSR 0x18 // Channel Status Register
#define ADC_SR 0x1C	//  ADC Status Register
#define ADC_CDR0 0x30 // Channel Data Register 0
#define ADC_CDR1 0x34 // Channel Data Register 1

//int* ADC_BASE = getVAddr(16384, 0xFFFE0000);
//int* SYSC_BASE = getVAddr(16384, 0xFFFFC000);

/*
AT91PS_ADC ADC = ADC_BASE;
AT91PS_PMC PMC = (SYSC_BASE + PMC_ADDR);
AT91PS_PIO PIOB = (SYSC_BASE + PIOB_ADDR);
AT91PS_PIO PIOC = (SYSC_BASE + PIOC_ADDR);
*/

// Busy wait
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

void initAdc(){
	int i = 0;

	printf("ADC Virtual address: %p \n",ADC_BASE);
	printf("PMC Virtual address: %p \n",SYSC_BASE);
	// Enable clock for ADC interface page 32 and 217
	// Segmentation fault also with mmap
	// 0x64512 address of pmc + 5 ==> PID5
	//CST(SYSC_BASE + PMC_ADDR + PMC_PCER) = 1 << 5; // Enable PID5 ==> ADC

	printf("Init\n");
	// Reset ADC page 709
	CST(ADC_BASE + ADC_CR) = 1 << 0;
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
	CST(ADC_BASE + ADC_MR) |= 0x3F << 8;
	CST(ADC_BASE + ADC_MR) |= 0x1F << 16;
	CST(ADC_BASE + ADC_MR) |= 0xF << 24;

	printf("Init complete\n");
}


U_L readAdc(int channel){
	U_L result;
	int i = 0;
	printf(" \n");

	// Enable disired channel
	CST(ADC_BASE + ADC_CHER) = 1 << channel;

	// Start convertion
	CST(ADC_BASE + ADC_CR) = 1 << 1;

	while(!(CST(ADC_BASE + ADC_SR) & (1 << channel))){ // Read page 714 for more details
	// Just to check if the conversion remain. Break after 3 steps
	printf("Still in convertion!\n");
		i++;
		if(i == 3)
			break;
	}

	switch(channel){
		case 0:
			result = CST(ADC_BASE + ADC_CDR0);
			break;
		case 1:
			result = CST(ADC_BASE + ADC_CDR1);
			break;
		default:
			printf("Wrong entry\n");
			printf("Code stop!");
			return -1;
	}
	return result;
}

int  main(void){

	printf("Start\n");

/*
	CST(SYSC_BASE + PIOC_ADDR + PIOC_ODR) |= 0x01;// for AD0 as input Page 346
	CST(SYSC_BASE + PIOC_ADDR + PIOC_PER) |= 0x01; 	// Enable AD0

	CST(SYSC_BASE + PIOC_ADDR + PIOC_ODR) |= 0x10; // for AD1 as input
	CST(SYSC_BASE + PIOC_ADDR + PIOC_PER) |= 0x10; 	// Enable AD1
*/

	int channel;
	U_L readValue = 0;
	float analogVoltage = 0, vRef = 3.3;
	int count = 0;

	initAdc();
	printf("Wich channel should be read ? \n");
	printf("0 for channel 0 and 1 for channel 1: ");

	scanf("%d",&channel);

	while(count != 10){

		printf("\nConvertion starts \n");

		readValue = readAdc(channel);
		if(readValue < 0){
			break;
		}
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
}


