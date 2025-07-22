/*
 * String utility functions implementation
 * Phase 2 Day 10: Essential string operations for shell implementation
 */

#include "string.h"

size_t strlen(const char* str)
{
    if (!str) return 0;
    
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int strcmp(const char* s1, const char* s2)
{
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;
    
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char* s1, const char* s2, size_t n)
{
    if (n == 0) return 0;
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;
    
    while (n-- && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    
    if (n == (size_t)-1) return 0;  // Reached end without difference
    return (unsigned char)*s1 - (unsigned char)*s2;
}

char* strcpy(char* dest, const char* src)
{
    if (!dest || !src) return dest;
    
    char* orig_dest = dest;
    while ((*dest++ = *src++));
    return orig_dest;
}

char* strncpy(char* dest, const char* src, size_t n)
{
    if (!dest || !src || n == 0) return dest;
    
    char* orig_dest = dest;
    while (n-- && (*dest++ = *src++));
    
    // Pad with null bytes if src is shorter than n
    while (n-- > 0) {
        *dest++ = '\0';
    }
    
    return orig_dest;
}