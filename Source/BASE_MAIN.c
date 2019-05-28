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

#define on 1
#define off 0

#define F_NUM 5

unsigned int 	Key_Count=0,Pre_Key_Data=0;
unsigned char Switch_Check(void);
unsigned char Port_Flag=0;
unsigned int	Count=0; 

int hour11 = 0;
int min11 = 0;
int sec11 = 0;
int msec11 = 0;
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
	
	//Trig set output mode
	AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, PA0 );
	
	//Echo set input mode
	AT91F_PIO_CfgInput( AT91C_BASE_PIOA, PA1 );

	// Switch (Port A: 8,9)
	AT91F_PIO_CfgInput( AT91C_BASE_PIOA, SW1|SW2 ); // output mode
	AT91F_PIO_CfgPullup( AT91C_BASE_PIOA, SW1|SW2 ); // pull-up

//AT91F_PIO_SetOutput(AT91C_BASE_PIOA, (1<<13));
//AT91F_PIO_ClearOutput(AT91C_BASE_PIOA, (1<<13));
	
}


/*
// PIT interrupt service routine
volatile unsigned int ms_count = 0;
void PIT_ISR()
{
	// Clear PITS
	AT91F_PITGetPIVR(AT91C_BASE_PITC);

	// increase ms_count
	ms_count++;
}
*/

volatile unsigned int counts_per_10us = 0;
void PIT_ISR()
{
	// Clear PITS
	AT91F_PITGetPIVR(AT91C_BASE_PITC);

	// increase counts_per_10us
	counts_per_10us++;
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
	AT91F_PITSetPIV(AT91C_BASE_PITC, 30-1);

	// disable PIT periodic interrupt for now
	AT91F_PITDisableInt(AT91C_BASE_PITC);

	// interrupt handler initializatioin
	AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_SYS, 7, 1, PIT_ISR);

	// enable the PIT interrupt
	AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_SYS);
}
/*
void PIT_initiailize_us()
{
	// enable peripheral clock for PIT
	AT91F_PITC_CfgPMC();

	// set the period to be every 1 msec in 48MHz
	AT91F_PITInit(AT91C_BASE_PITC, 1, 48);

	// PIV (Periodic Interval Value)
	// MCK/16 = 48,000,000 / 16 = 3,000,000 clocks/sec
	// = 3,000,000 clocks / 1,000 ms = 3000 clocks / ms
	// = 3,000,000 clocks / 1,000,000 us = 30 clocks / 10us
	AT91F_PITSetPIV(AT91C_BASE_PITC, 30 - 1);

	// disable PIT periodic interrupt for now
	AT91F_PITDisableInt(AT91C_BASE_PITC);

	// interrupt handler initializatioin
	AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_SYS, 7, 1, PIT_ISR);

	// enable the PIT interrupt
	AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_SYS);
}

*/
/*
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
*/

// delay in ms by PIT interrupt
void HW_delay_10us(unsigned int ten_us)
{
	// special case
	if (ten_us == 0) return;

	// start time
	counts_per_10us = 0;

	// enable PIT interrupt
	AT91F_PITEnableInt(AT91C_BASE_PITC);

	// wait for ms
	while (counts_per_10us < ten_us);

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

//pio interrupt service routine
void PIO_ISR()
{
 hour11 = 0;
 min11 = 0;
 sec11 = 0;
 msec11 = 0;
}

void Interrupt_setup()
{
//use sw1 as an input
	AT91F_PIO_InputFilterEnable(AT91C_BASE_PIOA, SW1);
	
	//Set interrupt sw1
	AT91F_PIO_InterruptEnable(AT91C_BASE_PIOA, SW1);
	
	//Set callback function
	AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_PIOA, 7, 1, PIO_ISR);
	
	//Enable AIC
	//AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_PIOA);
	
	



}

void TC_initialize()
{
  // Enable peripheral clock in PMC for Timer Counter 0
  AT91F_TC0_CfgPMC();

  // Configure Timer Counter 0 with Software Trigger Effect on TIOA
  // MCK/2 = 48,000,000 / 2 Hz = 24,000,000 clocks / sec
  // = 24,000,000 clocks / 1,000,000 us
  // = 24 clocks / 1 us
  TC_Configure(AT91C_BASE_TC0, AT91C_TC_CLKS_TIMER_DIV3_CLOCK |
                               AT91C_TC_ASWTRG); 
}

//-----------------------------------------------------------------------------
/// Main Procedure
//-----------------------------------------------------------------------------
                   
int main()
{
	
	
int n =0;

  // Port set up
Port_Setup();
   
// UART 
  DBG_Init();
  Uart_Printf("Ultrasound - Test\n\r");

  // PIT setup
  PIT_initiailize();
  //PIT_initiailize_us();

  // Timer counter
  TC_initialize();

  while(1) 
  {
    Uart_Printf("iter = %d\n\r", n);
    rPIO_SODR_B=(LED1|LED2|LED3);

	// [1] Set Trigger pin (PA0) on
	rPIO_SODR_A = PA0;
	
	// [2] Wait for 10us
    HW_delay_10us(1); // 10us
    
	// [3] Set Trigger pin off
	rPIO_CODR_A = PA0;
	
	// [4] Listen to Echo pin (PA1) if it's on
	while(true)
	{
		if(AT91F_PIO_IsInputSet(AT91C_BASE_PIOA, PA1))
			break;
	}

	// [5] Start the timer
    // 타이머시작
    TC_Start(AT91C_BASE_TC0);
    
	// [6] Listen to Echo pin if it's off
	while(true)
	{
		if(!AT91F_PIO_IsInputSet(AT91C_BASE_PIOA, PA1))
			break;
	}
	
	// [7] Stop the timer
    // 타이머스탑
    TC_Stop(AT91C_BASE_TC0);
    
	// [8] TC_CV [count] -> distance [cm]
    // 타이머값
    // 1us = 58cm?
    
    //Uart_Printf("\tStop TC_CV = %d /*clocks*/ cm = %lf us\n\r", AT91C_BASE_TC0->TC_CV, (double)(AT91C_BASE_TC0-> TC_CV) / 1.5 * 58.0);
	
	Uart_Printf("\t\rUltraSound = %lf [cm]", (double)(AT91C_BASE_TC0-> TC_CV) / (1.5 * 58.0));
	
    rPIO_CODR_B=(LED1|LED2|LED3);
    HW_delay_10us(10000);
    n++;
  } 
  	
  	
		

}




//rPIO_CODR_B=(LED3);
//rPIO_SODR_B=(LED3);


/*  
Port_Setup();
  	
  	//Interrupt setup
  	Interrupt_setup();	
  	
  	DBG_Init();
  	PIT_initiailize();
  	
  	
  	
  	
	Uart_Printf("START\n\r");

	while(1) 
	{
	
	
		if(msec11 == 100)
		{
			sec11 += 1;
			msec11 = 0;
		}
		
		if(sec11 == 60)
		{
			min11 += 1;
			sec11 = 0;
		}
		
		if(min11 == 60)
		{
			hour11 += 1;
			min11 = 0;
		}
		
		if(hour11 == 24)
		{
			hour11 = 0;
		}
	
		Uart_Printf("\r%02d : %02d : %02d : %02d", hour11, min11, sec11, msec11);
		
		rPIO_SODR_B=(LED3);
		HW_delay_ms(5);
		rPIO_CODR_B=(LED3);
		HW_delay_ms(5);
		
		msec11++;		

	}	
*/