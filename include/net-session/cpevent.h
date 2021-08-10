#ifndef CPEVENT_H
#define CPEVENT_H

#include "cplatforms.h"
#if defined PLATFORM_WINDOWS
#include <windows.h>
#elif defined PLATFORM_UNIX
#include <pthread.h> 
#endif

//Signals are Windows-Unix cross platform encapsulation of events
//The events use Windows events or pthread_conditions on Unix
//Events can be waited for with or without time limit, and events can be set to alarm treads waiting
class signal
{
#if defined PLATFORM_WINDOWS
private:
	HANDLE evnt;
	
public:
	inline signal() : evnt(CreateEvent(NULL, false, false, NULL)) { }
	inline void wait(int waitTime = INFINITE) { WaitForSingleObject(evnt, waitTime); }
	inline void set() { SetEvent(evnt); }

#elif defined PLATFORM_UNIX
private:
	pthread_cond_t cnd;
	pthread_mutex_t mtx;
	int timestr;

public:
	inline signal() {	pthread_mutex_init(&mtx, NULL); pthread_cond_init(&cnd, NULL); }
	inline ~signal() { pthread_mutex_destroy(&mtx); pthread_cond_destroy(&cnd); }
	inline void set() { pthread_cond_signal(&cnd); }
	inline void wait(int waitTime =-1) 
	{
		struct timespec t = { 0,0 }; 
		clock_gettime(CLOCK_REALTIME, &t); 
		t.tv_sec += waitTime; 
		
		waitTime != -1 ? pthread_cond_timedwait(&cnd, &mtx, &t) : pthread_cond_wait(&cnd, &mtx); 
	}
#endif
};

#endif