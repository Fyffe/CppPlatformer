#pragma once

#include <stdlib.h>
#include <string.h>

#include "debug_lib.h"

#define BIT(x) 1 << (x)
#define KB(x) ((unsigned long long)1024 * x)
#define MB(x) ((unsigned long long)1024 * KB(x))
#define GB(x) ((unsigned long long)1024 * MB(x))

struct BumpAllocator
{
    size_t capacity;
    size_t used;
    char* memory;
};

BumpAllocator make_bump_allocator(size_t size)
{
    BumpAllocator ba = {};
    ba.memory = (char*)malloc(size);
    
    _ASSERT(ba.memory, "Failed to allocate memory");

    if(ba.memory)
    {
        ba.capacity = size;
        memset(ba.memory, 0, size);
    }

    return ba;
}

char* bump_alloc(BumpAllocator* bumpAllocator, size_t size)
{
    char* result = nullptr;

    size_t alignedSize = (size + 7) &~ 7;
    bool isAlignedWithinCapacity = bumpAllocator->used + alignedSize <= bumpAllocator->capacity;
    
    _ASSERT(isAlignedWithinCapacity, "BumpAllocator is full");
    
    if(isAlignedWithinCapacity)
    {
        result = bumpAllocator->memory + bumpAllocator->used;
        bumpAllocator->used += alignedSize;
    }

    return result;
}