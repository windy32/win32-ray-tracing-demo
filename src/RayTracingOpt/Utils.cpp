#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include "Utils.h"

LogCallback Utils::log = NULL;

int Utils::GetTickCount()
{
    return (int)::GetTickCount();
}

void *Utils::AllocCriticalSection()
{
    CRITICAL_SECTION *cs = new CRITICAL_SECTION();
    InitializeCriticalSection(cs);
    return cs;
}

void Utils::Lock(void *cs)
{
    EnterCriticalSection((CRITICAL_SECTION *)cs);
}

void Utils::Unlock(void *cs)
{
    LeaveCriticalSection((CRITICAL_SECTION *)cs);
}

void Utils::DeleteCriticalSection(void *cs)
{
    ::DeleteCriticalSection((CRITICAL_SECTION *)cs);
    delete (CRITICAL_SECTION *)cs;
}

void Utils::RegisterOutputTarget(LogCallback target)
{
    log = target;
}

void Utils::SysDbgPrint(char *format, ...)
{
    char buf[1024];
    va_list argList;
    va_start(argList, format);

    vsprintf_s(buf, sizeof(buf), format, argList);
    OutputDebugStringA(buf);

    va_end(argList);
}

void Utils::DbgPrint(char *format, ...)
{
    char buf[1024];
    va_list argList;
    va_start(argList, format);

    vsprintf_s(buf, sizeof(buf), format, argList);
    if (log != NULL)
    {
        log(buf);
    }
    else
    {
        OutputDebugStringA(buf);
    }
    va_end(argList);
}

void Utils::PrintTickCount(char *desc)
{
    DWORD tickCount = GetTickCount();
    DbgPrint("%s: %.2lf\r\n", desc, tickCount / 1000.0);
}

bool Utils::SaveBitmap(const char *filename, int width, int height, void *data)
{
    int size = width * height * 3;

    // Create bitmap file header
    BITMAPFILEHEADER fileHeader;

    fileHeader.bfType = 0x4D42;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + size;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Create bitmap info header
    BITMAPINFOHEADER bitmapHeader = {0};

    bitmapHeader.biSize = sizeof(BITMAPINFOHEADER); 
    bitmapHeader.biHeight = height;
    bitmapHeader.biWidth = width;
    bitmapHeader.biPlanes = 3;
    bitmapHeader.biBitCount = 24;
    bitmapHeader.biSizeImage = size;
    bitmapHeader.biCompression = 0; //BI_RGB

    // Write to file
    FILE *output = NULL;
    fopen_s(&output, filename, "wb");

    if(output == NULL)
    {
        MessageBox(0, "Cannot create file!", "Error", MB_OK | MB_ICONWARNING);
        return false;
    }

    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, output);
    fwrite(&bitmapHeader, sizeof(BITMAPINFOHEADER), 1, output);
    fwrite(data, size, 1, output);
    fclose(output);

    MessageBox(0, "File created successfully!", "Informaiton", MB_OK | MB_ICONINFORMATION);
    return true;
}
