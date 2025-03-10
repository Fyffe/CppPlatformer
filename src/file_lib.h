#pragma once

#include <sys/stat.h>
#include <string.h>

#include "debug_lib.h"
#include "alloc_lib.h"

long long get_timestamp(char* file)
{
    struct stat file_stat = {};
    stat(file, &file_stat);
    return file_stat.st_mtime;
}

bool file_exists(char* filePath)
{
    _ASSERT(filePath, "No file path specified!");

    auto file = fopen(filePath, "rb");

    if(!file)
    {
        return false;
    }

    fclose(file);

    return true;
}

long get_file_size(char* filePath)
{
    _ASSERT(filePath, "No file path specified!");

    long fileSize = 0;
    auto file = fopen(filePath, "rb");

    if(!file)
    {
        _ERROR("Failed opening file: %s", filePath);

        return 0;
    }

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);

    return fileSize;
}

char* read_file(char* filePath, int* fileSize, char* buffer)
{
    _ASSERT(filePath, "No file path specified!");
    _ASSERT(fileSize, "No file size specified!");

    *fileSize = 0;

    auto file = fopen(filePath, "rb");

    if(!file)
    {
        _ERROR("Failed opening file: %s", filePath);
        
        return nullptr;
    }

    fseek(file, 0, SEEK_END);

    *fileSize = ftell(file);

    fseek(file, 0, SEEK_SET);
    memset(buffer, 0, *fileSize + 1);
    fread(buffer, sizeof(char), *fileSize, file);
    fclose(file);

    return buffer;
}

char* read_file(char* filePath, int* fileSize, BumpAllocator* bumpAllocator)
{
    char* file = nullptr;
    long lFileSize = get_file_size(filePath);

    if(lFileSize)
    {
        char* buffer = bump_alloc(bumpAllocator, lFileSize + 1);

        file = read_file(filePath, fileSize, buffer);
    }

    return file;
}

void write_file(char* filePath, char* buffer, int size)
{
    _ASSERT(filePath, "No file path specified!");
    _ASSERT(buffer, "No buffer specified!");

    auto file = fopen(filePath, "rb");

    if(!file)
    {
        _ERROR("Failed opening file: %s", filePath);

        return;
    }

    fwrite(buffer, sizeof(char), size, file);
    fclose(file);
}

bool copy_file(char* filePath, char* outputPath, char* buffer)
{
    int fileSize = 0;
    char* data = read_file(filePath, &fileSize, buffer);
    auto outFile = fopen(outputPath, "wb");

    if(!outFile)
    {
        _ERROR("Failed opening file: %s", outputPath);

        return false;
    }

    int result = fwrite(data, sizeof(char), fileSize, outFile);

    if(!result)
    {
        _ERROR("Failed opening file: %s", outputPath);
    
        return false;
    }

    fclose(outFile);

    return true;
}

bool copy_file(char* filePath, char* outputPath, BumpAllocator* bumpAllocator)
{
    char* file = 0;
    long lFileSize = get_file_size(filePath);

    if(lFileSize)
    {
        char* buffer = bump_alloc(bumpAllocator, lFileSize + 1);

        return copy_file(filePath, outputPath, buffer);
    }

    return false;
}