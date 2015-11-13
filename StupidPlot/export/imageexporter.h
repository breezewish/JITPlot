#pragma once

#include <string>

#include <memory>
#include <vector>
#include <cmath>

#include <windows.h>
#include <wincodec.h>

using std::vector;
using std::shared_ptr;

namespace StupidPlot
{
    namespace Export
    {
        enum class ImageFormat
        {
            BMP,
            JPEG,
            PNG
        };

        class ImageExporter
        {
        protected:
            static shared_ptr<vector<unsigned char>> getPixels(HBITMAP hBitmap, int & width, int & height)
            {
                auto data = shared_ptr<vector<unsigned char>>(new vector<unsigned char>());

                BITMAP bitmap;
                BITMAPINFO bmInfo = { 0 };

                HDC hDC = CreateCompatibleDC(NULL);
                HBITMAP oldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);
                GetObject(hBitmap, sizeof(bitmap), &bitmap);

                width = bitmap.bmWidth;
                height = std::abs(bitmap.bmHeight);

                bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmInfo.bmiHeader.biWidth = width;
                bmInfo.bmiHeader.biHeight = -height;
                bmInfo.bmiHeader.biPlanes = 1;
                bmInfo.bmiHeader.biBitCount = 24;
                bmInfo.bmiHeader.biCompression = BI_RGB;
                bmInfo.bmiHeader.biSizeImage = ((width * 24 + 31) / 32) * 4 * height;

                data->resize(bmInfo.bmiHeader.biSizeImage);
                GetDIBits(hDC, hBitmap, 0, height, data->data(), &bmInfo, DIB_RGB_COLORS);
                SelectObject(hDC, oldBitmap);
                DeleteDC(hDC);

                return data;
            }
        public:
            static bool save(HBITMAP hBitmap, ImageFormat format, wstring fileName)
            {
                IWICImagingFactory *pFactory = NULL;
                IWICBitmapEncoder *pEncoder = NULL;
                IWICBitmapFrameEncode *pBitmapFrame = NULL;
                IWICStream *pStream = NULL;
                IPropertyBag2 *pPropertybag = NULL;

                HRESULT hr = CoCreateInstance(
                    CLSID_WICImagingFactory,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_IWICImagingFactory,
                    (LPVOID*)&pFactory
                    );

                if (SUCCEEDED(hr)) hr = pFactory->CreateStream(&pStream);
                if (SUCCEEDED(hr)) hr = pStream->InitializeFromFilename(fileName.c_str(), GENERIC_WRITE);
                if (SUCCEEDED(hr))
                {
                    GUID encoder = GUID_ContainerFormatPng;

                    if (format == ImageFormat::BMP)
                    {
                        encoder = GUID_ContainerFormatBmp;
                    }
                    else if (format == ImageFormat::JPEG)
                    {
                        encoder = GUID_ContainerFormatJpeg;
                    }

                    hr = pFactory->CreateEncoder(encoder, NULL, &pEncoder);
                }
                if (SUCCEEDED(hr)) hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
                if (SUCCEEDED(hr)) hr = pEncoder->CreateNewFrame(&pBitmapFrame, &pPropertybag);

                int width, height;
                auto data = ImageExporter::getPixels(hBitmap, width, height);
                UINT cbStride = (width * 24 + 7) / 8;
                UINT cbBufferSize = height * cbStride;
                WICPixelFormatGUID formatGUID = GUID_WICPixelFormat24bppBGR;

                if (SUCCEEDED(hr))
                {
                    if (format == ImageFormat::JPEG)
                    {
                        PROPBAG2 option = { 0 };
                        option.pstrName = L"ImageQuality";
                        VARIANT varValue;
                        VariantInit(&varValue);
                        varValue.vt = VT_R4;
                        varValue.fltVal = 0.95f;
                        hr = pPropertybag->Write(1, &option, &varValue);
                    }
                }

                if (SUCCEEDED(hr)) hr = pBitmapFrame->Initialize(pPropertybag);
                if (SUCCEEDED(hr)) hr = pBitmapFrame->SetSize(width, height);
                if (SUCCEEDED(hr)) hr = pBitmapFrame->SetPixelFormat(&formatGUID);
                if (SUCCEEDED(hr)) hr = IsEqualGUID(formatGUID, GUID_WICPixelFormat24bppBGR) ? S_OK : E_FAIL;
                if (SUCCEEDED(hr)) hr = pBitmapFrame->WritePixels(height, cbStride, cbBufferSize, data->data());
                if (SUCCEEDED(hr)) hr = pBitmapFrame->Commit();
                if (SUCCEEDED(hr)) hr = pEncoder->Commit();

                if (pFactory) pFactory->Release();
                if (pBitmapFrame) pBitmapFrame->Release();
                if (pEncoder) pEncoder->Release();
                if (pStream) pStream->Release();

                return (SUCCEEDED(hr));
            }
        };
    }
}