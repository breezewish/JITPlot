#include <windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include "resource.h"

#include "app.h"

using StupidPlot::App;
using StupidPlot::Debug;

App * app;

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
    case WM_SIZE:
    case WM_SIZING:
        app->updateSize();
        break;
    case WM_CLOSE:
        DestroyWindow(hDlg);
        return TRUE;
    case WM_DESTROY:
        PostQuitMessage(0);
        return TRUE;
    case WM_PAINT:
        Debug::output(L"%d winform.redraw\n", GetTickCount());
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

    app = new App(hDlg);

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

    delete app;

    return 0;
}