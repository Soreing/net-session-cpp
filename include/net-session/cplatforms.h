#ifndef PLATFORMS_H
#define PLATFORMS_H

#if defined _WIN32 || defined _WIN64
#define PLATFORM_WINDOWS
#elif defined unix || defined __unix || defined __unix__ || defined __linux__ || defined __APPLE__ || defined __MACH__ 
#define PLATFORM_UNIX
#endif

#endif