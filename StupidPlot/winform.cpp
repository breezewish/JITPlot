#include <windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include "resource.h"

#include "app.h"

using StupidPlot::App;

BOOL CALLBACK PlotDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER(hDlg);

    switch (message)
    {
    case WM_INITDIALOG:
        App::init(hDlg);
        return true;
    case WM_SIZE:
    case WM_SIZING:
        App::updateSize();
        return true;
    case WM_CLOSE:
        DestroyWindow(hDlg);
        return true;
    case WM_DESTROY:
        PostQuitMessage(0);
        return true;
    default:
        App::handleEvent(message, wParam, lParam);
        break;
    }

    return false;
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
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    App::terminate();

    return 0;
}