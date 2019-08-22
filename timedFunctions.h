/**
timedFunctions.h

This library allows basic very lightweight multitasking by
converting wait statements to timer callbacks. This way, the 
main program can continue while one function is waiting for 
a certain time.

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

#ifndef __TIMEDFUNCTIONS_H__
#define __TIMEDFUNCTIONS_H__

// Timed function callback function prototype
typedef void (*timedCallbackFunction)(void *pdata, char caller);

// Timed function callback data structure
typedef struct _timedCallback {
  uint32_t timer;						// Timer value
  timedCallbackFunction callback;		// Callback function
  void *data;							// Data pointer to pass to callback, e.g. state
} timedCallback;


// Globals from timedFunctions.c
extern volatile uint8_t waitMutex;		// Lock for the wait timer
extern volatile timedCallback waitCb;	// Callback for the interrupt

// Timer state
#define TIMER_FREE			0
#define TIMER_LOCK			1
#define TIMER_RUN			2

// Who called the function (timer callback or other thread)
#define CALLER_TIMER		1
#define CALLER_THREAD		0

// Return status of timed function, still waiting for the timer to expire or done
#define RETURN_WAIT			0
#define RETURN_DONE			1

// Macro to help defining the timed function
#define TIMED_FUNCTION(name_args) char name_args(void *pdata, char caller)


// Declare start of a timed function
#define TIMED_BEGIN()											\
	static unsigned short _timed_pt = 0;						\
	static char _timed_mutex = 0;								\
	if(_timed_mutex && caller == CALLER_THREAD) return RETURN_WAIT;		\
	switch(_timed_pt) {											\
	case 0:


// Declare end of a timed function
#define TIMED_END()												\
	}															\
	_timed_pt = 0;												\
	_timed_mutex = 0;											\
	return RETURN_DONE;

	
// Wait (and block) until the timer is available
#define TIMED_LOCK()											\
	do {														\
		_timed_pt = __LINE__; case __LINE__:					\
		if(waitMutex != TIMER_FREE) return RETURN_WAIT;			\
		waitMutex = TIMER_LOCK;									\
	} while(0)


// Schedule continuation with the timer
// Time is given in us in the default implementation
#define TIMED_WAIT(func, time, data)							\
	do {														\
		waitCb.callback = &func;								\
		waitCb.data = data;										\
		waitMutex = TIMER_RUN;									\
		_timed_mutex = 1;										\
		timedLoadTimer(time * CLOCKS_IN_US);					\
		_timed_pt = __LINE__; case __LINE__:					\
		if(waitMutex == TIMER_RUN) return RETURN_WAIT;			\
		_timed_mutex = 0;										\
	} while(0)


// Yield from timer (interrupt callback) and continue next time the thread is run
// (without releasing the timer mutex)
#define TIMED_YIELD()											\
	do {														\
		_timed_pt = __LINE__; case __LINE__:					\
		if(caller == CALLER_TIMER) return RETURN_WAIT;			\
	} while(0)


// Release the timer
#define TIMED_RELEASE()											\
	do {														\
		waitCb.callback = 0;									\
		waitMutex = TIMER_FREE;									\
		_timed_pt = __LINE__; case __LINE__:					\
		if(caller == CALLER_TIMER) return RETURN_WAIT;			\
	} while(0)



// Initialize the timer used for exact wait macros
void initTimedFunctions(void);

// Load and enable the timer
void timedLoadTimer(uint64_t value)

#endif
