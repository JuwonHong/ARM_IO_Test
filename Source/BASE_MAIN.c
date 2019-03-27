#include "board.h"
#include "AT91SAM7x.h"
#include "myLIB.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <pio/pio.h>
#include <aic/aic.h>
#include <utility\trace.h>
#include <cs8900a/cs8900a.h>

//For Switch
#define LEFT	1
#define RIGHT	2

#define FIRST 28
#define LAST 30

#define F_NUM 5

unsigned int 	Key_Count=0,Pre_Key_Data=0;
unsigned char Switch_Check(void);
unsigned char Port_Flag=0;
unsigned int	Count=0; 
//============================================================================
//  Function  : PIT Interrupt
//============================================================================
void Isr_PIT(void)
{
    volatile unsigned int pit_pivr;
	if((rPIT_SR & 1) != 0)  //The Periodic Interval timer has reached PIV since the last read of PIT_PIVR
    {
		pit_pivr = rPIT_PIVR;    //Reads Periodic Interval Timer Value Register - Clears PITS in PIT_SR
//		Count++;
//		if(Count==100)
//		{
//			Count=0;
			if(Port_Flag==0)
			{
				rPIO_SODR_B=(LED1|LED2|LED3);
				Port_Flag=1;
			}
			else
			{
				
				rPIO_CODR_B=(LED1|LED2|LED3);
				Port_Flag=0;
			}	
//		}		
	}
}
void 	PIT_Interrupt_Setup(void) 
{
	unsigned int	tmp=0;

    rAIC_IECR = (1<<1);

	// System Advanced Interrupt Controller
    rAIC_SMR1 = (1<<5) +  (7<<0);  //Edge Trigger, Prior 7
    rAIC_SVR1 = (unsigned)Isr_PIT;

	// System Periodic Interval Timer (PIT) Mode Register (MR)

	// PITEN(24) - Periodic Interval Timer Enabled
	// unsigned int PITEN = (1<<24);

	// PITIEN(25) - Periodic Interval Timer Interrupt Enabled
	// unsigned int PITIEN = (1<<25);

	// PIV(19:0) - Periodic Interval Time
	// will be compared with 20-bit CPIV (Counter of Periodic Interval Timer)
    tmp=(48000000/16/100)&0xFFFFF;         // T=30Hz
	// unsigned int PIV = 0;
	// PIV = (48000000/16/100)&0xFFFFF;

    rPIT_MR=(1<<25)+(1<<24)+(tmp<<0);      // Enable PIT, Disable Interrupt
	// rPIT_MR = PITIEN + PITEN + PIV;
}


void Port_Setup(void)
{
	// PMC (Power Management Clock) enables peripheral clocks
	AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_PIOB );
	AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_PIOA );
	
	// Enable PIO in output mode: Port A 0-7
	AT91F_PIO_CfgOutput( AT91C_BASE_PIOA,  PORTA);

	// LED (Port B: 28-30)
	AT91F_PIO_CfgOutput( AT91C_BASE_PIOB, LED1|LED2|LED3 ); // output mode
	AT91F_PIO_CfgPullup( AT91C_BASE_PIOB, LED1|LED2|LED3 ); // pull-up

	// Switch (Port A: 8,9)
	AT91F_PIO_CfgInput( AT91C_BASE_PIOA, SW1|SW2 ); // output mode
	AT91F_PIO_CfgPullup( AT91C_BASE_PIOA, SW1|SW2 ); // pull-up

//AT91F_PIO_SetOutput(AT91C_BASE_PIOA, (1<<13));
//AT91F_PIO_ClearOutput(AT91C_BASE_PIOA, (1<<13));
	
}

void LED_F_L(int a, int b, int c)
{
	int d = 0, e = 0;
    
	for(e=a; e<=b; d = d + c)
		{
				
		
		rPIO_SODR_B=(1<<e);
		for(e = 0; e < 10; ++e) Delay(100000);

		rPIO_CODR_B=(LED1|LED2|LED3);
		for(e = 0; e < 10; ++e) Delay(100000);
		
		if(c>0 && d >0) break;
		
		}			

}


int Factorial(int Factorial_Num)
{
int i;
int Factorial_Result = 1;


for(i = 1; i<= Factorial_Num; i++)
{
	Factorial_Result *= i;
	


}



return Factorial_Result;


}

/*
void Read_For_Setup_CMOS(void)
{

	//Read address Reset
	rPIO_CODR_A=FIFO_RD_RST;			
	rPIO_SODR_A=FIFO_RD;
	rPIO_CODR_A=FIFO_RD;
	rPIO_SODR_A=FIFO_RD;
	rPIO_CODR_A=FIFO_RD;
	rPIO_SODR_A=FIFO_RD_RST;

	//Write On		
	//rPIO_SODR_A=XCLK_ON;
	rPIO_SODR_A=HREF_SYNC;
			
	//Until Wait Low
	while((rPIO_PDSR_A & 0x00000004)){}
	//Until Wait High
	while(!(rPIO_PDSR_A & 0x00000004)){}
			
	//Write Off		
	//rPIO_CODR_A=HREF_SYNC;
			
	//CS_LOW
	rPIO_CODR_A=FIFO_CS;

}

void CMOS_Read_Clk(void)
{
	rPIO_SODR_A=FIFO_RD;
	rPIO_CODR_A=FIFO_RD;
}
*/

unsigned char Switch_Check(void)
{
unsigned char Result=0;

	if(!(rPIO_PDSR_A & SW1)) Result=LEFT;
	else if(!(rPIO_PDSR_A & SW2)) Result=RIGHT;
	
	
	if(Pre_Key_Data==Result) Key_Count++;
	else Key_Count=0;
	
	Pre_Key_Data=Result;
	return	Result;
}
//-----------------------------------------------------------------------------
/// Main Procedure
//-----------------------------------------------------------------------------
                   
int main()
{
	int i = 0;
	//int count = 0;
	//int Z;
	//int LEDNUM[3] = {LED1, LED2, LED3};
	//count = LAST - FIRST;
	//int x = 0,y = 0;
	//int LED_CON[count] = {};
	int n = 0;

	
	
	
  	Port_Setup();
  	DBG_Init();
  	
  	Uart_Printf("Hello World\n\r");
  	
  	
	
	while(1) 
	{
	
		Uart_Printf("%d! = %d\n\r", n, Factorial(n) );
		
		for(i = 0; i < 10; ++i) 
		{
		Delay(100000);
		
		}
		
	n++;
	
	/*
		for(n=0; n<=F_NUM; n++)
		{
		
		Uart_Printf("%d! = %d\n\r", n, Factorial(n) );
		
		for(i = 0; i < 10; ++i) 
		{
		Delay(100000);
		}
		
		}
		
		*/
		
		
	
		//Uart_Printf("n = %d\n\r", n);
	
	
		//n++;
	}	
		
		// LED off
		
		//rPIO_CODR_B=(LED1|LED2|LED3);
		
		
	

		// LED on
		//rPIO_SODR_B=(LED1|LED2|LED3);
		/*
		if (n%2 == 0)
		{
	
		LED_F_L(FIRST, LAST, 1);
			
		}
		else
		{
	
	
		LED_F_L(LAST, FIRST, -1);
				
		}
		n++;
		
		
		
		*/
		
		
		
		
		
		
			/*Z=LAST;
			
		
			for(i=FIRST; i<=2*LAST; i++)
			{
			
			rPIO_SODR_B=(1<<i);
			for(i = 0; i < 10; ++i) Delay(100000);

			rPIO_CODR_B=(LED1|LED2|LED3);
			for(i = 0; i < 10; ++i) Delay(100000);
			
				if(LAST<i<=2*LAST)
				{					
					Z-=1;
					rPIO_SODR_B=(1<<Z);
					for(i = 0; i < 10; ++i) Delay(100000);

					rPIO_CODR_B=(LED1|LED2|LED3);
					for(i = 0; i < 10; ++i) Delay(100000);
				}
				
			}
			Z=LAST;
			*/
			/*	

			for(i=LAST; i<=FIRST; i--)
			{
			
			rPIO_SODR_B=(1<<i);
			for(i = 0; i < 10; ++i) Delay(100000);

			rPIO_CODR_B=(LED1|LED2|LED3);
			for(i = 0; i < 10; ++i) Delay(100000);

			}
			
			*/
			/*
		rPIO_SODR_B=(LED1);
		for(i = 0; i < 10; ++i) Delay(100000);
		rPIO_CODR_B=(LED1|LED2|LED3);
		for(i = 0; i < 10; ++i) Delay(100000);
		
		rPIO_SODR_B=(LED2);
		for(i = 0; i < 10; ++i) Delay(100000);
		rPIO_CODR_B=(LED1|LED2|LED3);
		for(i = 0; i < 10; ++i) Delay(100000);
		
		rPIO_SODR_B=(LED3);
		for(i = 0; i < 10; ++i) Delay(100000);
		rPIO_CODR_B=(LED1|LED2|LED3)
		for(i = 0; i < 10; ++i) Delay(100000);
		
		*/
		
		
	//}	
}


