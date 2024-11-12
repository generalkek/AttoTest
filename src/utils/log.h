#pragma once

#include <stdio.h>

#ifdef WIN32
#define myLogFunc printf_s
#else
#define myLogFunc printf
#endif

#ifdef NDEBUG
	#define LOG_DEBUG(...)
#else 
	#define LOG_DEBUG(fmt, ...) myLogFunc ("%s " fmt "\n", "[DEBUG]" , ## __VA_ARGS__)
#endif

#define LOG_INFO(fmt, ...) myLogFunc ("[INFO] " fmt "\n", ## __VA_ARGS__)
#define LOG_ERROR(fmt, ...) myLogFunc ("[ERROR] " fmt "\n", ## __VA_ARGS__)
