#include "memory.h"

void *memset(void *str, int c, size_t n)
{
	char *c_ptr = (char *)str;

	for (int i = 0; i < n; i++)
		c_ptr[i] = (char)c;

	return str;
}
