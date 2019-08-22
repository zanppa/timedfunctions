# Timed Functions
This library allows basic very lightweight multitasking by
converting wait statements to timer callbacks. This way, the 
main program can continue while one function is waiting for 
a certain time.

## Description
This software is very similar to [Protothreads](http://dunkels.com/adam/pt/) by Adam Dunkels which was my main inspiration
for writing this. You should first get familiar 
with that because I wont go into as much detail as he does.

`timedFunctions.c` contains functions that need to be re-implemented for different platforms. The default is 
for Texas Instrument's Tiva (TM4C123GH6PMI), and has been tested with [Energia IDE](https://energia.nu/). Probably 
works just fine with Code Composer Studio also.

## API
`void InitTimedFunctions(void)`  
Initialize hardware timers and set up data structures. This is implementation specific function

`void timedLoadTimer(uint64_t value)`  
Load the time value to the timer and enable it. This is implementation specific function

`void __attribute__ ((interrupt)) timedFunctionsIntHandler(void)`  
Timer interrupt that calls the timed functions. This is implementation specific function

`TIMED_FUNCTION(name_args)`  
Create a timed function with name `name_args`

`TIMED_BEGIN()`  
Declare start of a timed function

´TIMED_END()`  
Declare end of a timed function

`TIMED_LOCK()`  
Wait (and block) until the hardware timer is available

`TIMED_RELEASE()`  
Release the timer for other threads/functions to use

`TIMED_WAIT(func, time, data)`  
Wait a specified amount of time (default in us) and then call func (should be the same function from where this is called). Data is a ´void *` pointer
to any custom data that needs to be stored during the wait. Will be available in `pdata` after the wait

`TIMED_YIELD()`  
Yield from the function after the wait time has passed, and continue next time the function is called. Should be used if the function should return
back to the main program before the next ´TIMED_WAIT` or `TIMED_RELEASE`


## Caveats
As with Protothreads, the stack is not preserved between calls (i.e. after lock, wait, yield, ...) and as such any local 
variables are not preserved. Do not use local variables but instead use global variables or pass the data in the wait statement.


## Example
This is an example of a timed reset function that sends a long low pulse to reset some equipment.
```
TIMED_FUNCTION(resetCmd)
{
	TIMED_BEGIN();

	TIMED_LOCK();
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0);	// Pin 0 low to indicate reset
	TIMED_WAIT(resetCmd, 500000);	 	// Wait 500 ms

	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 1);	// Pin 0 high
	TIMED_RELEASE();

	TIMED_END();
}
```

This could be called from this kind of main function:
```
void main(void)
{
	int state = 0;
	InitTimedFunctions();
	
	while(1)
	{
		if(state == 0)
		{
			if(resetCmd(0, CALLER_THREAD) == RETURN_DONE)
			{
				state == 1;
			}
		}
		
		// Do other things and processes here
		// ....
	}
}
```

Even better usage is achieved if timed functions are combined with Protothreads, for example:
```
PT_THREAD(deviceLoop(struct pt *pt))
{
	PT_BEGIN(pt);
	
	// Reset
	PT_WAIT_UNTIL(pt, resetCmd(0, CALLER_THREAD) == RETURN_DONE);
	
	// Do other things here
	while(1)
	{
		// ...
		PT_YIELD(pt);
	}
	
	PT_END(pt);
}
```

## License
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