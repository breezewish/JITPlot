#pragma once

#include <UIRibbon.h>

#include <uiapplication.h>

namespace StupidPlot
{
    class Ribbon
    {
    public:
        static IUIFramework     * g_pFramework;
        static IUIApplication   * g_pApplication;
        static IUIRibbon        * g_pRibbon;

        static bool init(HWND hWnd)
        {
            HRESULT hr = CoCreateInstance(
                CLSID_UIRibbonFramework,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&g_pFramework)
                );
            if (FAILED(hr)) return false;

            hr = UIApplication::CreateInstance(&g_pApplication);
            if (FAILED(hr)) return false;

            hr = g_pFramework->Initialize(hWnd, g_pApplication);
            if (FAILED(hr)) return false;

            hr = g_pFramework->LoadUI(GetModuleHandle(NULL), L"APPLICATION_RIBBON");
            if (FAILED(hr)) return false;

            return true;
        }

        static void destroy()
        {
            if (g_pFramework)
            {
                g_pFramework->Destroy();
                g_pFramework->Release();
                g_pFramework = NULL;
            }

            if (g_pApplication)
            {
                g_pApplication->Release();
                g_pApplication = NULL;
            }
        }

        static void showContextualUI(POINT & ptLocation)
        {
            UINT32 rh;
            g_pRibbon->GetHeight(&rh);

            IUIContextualUI * pContextualUI = NULL;
            g_pFramework->GetView(IDC_CMD_CONTEXT_PLOT, IID_PPV_ARGS(&pContextualUI));
            pContextualUI->ShowAtLocation(ptLocation.x, ptLocation.y + rh);
            pContextualUI->Release();
        }
    };
}