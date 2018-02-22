#pragma once

#include <assert.h>

#define ASSERT assert

#ifdef WIN32
typedef unsigned int			ux;
typedef int						ix;
#else
typedef unsigned long long int	ux;
typedef long long int			ix;
#endif