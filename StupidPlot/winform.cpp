#include <windows.h>
#include <CommCtrl.h>
#include <tchar.h>

#include <ribbon.h>
#include <resource.h>
#include <app.h>

#include <util.h>

using StupidPlot::App;
using StupidPlot::Ribbon;

BOOL CALLBACK PlotDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER(hDlg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
        Ribbon::init(hDlg);
        App::init(hDlg);
        return TRUE;
    case WM_SIZE:
    case WM_SIZING:
        if (App::hasInitialized())
        {
            App::updateSize();
            return TRUE;
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hDlg);
        return TRUE;
    case WM_DESTROY:
        Ribbon::destroy();
        PostQuitMessage(0);
        return TRUE;
    default:
        return App::handleEvent(message, wParam, lParam);
        break;
    }

    return FALSE;
}

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CoInitialize(NULL);

    HWND hDlg;
    MSG msg;
    BOOL ret;

    InitCommonControls();

    hDlg = CreateDialogParamW(
        hInstance,
        MAKEINTRESOURCEW(IDD_DIALOG_MAIN),
        NULL,
        PlotDialogProc,
        NULL
        );

    ShowWindow(hDlg, nCmdShow);

    while ((ret = GetMessageW(&msg, 0, 0, 0)) != 0)
    {
        if (ret == -1)
        {
            return -1;
        }
        if (!IsDialogMessageW(hDlg, &msg))
        {
            StupidPlot::Debug::Debug() << msg.message >> StupidPlot::Debug::writeln;
            if (msg.message != WM_MOUSEWHEEL)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                POINT mouse;
                GetCursorPos(&mouse);
                msg.hwnd = WindowFromPoint(mouse);
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    App::terminate();

    CoUninitialize();

    return 0;
}