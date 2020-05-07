r#include "AT91SAM9260.h"
//#include "AT91SAM9260_inc.h"
#include "test.h"

#include "bits.h"
#define AT91B_SLOW_OSC         (32768)                   // Slow clock
#define AT91B_MAIN_OSC         (18432000)                // Exetrnal Main Oscillator MAINCK
#define AT91B_PROCESSOR_CLOCK  ((AT91B_MAIN_OSC/16)*173) // Output PLL Clock (100 MHz)
#define AT91B_MASTER_CLOCK     (AT91B_PROCESSOR_CLOCK/2) // Output PLL Clock (100 MHz)
#define AT91B_DBGU_BAUD_RATE    115200

#define __inline inline
#include "lib_AT91SAM9260.h"


volatile unsigned int dly = 0;
unsigned int val = 0;
unsigned int Tick = 0;

AT91PS_PIO      p_pPioA     = AT91C_BASE_PIOA;
AT91PS_PIO      p_pPioB     = AT91C_BASE_PIOB;
AT91PS_PIO      p_pPioC     = AT91C_BASE_PIOC;
AT91PS_PMC      p_pPMC      = AT91C_BASE_PMC;
AT91PS_RTTC     p_pRTTC     = AT91C_BASE_RTTC;


void AT91F_UART1_Init (void)
{
	// Configure DBGU
    AT91F_US_ResetRx((AT91PS_USART)AT91C_BASE_US1);
    AT91F_US_Configure(
		(AT91PS_USART)AT91C_BASE_US1, // DBGU base address
		AT91B_MASTER_CLOCK,            // 100 MHz
		AT91C_US_ASYNC_MODE,           // mode Register to be programmed
		AT91B_DBGU_BAUD_RATE,          // baudrate to be programmed
		0                              // timeguard to be programmed
	);
	// Open PIO for DBGU
    AT91F_US1_CfgPIO();
	// Enable Transmitter
		AT91F_US_DisableTx((AT91PS_USART)AT91C_BASE_US1);
		AT91F_US_DisableRx((AT91PS_USART)AT91C_BASE_US1);
		AT91F_US_ResetTx((AT91PS_USART)AT91C_BASE_US1);
		AT91F_US_ResetRx((AT91PS_USART)AT91C_BASE_US1);
    AT91F_US_EnableTx((AT91PS_USART)AT91C_BASE_US1);
		AT91F_US_EnableRx((AT91PS_USART)AT91C_BASE_US1);
}

void AT91F_UART1_Printk(	char *buffer) // \arg pointer to a string ending by \0
{
	while(*buffer != '\0') {
		while (!AT91F_US_TxReady((AT91PS_USART)AT91C_BASE_US1));
		AT91F_US_PutChar((AT91PS_USART)AT91C_BASE_US1, *buffer++);
	}
}


void AT91F_DBGU_Init (void)
{
	// Configure DBGU
    AT91F_US_ResetRx((AT91PS_USART)AT91C_BASE_DBGU);
    AT91F_US_Configure(
		(AT91PS_USART)AT91C_BASE_DBGU, // DBGU base address
		AT91B_MASTER_CLOCK,            // 100 MHz
		AT91C_US_ASYNC_MODE,           // mode Register to be programmed
		AT91B_DBGU_BAUD_RATE,          // baudrate to be programmed
		0                              // timeguard to be programmed
	);
	// Open PIO for DBGU
    AT91F_DBGU_CfgPIO();
	// Enable Transmitter
    AT91F_US_EnableTx((AT91PS_USART)AT91C_BASE_DBGU);
		AT91F_US_EnableRx((AT91PS_USART)AT91C_BASE_DBGU);
}


void AT91F_DBGU_Printk(	char *buffer) // \arg pointer to a string ending by \0
{
	while(*buffer != '\0') {
		while (!AT91F_US_TxReady((AT91PS_USART)AT91C_BASE_DBGU));
		AT91F_US_PutChar((AT91PS_USART)AT91C_BASE_DBGU, *buffer++);
	}
}


int main()
{
  unsigned int i;


  InitADC();

  val = GetAdcChanel(ADC_CHN_1);
  if((val<300)||(val>380)) {
    // error
    val = 0;
  }
  val = GetAdcChanel(ADC_CHN_2);
  if((val<640)||(val>720)) {
    // error
    val=0;
  }


  // Enable clock
  //enable the clock of the PIO
  p_pPMC->PMC_PCER = 1 << AT91C_ID_PIOA;
  //enable the clock of the PIO
  p_pPMC->PMC_PCER = 1 << AT91C_ID_PIOB;
  //enable the clock of the PIO
  p_pPMC->PMC_PCER = 1 << AT91C_ID_PIOC;
	p_pPMC->PMC_PCER = 1 << AT91C_ID_US1;
	

	AT91F_DBGU_Init();
	
	//while (TestBoard()==0);


  p_pPioA->PIO_PPUDR  = 0xffffffff;   // Disable Pull-up resistor
  p_pPioB->PIO_PPUDR  = 0xffffffff;   // Disable Pull-up resistor
  p_pPioC->PIO_PPUDR  = 0xffffffff;   // Disable Pull-up resistor

  //LED STAT
  p_pPioA->PIO_PER  |= BIT6;     //Enable PA6
  p_pPioA->PIO_OER  |= BIT6;     //Configure in Output
  p_pPioA->PIO_SODR |= BIT6;     //set reg to 1

  //LED PWR_LED :-))))
  p_pPioA->PIO_PER  |= BIT9;     //Enable PA9
  p_pPioA->PIO_OER  |= BIT9;     //Configure in Output
  p_pPioA->PIO_SODR |= BIT9;     //set reg to 1

  //BUTTON BUT
  p_pPioC->PIO_ODR  |= BIT15;   //Configure in Input
  p_pPioC->PIO_PER  |= BIT15;   //Enable PC15

  //BUTTON WAKE_UP
  //p_pPio->PIO_ODR   |= BIT20;   //Configure in Input
  //p_pPio->PIO_PER   |= BIT20;   //Enable PA20

  // RTC
  // Set prescaler
  p_pRTTC->RTTC_RTMR &= ~0x0000FFFF;	
  p_pRTTC->RTTC_RTMR |= 0x00008000;

  // Restart
  p_pRTTC->RTTC_RTMR |= AT91C_RTTC_RTTRST;	
	

	AT91F_UART1_Init();


  while(1) {

		if (AT91F_US_RxReady(AT91C_BASE_US1))
		{
			while (!AT91F_US_TxReady((AT91PS_USART)AT91C_BASE_US1));
			AT91F_US_PutChar((AT91PS_USART)AT91C_BASE_US1, '*');
			while (!AT91F_US_TxReady((AT91PS_USART)AT91C_BASE_US1));
			AT91F_US_PutChar((AT91PS_USART)AT91C_BASE_US1, AT91F_US_GetChar((AT91PS_USART)AT91C_BASE_US1)+1);
		}
		
    if(!(p_pPioC->PIO_PDSR&BIT15)) {
      p_pPioA->PIO_CODR |= BIT9;
    }
    else {
      p_pPioA->PIO_SODR |= BIT9;
    }

    //
    Tick = AT91C_BASE_RTTC->RTTC_RTSR;

    if((Tick)&0x0002)  {

      for(dly=0; dly<10000; dly++);
			//AT91F_US_PutChar((AT91PS_USART)AT91C_BASE_US1, '*');

      if((p_pPioA->PIO_PDSR)&(BIT6)) {
        p_pPioA->PIO_CODR = BIT6;
      }
      else {
        p_pPioA->PIO_SODR = BIT6;
      }
    }
  }
}
