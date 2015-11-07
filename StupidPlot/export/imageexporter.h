#pragma once

#include <memory>
#include <vector>
#include <cmath>

using std::vector;
using std::shared_ptr;

namespace StupidPlot
{
    namespace Export
    {
        class ImageExporter
        {
        public:
            static shared_ptr<vector<char>> getPixels(HBITMAP hBitmap, int & width, int & height)
            {
                auto data = shared_ptr<vector<char>>(new vector<char>());

                BITMAP bitmap;
                BITMAPINFO bmInfo = { 0 };

                HDC hDC = CreateCompatibleDC(NULL);
                HBITMAP oldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);
                GetObject(hBitmap, sizeof(bitmap), &bitmap);

                bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmInfo.bmiHeader.biWidth = width = bitmap.bmWidth;
                bmInfo.bmiHeader.biHeight = height = bitmap.bmHeight;
                bmInfo.bmiHeader.biPlanes = 1;
                bmInfo.bmiHeader.biBitCount = 24;
                bmInfo.bmiHeader.biCompression = BI_RGB;
                bmInfo.bmiHeader.biSizeImage = ((width * 24 + 31) / 32) * 4 * height;

                data->resize(bmInfo.bmiHeader.biSizeImage);
                GetDIBits(hDC, hBitmap, 0, height, &data.get()[0], &bmInfo, DIB_RGB_COLORS);
                SelectObject(hDC, oldBitmap);

                height = std::abs(height);
                DeleteDC(hDC);

                return data;
            }
        };
    }
}