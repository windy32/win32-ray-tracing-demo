#ifndef UTILS_H
#define UTILS_H

class Utils
{
public:
    // Added here to avoid include <windows.h>
    static int GetTickCount();

    // Debug output
    static void DbgPrint(char *format, ...);
    static void PrintTickCount(char *desc);

    // Memory
    static int GetMemorySize();
};

#endif
