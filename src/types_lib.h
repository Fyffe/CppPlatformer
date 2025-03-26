#pragma once

#include "debug_lib.h"

template<typename T, int N>
struct Array
{
    static constexpr int maxElements = N;
    int count = 0;
    T elements[N];

    T& operator[](int index)
    {
        _ASSERT(index >= 0, "Index can't be negative");
        _ASSERT(index < count, "Index can't be outside of array bounds");

        return elements[index];
    }

    int add(T element)
    {
        _ASSERT(count < maxElements, "Array is full");
        
        elements[count] = element;
        
        return count++;
    }

    void remove_swap(int index)
    {
        _ASSERT(index >= 0, "Index can't be negative");
        _ASSERT(index < count, "Index can't be outside of array bounds");

        elements[index] = elements[--count];
    }

    void clear()
    {
        count = 0;
    }

    bool is_full()
    {
        return count == N;
    }
};