#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

// Declarations for the string / memory helpers implemented in string.cpp.
size_t strlen(const char* str);
int    strcmp(const char* a, const char* b);
void*  memcpy(void* dest, const void* src, size_t count);
void*  memset(void* dest, uint8_t value, size_t count);
int    atoi(const char* str);

#endif // STRING_H
