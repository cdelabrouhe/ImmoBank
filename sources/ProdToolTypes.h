#pragma once

#include <assert.h>

#ifndef MAX_PATH
#define MAX_PATH          260
#endif

#define ASSERT assert

// Windows forward declaration and general horror
typedef void* HANDLE;
typedef unsigned long DWORD;
typedef struct _SYSTEMTIME SYSTEMTIME;
typedef struct _FILETIME FILETIME;
typedef struct _WIN32_FIND_DATAA WIN32_FIND_DATAA;

// Helpers
#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif