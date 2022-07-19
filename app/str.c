#include "str.h"

char* str_prbrk (const char *source, const char *accept, bool nullOnNoMatch){
    const char *end = source + strlen(source);

    while(end >= source) {
        const char *a = accept;
        while(*a){
            if(*a++ == *end) {
                return (char *)end+1;
            }
        }
        end--;
    }

	if(nullOnNoMatch)
    	return NULL;

	return (char *)source;
}