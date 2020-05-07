#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define PMC_PCER 0x10 

#define ADC_CR 0x00	//	ADC Control Register
#define ADC_MR 0x04	//	ADC Mode Register
#define ADC_CHER 0x10 // ADC Channel Enable Register
#define ADC_CHSR 0x18 // Channel Status Register
#define ADC_SR 0x1C	//  ADC Status Register
#define ADC_CDR0 0x30 // Channel Data Register 0
#define ADC_CDR1 0x34 

int main(){
	
	int result;
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	
	if(fd < 0){
		printf("Can't open /dev/mem \n");
		return -1;
	}
	
	void* adc_vaddr = mmap(NULL, 131072, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0xFFFFE800);

	if(adc_vaddr == NULL){
		printf("Maping failed\n");
		return -1;
	}


	volatile unsigned* adc_base = (unsigned*)adc_vaddr;
	*(adc_base) = 1 << 5;

	printf("pmc ok\n");

	munmap(adc_vaddr, 131072);
}
