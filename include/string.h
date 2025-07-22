/*
 * String utility functions for ARM64 OS
 * Phase 2 Day 10: Essential string operations for shell implementation
 */

#ifndef STRING_H
#define STRING_H

// Define size_t for freestanding environment
typedef unsigned long size_t;

// String length calculation
size_t strlen(const char* str);

// String comparison - returns 0 if equal, <0 if s1 < s2, >0 if s1 > s2
int strcmp(const char* s1, const char* s2);

// String comparison with maximum length
int strncmp(const char* s1, const char* s2, size_t n);

// String copy - destination must have enough space
char* strcpy(char* dest, const char* src);

// String copy with maximum length
char* strncpy(char* dest, const char* src, size_t n);

#endif // STRING_H