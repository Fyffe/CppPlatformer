#pragma once

#ifdef _WIN32
#define EXPORT_FN __declspec(dllexport)
#elif __linux__
#define EXPORT_FN
#elif __APPLE__
#define EXPORT_FN
#endif