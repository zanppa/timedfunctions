/**
timedFunctions.c

This library allows basic very lightweight multitasking by
converting wait statements to timer callbacks. This way, the 
main program can continue while one function is waiting for 
a certain time.

This file contains the harware-specific implementations of 
timer initialization function and interrupt routine.

This work was inspired by Adam Dunkels' Protothreads
http://dunkels.com/adam/pt/index.html


Copyright (C) 2017  Lauri Peltonen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
**/
#include <stdint.h>

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"

#include "driverlib/sysctl.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"

#include "driverlib/timer.h"

#include "timedFunctions.h"


// Globals for timed functions
volatile uint8_t waitMutex;		// Lock for the exact wait timer
volatile timedCallback waitCb;	// Callback for exact waitCb



// Timed functions interrupt handler
void __attribute__ ((interrupt)) timedFunctionsIntHandler(void)
{
	uint8_t i;
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	waitMutex = TIMER_LOCK;	// Timer stopped
	if(waitCb.callback)
		(*waitCb.callback)(waitCb[i].data, CALLER_TIMER);	// Handle callback
}


// Load and enable the timer
void timedLoadTimer(uint64_t value)
{
	TimerLoadSet64(TIMER0_BASE, time * CLOCKS_IN_US);
	TimerEnable(TIMER0_BASE, TIMER_A);
}

// Initialize timed functions
// Uses timer 0 which is setup to count microseconds
void initTimedFunctions(void)
{
	// General timer, meant for one-shot accurate delays
	if(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
	{
		SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
		while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
	}
	TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
	TimerLoadSet64(TIMER0_BASE, (uint64_t)SysCtlClockGet());	// Default
	TimerIntRegister(TIMER0_BASE, TIMER_A, timedFunctionsIntHandler);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntEnable(INT_TIMER0A);
}