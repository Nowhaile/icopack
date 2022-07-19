#ifndef STR_H
#define STR_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "str.h"

char* str_prbrk (const char *source, const char *accept, bool nullOnNoMatch);

#endif // STR_H
