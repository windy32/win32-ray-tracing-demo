#ifndef UTILS_H
#define UTILS_H

typedef void (*LogCallback)(const char *str); // same as the one defined in Scripts.h

class Utils
{
private:
    static LogCallback log;

public:
    // Added here to avoid include <windows.h>
    static int GetTickCount();

    // Added here to avoid include <windows.h>
    static void *AllocCriticalSection();
    static void Lock(void *cs);
    static void Unlock(void *cs);
    static void DeleteCriticalSection(void *cs);

    // Debug output
    static void RegisterOutputTarget(LogCallback target);
    static void SysDbgPrint(char *format, ...);
    static void DbgPrint(char *format, ...);
    static void PrintTickCount(char *desc);

    // Save bitmap (bgr 24bit bmp)
    static bool SaveBitmap(const char *filename, int width, int height, void *data);
};

#endif
