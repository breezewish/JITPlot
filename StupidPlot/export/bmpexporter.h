#pragma once

#include <string>

#include <windows.h>

#include <export/imageexporter.h>

using std::wstring;

namespace StupidPlot
{
    namespace Export
    {
        class BMPExporter : ImageExporter
        {
        public:
            static bool save(HBITMAP hBitmap, wstring fileName)
            {
                int width, height;
                auto data = ImageExporter::getPixels(hBitmap, width, height);

                BITMAPFILEHEADER bfHeader = { 0 };
                BITMAPINFOHEADER biHeader = { 0 };

                biHeader.biSize = sizeof(BITMAPINFOHEADER);
                biHeader.biWidth = width;
                biHeader.biHeight = -height;
                biHeader.biPlanes = 1;
                biHeader.biBitCount = 24;
                biHeader.biCompression = BI_RGB;

                auto fh = CreateFileW(
                    fileName.c_str(),   // name
                    GENERIC_WRITE,      // access
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL
                    );

                if (fh == INVALID_HANDLE_VALUE) return false;

                auto dibSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + data->size();

                bfHeader.bfType = 0x4D42;
                bfHeader.bfSize = dibSize;
                bfHeader.bfReserved1 = 0;
                bfHeader.bfReserved2 = 0;
                bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

                DWORD written;

                WriteFile(fh, &bfHeader, sizeof(BITMAPFILEHEADER), &written, NULL);
                WriteFile(fh, &biHeader, dibSize, &written, NULL);
                CloseHandle(fh);

                return true;
            }
        };
    }
}