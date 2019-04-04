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



// PIT interrupt service routine
volatile unsigned int ms_count = 0;
void PIT_ISR()
{
	// Clear PITS
	AT91F_PITGetPIVR(AT91C_BASE_PITC);

	// increase ms_count
	ms_count++;
}



// Initialize PIT and interrupt
void PIT_initiailize()
{
	// enable peripheral clock for PIT
	AT91F_PITC_CfgPMC();

	// set the period to be every 1 msec in 48MHz
	AT91F_PITInit(AT91C_BASE_PITC, 1, 48);

	// PIV (Periodic Interval Value) = 3000 clocks = 1 msec
	// MCK/16 = 48,000,000 / 16 = 3,000,000 clocks/sec
	AT91F_PITSetPIV(AT91C_BASE_PITC, 3000-1);

	// disable PIT periodic interrupt for now
	AT91F_PITDisableInt(AT91C_BASE_PITC);

	// interrupt handler initializatioin
	AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_SYS, 7, 1, PIT_ISR);

	// enable the PIT interrupt
	AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_SYS);
}



// delay in ms by PIT interrupt
void HW_delay_ms(unsigned int ms)
{
	// special case
	if(ms == 0) return;

	// start time
	ms_count = 0;

	// enable PIT interrupt
	AT91F_PITEnableInt(AT91C_BASE_PITC);

	// wait for ms
	while(ms_count < ms);

	// disable PIT interrupt
	AT91F_PITDisableInt(AT91C_BASE_PITC);
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
	int time, cnt = 0;
	int hour = 0, min = 0, sec = 0, msec = 0;
	

  	Port_Setup();
  	
  	DBG_Init();
  	PIT_initiailize();
  	
Uart_Printf("START\n\r");

	while(1) 
	{
	
	
	
		Uart_Printf("\r%02d : %02d : %02d : %02d", hour, min, sec, msec);
		
		HW_delay_ms(10);
		msec++;
		
		
		if(msec == 100)
		{
			sec += 1;
			msec = 0;
			rPIO_SODR_B=(LED1);
			rPIO_CODR_B=(LED1);

		}
		if(sec == 60)
		{
			min += 1;
			sec = 0;
			rPIO_SODR_B=(LED2);
			rPIO_CODR_B=(LED2);

		}
		if(min == 60)
		{
			hour += 1;
			min = 0;
			rPIO_SODR_B=(LED3);
			rPIO_CODR_B=(LED3);

		}
		if (hour == 24) hour = 0;
		
		
	}	
		

}


