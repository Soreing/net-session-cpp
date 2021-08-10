#ifndef CPMUTEX_H
#define CPMUTEX_H

#include "cplatforms.h"
#if defined PLATFORM_WINDOWS
#include <windows.h>
#elif defined PLATFORM_UNIX
#include <pthread.h> 
#endif

//A Windows-Unix cross platform encapsulation of mutex locking.
//The lock is an event on Windows, while a mutex variable on Unix.
//The lock can be locked/unlocked with interfacing functions.
class mutex
{
#if defined PLATFORM_WINDOWS
private:
	HANDLE lck;

public:
	inline mutex() : lck(CreateEvent(NULL, false, true, false)) {}
	inline void lock() { WaitForSingleObject(lck, INFINITE); }
	inline void unlock() { SetEvent(lck); }

#elif defined PLATFORM_UNIX
private:
	pthread_mutex_t lck;

public:
	inline mutex() {pthread_mutex_init(&lck, NULL); }
	inline ~mutex() {pthread_mutex_destroy(&lck); }
	inline void lock() { pthread_mutex_lock(&lck); }
	inline void unlock() { pthread_mutex_unlock(&lck); }
#endif
};
#endif