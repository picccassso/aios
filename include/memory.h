/*
 * ARM64 OS Memory Management
 * Phase 2: Simple Bump Allocator
 */

#ifndef MEMORY_H
#define MEMORY_H

// Define our own types since we're freestanding
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef unsigned long uintptr_t;
typedef unsigned long size_t;

// Define NULL
#define NULL ((void*)0)

// Memory alignment (16 bytes for ARM64)
#define MEMORY_ALIGNMENT 16

// Heap size (1MB initially)
#define HEAP_SIZE (1024 * 1024)

// Memory allocation functions
void memory_init(void);
void* malloc(size_t size);
void memory_info(void);

// Memory utility functions
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t size);
void* memmove(void* dest, const void* src, size_t size);
char* strdup(const char* str);

// Memory statistics
typedef struct {
    uintptr_t heap_start;
    uintptr_t heap_end;
    uintptr_t current_ptr;
    size_t total_allocated;
    size_t num_allocations;
    size_t bytes_remaining;
} memory_stats_t;

memory_stats_t* get_memory_stats(void);

// Testing functions
void test_memory_utilities(void);
void test_allocation_patterns(void);
void test_allocation_failure(void);
void test_string_operations(void);
void run_all_memory_tests(void);

#endif // MEMORY_H