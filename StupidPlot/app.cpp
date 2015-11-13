#pragma region Include
#include <map>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>

#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>

#include <app.h>
#include <resource_ribbon.h>
#include <resource.h>
#include <throttler.h>
#include <ui/controls/control.h>
#include <ui/controls/win32control.h>
#include <ui/controls/ribboncontrol.h>
#include <ui/controls/checkbox.h>
#include <ui/controls/textbox.h>
#include <ui/controls/bufferedcanvas.h>
#include <ui/controls/listview.h>
#include <ui/container.h>
#include <ui/layout/layoutmanager.h>
#include <ui/events/event.h>
#include <ui/events/notifyevent.h>
#include <ui/events/canvasrebuildevent.h>
#include <ui/events/listviewendeditevent.h>
#include <ui/events/customdrawevent.h>
#include <ui/events/eventmanager.h>
#include <ui/events/ribbonupdatepropertyevent.h>
#include <ui/events/ribbonexecuteevent.h>
#include <plot/drawer.h>
#include <plot/graphic.h>
#include <plot/optionbag.h>
#include <plot/animation.h>
#include <formula/exp.h>
#include <formula/expdrawer.h>
#include <pointset/polylinedrawer.h>
#include <export/imageexporter.h>
#pragma endregion

namespace StupidPlot
{
    using std::map;
    using std::wstring;

    using namespace Gdiplus;
    using namespace UI;
    using namespace Layout;
    using namespace Controls;
    using namespace Events;
    using namespace Plot;
    using namespace Formula;
    using namespace Pointset;
    using namespace Export;

#pragma region Constants

    const double CANVAS_ENLARGE = 2.0;
    const double SCROLL_FACTOR = 240.0;

    const int HITTEST_RADIUS = 20;

    const vector<Gdiplus::Color> FORMULA_COLORS = {
        Gdiplus::Color(0, 176, 244),
        Gdiplus::Color(242, 95, 0),
        Gdiplus::Color(0, 187, 81),
        Gdiplus::Color(136, 90, 163),
        Gdiplus::Color(73, 126, 192),
        Gdiplus::Color(208, 0, 0),
        Gdiplus::Color(255, 193, 0)
    };

#pragma endregion

#pragma region Variables

    map<wstring, double>    mathConstants;

    GdiplusStartupInput     gdiplusStartupInput;
    ULONG_PTR               gdiplusToken;

    HWND                    hWnd;
    bool                    initialized = false;

    // ======== Managers ========
    ThrottlerPtr            throttler;
    ContainerPtr            container;
    LayoutManagerPtr        lm;
    EventManagerPtr         em;

    // ======== Win32 Controls ========
    Win32Control            * grpInfo;
    Win32Control            * lblInfoCursor;
    Win32Control            * lblInfoCursorX;
    Win32Control            * lblInfoCursorY;
    Win32Control            * lblInfoFormula;
    Win32Control            * lblInfoFormulaX;
    Win32Control            * lblInfoFormulaY;
    Textbox                 * lblInfoCursorXValue;
    Textbox                 * lblInfoCursorYValue;
    Textbox                 * lblInfoFormulaXValue;
    Textbox                 * lblInfoFormulaYValue;
    Win32Control            * grpCanvas;
    Checkbox                * chkAntialias;
    Checkbox                * chkShowGrid;
    Checkbox                * chkShowAxis;
    Win32Control            * lblGridInterval;
    Win32Control            * lblAxisInterval;
    Textbox                 * txtGridSize;
    Textbox                 * txtAxisSize;
    BufferedCanvas          * bmpCanvas;
    Win32Control            * lblRange;
    Win32Control            * lblRangeXFrom;
    Win32Control            * lblRangeXTo;
    Win32Control            * lblRangeYFrom;
    Win32Control            * lblRangeYTo;
    Textbox                 * txtRangeXFrom;
    Textbox                 * txtRangeXTo;
    Textbox                 * txtRangeYFrom;
    Textbox                 * txtRangeYTo;
    Win32Control            * grpPlot;
    ListView                * lstFormulas;
    Win32Control            * btnAdd;
    Win32Control            * btnRemove;

    // ======== Ribbon Controls ========
    RibbonControl           * rcmdFormulaColor;
    RibbonControl           * rcmdBgColor;
    RibbonControl           * rcmdShowGrid;
    RibbonControl           * rcmdShowAxis;
    RibbonControl           * rcmdSave;
    RibbonControl           * rcmdEditFormula;
    RibbonControl           * rcmdRemoveFormula;

    // ======== Data Struct -> UI ========
    CallbackFunction        syncRangeFromOption;
    CallbackFunction        syncGridFromOption;
    CallbackFunction        syncAxisFromOption;
    CallbackFunction        syncCursorPosition;

    // ======== Plot Drawing ========
    PlotOptionsPtr          options;
    PlotDrawerPtr           drawer;
    double                  initialLeft, initialRight, initialTop, initialBottom;

    // ======== Scaling ========
    AnimationPtr            animation;
    double                  scaleFactor = 1.0;
    int                     scrollValue = 0;
    int                     completedScrollValue = 0;
    double                  scaleOriginX, scaleOriginY;

    // ======== Cursor Resources ========
    HCURSOR                 hCursorDefault;
    HCURSOR                 hCursorCross;
    HCURSOR                 hCursorHand;
    HCURSOR                 hCurrentCursor;
    HCURSOR                 hOldCursor;

    // ======== Cursor Position ========
    int                     currentCursorX, currentCursorY;
    double                  currentCursorFormulaX, currentCursorFormulaY;

    // ======== Formula Hot Track Position ========
    double                  currentHTX, currentHTY;
    CallbackFunction        updateCursorHitTest;

    // ======== Context menu ========
    int                     cmFormulaIdx;
    bool                    cmHasMouseMoved;

#pragma endregion

    void setup();
    void _updateCursorHitTest();
    void _syncRangeFromOption();
    void _syncGridFromOption();
    void _syncAxisFromOption();
    void _syncCursorPosition();

#pragma region Public Interface

    bool App::hasInitialized()
    {
        return initialized;
    }

    void App::init(HWND _hWnd)
    {
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        hCursorDefault = LoadCursorW(NULL, IDC_ARROW);
        hCursorCross = LoadCursorW(NULL, IDC_CROSS);
        hCursorHand = LoadCursorW(NULL, IDC_HAND);
        hCurrentCursor = hCursorCross;

        hWnd = _hWnd;

        throttler = ThrottlerPtr(new Throttler(hWnd));
        syncRangeFromOption = throttler->applyThrottle(300, CallbackFunction(_syncRangeFromOption));
        syncGridFromOption = CallbackFunction(_syncGridFromOption);
        syncAxisFromOption = CallbackFunction(_syncAxisFromOption);
        syncCursorPosition = throttler->applyThrottle(30, CallbackFunction(_syncCursorPosition));
        updateCursorHitTest = throttler->applyThrottle(30, CallbackFunction(_updateCursorHitTest));

        container = ContainerPtr(new Container());
        lm = LayoutManagerPtr(new LayoutManager(hWnd));
        em = EventManagerPtr(new EventManager(container));

        bmpCanvas = new BufferedCanvas(hWnd, IDC_STATIC_CANVAS, CANVAS_ENLARGE);
        grpInfo = new Win32Control(hWnd, IDC_STATIC_GROUP_INFO);
        lblInfoCursor = new Win32Control(hWnd, IDC_STATIC_INFO_CURSOR);
        lblInfoCursorX = new Win32Control(hWnd, IDC_STATIC_INFO_CURSOR_X);
        lblInfoCursorY = new Win32Control(hWnd, IDC_STATIC_INFO_CURSOR_Y);
        lblInfoFormula = new Win32Control(hWnd, IDC_STATIC_INFO_FORMULA);
        lblInfoFormulaX = new Win32Control(hWnd, IDC_STATIC_INFO_FORMULA_X);
        lblInfoFormulaY = new Win32Control(hWnd, IDC_STATIC_INFO_FORMULA_Y);
        lblInfoCursorXValue = new Textbox(hWnd, IDC_STATIC_INFO_CURSOR_X_VALUE);
        lblInfoCursorYValue = new Textbox(hWnd, IDC_STATIC_INFO_CURSOR_Y_VALUE);
        lblInfoFormulaXValue = new Textbox(hWnd, IDC_STATIC_INFO_FORMULA_X_VALUE);
        lblInfoFormulaYValue = new Textbox(hWnd, IDC_STATIC_INFO_FORMULA_Y_VALUE);
        grpCanvas = new Win32Control(hWnd, IDC_STATIC_GROUP_CANVAS);
        chkAntialias = new Checkbox(hWnd, IDC_CHECK_ANTIALIAS);
        chkShowGrid = new Checkbox(hWnd, IDC_CHECK_SHOW_GRID);
        chkShowAxis = new Checkbox(hWnd, IDC_CHECK_SHOW_AXIS);
        txtGridSize = new Textbox(hWnd, IDC_EDIT_GRID_SIZE);
        txtAxisSize = new Textbox(hWnd, IDC_EDIT_AXIS_SIZE);
        lblGridInterval = new Win32Control(hWnd, IDC_STATIC_GRID_INTERVAL);
        lblAxisInterval = new Win32Control(hWnd, IDC_STATIC_AXIS_INTERVAL);
        lblRange = new Win32Control(hWnd, IDC_STATIC_RANGE);
        lblRangeXFrom = new Win32Control(hWnd, IDC_STATIC_RANGE_X_FROM);
        lblRangeXTo = new Win32Control(hWnd, IDC_STATIC_RANGE_X_TO);
        lblRangeYFrom = new Win32Control(hWnd, IDC_STATIC_RANGE_Y_FROM);
        lblRangeYTo = new Win32Control(hWnd, IDC_STATIC_RANGE_Y_TO);
        txtRangeXFrom = new Textbox(hWnd, IDC_EDIT_X_FROM);
        txtRangeXTo = new Textbox(hWnd, IDC_EDIT_X_TO);
        txtRangeYFrom = new Textbox(hWnd, IDC_EDIT_Y_FROM);
        txtRangeYTo = new Textbox(hWnd, IDC_EDIT_Y_TO);
        grpPlot = new Win32Control(hWnd, IDC_STATIC_GROUP_PLOT);
        lstFormulas = new ListView(hWnd, IDC_LIST_FORMULAS);
        btnAdd = new Win32Control(hWnd, IDC_BUTTON_ADD);
        btnRemove = new Win32Control(hWnd, IDC_BUTTON_REMOVE);

        rcmdFormulaColor = new RibbonControl(IDR_CMD_FORMULA_COLORPICKER);
        rcmdBgColor = new RibbonControl(IDR_CMD_BACKGROUND_COLORPICKER);
        rcmdShowGrid = new RibbonControl(IDR_CMD_SHOW_GRID);
        rcmdShowAxis = new RibbonControl(IDR_CMD_SHOW_AXIS);
        rcmdSave = new RibbonControl(IDR_CMD_SAVE);
        rcmdEditFormula = new RibbonControl(IDR_CMD_EDIT_FORMULA);
        rcmdRemoveFormula = new RibbonControl(IDR_CMD_REMOVE_FORMULA);

        chkAntialias->setChecked(true);
        chkShowGrid->setChecked(true);
        chkShowAxis->setChecked(true);
        btnRemove->setEnabled(false);

        container
            ->addWin32Control(bmpCanvas)
            ->addWin32Control(grpInfo)
            ->addWin32Control(lblInfoCursor)
            ->addWin32Control(lblInfoCursorX)
            ->addWin32Control(lblInfoCursorY)
            ->addWin32Control(lblInfoFormula)
            ->addWin32Control(lblInfoFormulaX)
            ->addWin32Control(lblInfoFormulaY)
            ->addWin32Control(lblInfoCursorXValue)
            ->addWin32Control(lblInfoCursorYValue)
            ->addWin32Control(lblInfoFormulaXValue)
            ->addWin32Control(lblInfoFormulaYValue)
            ->addWin32Control(grpCanvas)
            ->addWin32Control(chkAntialias)
            ->addWin32Control(chkShowGrid)
            ->addWin32Control(chkShowAxis)
            ->addWin32Control(txtGridSize)
            ->addWin32Control(txtAxisSize)
            ->addWin32Control(lblGridInterval)
            ->addWin32Control(lblAxisInterval)
            ->addWin32Control(lblRange)
            ->addWin32Control(lblRangeXFrom)
            ->addWin32Control(lblRangeXTo)
            ->addWin32Control(lblRangeYFrom)
            ->addWin32Control(lblRangeYTo)
            ->addWin32Control(txtRangeXFrom)
            ->addWin32Control(txtRangeXTo)
            ->addWin32Control(txtRangeYFrom)
            ->addWin32Control(txtRangeYTo)
            ->addWin32Control(grpPlot)
            ->addWin32Control(lstFormulas)
            ->addWin32Control(btnAdd)
            ->addWin32Control(btnRemove)
            ->addRibbonControl(rcmdFormulaColor)
            ->addRibbonControl(rcmdBgColor)
            ->addRibbonControl(rcmdShowGrid)
            ->addRibbonControl(rcmdShowAxis)
            ->addRibbonControl(rcmdSave)
            ->addRibbonControl(rcmdEditFormula)
            ->addRibbonControl(rcmdRemoveFormula);

        lm
            ->enableMagnet(bmpCanvas, true, true, true, true)
            ->enableMagnet(grpInfo, false, true, true, false)
            ->enableMagnet(lblInfoCursor, false, true, true, false)
            ->enableMagnet(lblInfoCursorX, false, true, true, false)
            ->enableMagnet(lblInfoCursorY, false, true, true, false)
            ->enableMagnet(lblInfoFormula, false, true, true, false)
            ->enableMagnet(lblInfoFormulaX, false, true, true, false)
            ->enableMagnet(lblInfoFormulaY, false, true, true, false)
            ->enableMagnet(lblInfoCursorXValue, false, true, true, false)
            ->enableMagnet(lblInfoCursorYValue, false, true, true, false)
            ->enableMagnet(lblInfoFormulaXValue, false, true, true, false)
            ->enableMagnet(lblInfoFormulaYValue, false, true, true, false)
            ->enableMagnet(grpCanvas, false, true, true, false)
            ->enableMagnet(chkAntialias, false, true, true, false)
            ->enableMagnet(chkShowGrid, false, true, true, false)
            ->enableMagnet(chkShowAxis, false, true, true, false)
            ->enableMagnet(txtGridSize, false, true, true, false)
            ->enableMagnet(txtAxisSize, false, true, true, false)
            ->enableMagnet(lblGridInterval, false, true, true, false)
            ->enableMagnet(lblAxisInterval, false, true, true, false)
            ->enableMagnet(lblRange, false, true, true, false)
            ->enableMagnet(lblRangeXFrom, false, true, true, false)
            ->enableMagnet(lblRangeXTo, false, true, true, false)
            ->enableMagnet(lblRangeYFrom, false, true, true, false)
            ->enableMagnet(lblRangeYTo, false, true, true, false)
            ->enableMagnet(txtRangeXFrom, false, true, true, false)
            ->enableMagnet(txtRangeXTo, false, true, true, false)
            ->enableMagnet(txtRangeYFrom, false, true, true, false)
            ->enableMagnet(txtRangeYTo, false, true, true, false)
            ->enableMagnet(grpPlot, false, true, true, true)
            ->enableMagnet(lstFormulas, false, true, true, true)
            ->enableMagnet(btnAdd, false, false, true, true)
            ->enableMagnet(btnRemove, false, false, true, true);

        updateSize();
        setup();

        initialized = true;
    }

    void App::terminate()
    {
        delete grpInfo;
        delete lblInfoCursor;
        delete lblInfoCursorX;
        delete lblInfoCursorY;
        delete lblInfoFormula;
        delete lblInfoFormulaX;
        delete lblInfoFormulaY;
        delete lblInfoCursorXValue;
        delete lblInfoCursorYValue;
        delete lblInfoFormulaXValue;
        delete lblInfoFormulaYValue;
        delete grpCanvas;
        delete chkAntialias;
        delete chkShowGrid;
        delete chkShowAxis;
        delete lblGridInterval;
        delete lblAxisInterval;
        delete txtGridSize;
        delete txtAxisSize;
        delete bmpCanvas;
        delete lblRange;
        delete lblRangeXFrom;
        delete lblRangeXTo;
        delete lblRangeYFrom;
        delete lblRangeYTo;
        delete txtRangeXFrom;
        delete txtRangeXTo;
        delete txtRangeYFrom;
        delete txtRangeYTo;
        delete grpPlot;
        delete lstFormulas;
        delete btnAdd;
        delete btnRemove;

        delete rcmdFormulaColor;
        delete rcmdBgColor;
        delete rcmdShowGrid;
        delete rcmdShowAxis;
        delete rcmdSave;
        delete rcmdEditFormula;
        delete rcmdRemoveFormula;

        // TODO: fix this hack
        // drawer destructor should be called before GdiplusShutdown
        drawer = NULL;

        GdiplusShutdown(gdiplusToken);
    }

    void App::updateSize()
    {
        lm->updateSize();
        lm->relayout();
    }

    BOOL App::handleEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_TIMER:
            if (wParam == IDT_TIMER_PLOT)
            {
                animation->update();
                return FALSE;
            }
            else if (throttler->canHandleTick(wParam))
            {
                throttler->handleTick(wParam);
                return FALSE;
            }
            break;
        case WM_NOTIFY:
        {
            auto nmh = *(reinterpret_cast<LPNMHDR>(lParam));
            auto control = container->getWin32ControlById(nmh.idFrom);
            if (control != NULL)
            {
                auto ret = control->dispatchEvent(EventName::EVENT_NOTIFY, EventPtr(new NotifyEvent(wParam, lParam)));
                if (ret != LRESULT_DEFAULT)
                {
                    SetWindowLongW(hWnd, DWL_MSGRESULT, ret);
                    return TRUE;
                }
            }
            break;
        }
        }

        return em->handleWin32Message(uMsg, wParam, lParam);
    }

    HRESULT App::handleRibbonUpdateProperty(UINT nCmdID,
        REFPROPERTYKEY key,
        const PROPVARIANT* ppropvarCurrentValue,
        PROPVARIANT* ppropvarNewValue)
    {
        if (ppropvarNewValue != NULL)
        {
            auto control = container->getRibbonControlById(nCmdID);
            if (control != NULL)
            {
                control->dispatchEvent(EventName::EVENT_RIBBON_UPDATE_PROPERTY, EventPtr(new RibbonUpdatePropertyEvent(key, ppropvarCurrentValue, ppropvarNewValue)));
                return S_OK;
            }
        }
        return E_NOTIMPL;
    }

    void App::handleRibbonExecute(UINT nCmdID,
        UI_EXECUTIONVERB verb,
        const PROPERTYKEY* key,
        const PROPVARIANT* ppropvarValue,
        IUISimplePropertySet* pCommandExecutionProperties)
    {
        auto control = container->getRibbonControlById(nCmdID);
        if (control != NULL)
        {
            control->dispatchEvent(EventName::EVENT_RIBBON_EXECUTE, EventPtr(new RibbonExecuteEvent(verb, key, ppropvarValue, pCommandExecutionProperties)));
        }
    }

#pragma endregion

#pragma region Helper Functions

    void saveAndApplyCursor(HCURSOR cursor)
    {
        hOldCursor = hCurrentCursor;
        hCurrentCursor = cursor;
        SetCursor(hCurrentCursor);
    }

    void restoreCursor()
    {
        hCurrentCursor = hOldCursor;
        SetCursor(hCurrentCursor);
    }

    inline void vpTakeShapshot()
    {
        initialLeft = options->vpLeft;
        initialRight = options->vpRight;
        initialTop = options->vpTop;
        initialBottom = options->vpBottom;
    }

    inline void vpRevertSnapshot()
    {
        options->vpLeft = initialLeft;
        options->vpRight = initialRight;
        options->vpTop = initialTop;
        options->vpBottom = initialBottom;
        syncRangeFromOption();
    }

    inline void updateDrawerRange()
    {
        drawer->updateDrawingRange(bmpCanvas->canvasW, bmpCanvas->canvasH, bmpCanvas->vpX, bmpCanvas->vpY, bmpCanvas->width, bmpCanvas->height);
    }

    inline void removeFormulaByIndex(int index)
    {
        if (index >= 0 && index < static_cast<int>(options->graphics.size()))
        {
            options->graphics.erase(options->graphics.begin() + index);
            lstFormulas->deleteRow(index);

            if (options->graphics.size() > 0)
            {
                cmFormulaIdx = 0;
                drawer->activeExpIdx = 0;
            }
            else
            {
                cmFormulaIdx = -1;
                drawer->activeExpIdx = -1;
            }

            drawer->hoverExpIdx = -1;
            drawer->htCanvasX = -1;
            drawer->htCanvasY = -1;
            saveAndApplyCursor(hCursorCross);

            bmpCanvas->dispatchRedraw();
        }
    }

    inline void showRibbonContextUI(UINT32 viewId)
    {
        POINT pt;
        GetCursorPos(&pt);

        // invalidate color
        Ribbon::g_pFramework->InvalidateUICommand(IDR_CMD_FORMULA_COLORPICKER, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_Color);

        // show UI
        IUIContextualUI * pContextualUI = NULL;
        Ribbon::g_pFramework->GetView(viewId, IID_PPV_ARGS(&pContextualUI));
        pContextualUI->ShowAtLocation(pt.x, pt.y);
        pContextualUI->Release();
    }

#pragma endregion

#pragma region Event Handlers

#pragma region Ribbon Handlers

    inline LRESULT rcmdFormulaColor_onUpdateProperty(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<RibbonUpdatePropertyEvent>(_event);

        if (event->key == UI_PKEY_ColorType)
        {
            UIInitPropertyFromUInt32(event->key, UI_SWATCHCOLORTYPE_RGB, event->newValue);
        }
        else if (event->key == UI_PKEY_Color)
        {
            if (cmFormulaIdx >= 0 && cmFormulaIdx < static_cast<int>(options->graphics.size()))
            {
                UIInitPropertyFromUInt32(
                    event->key,
                    options->graphics[cmFormulaIdx]->color.ToCOLORREF(),
                    event->newValue
                    );
            }
        }

        return LRESULT_DEFAULT;
    }

    inline LRESULT rcmdFormulaColor_onExecute(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<RibbonExecuteEvent>(_event);

        if (event->verb != UI_EXECUTIONVERB_EXECUTE) return LRESULT_DEFAULT;

        if (cmFormulaIdx >= 0 && cmFormulaIdx < static_cast<int>(options->graphics.size()))
        {
            UINT color = 0;
            PROPVARIANT var;
            event->properties->GetValue(UI_PKEY_Color, &var);
            UIPropertyToUInt32(UI_PKEY_Color, var, &color);
            options->graphics[cmFormulaIdx]->color.SetFromCOLORREF(color);
            bmpCanvas->dispatchRedraw();
        }

        return LRESULT_DEFAULT;
    }

    inline LRESULT rcmdBgColor_onUpdateProperty(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<RibbonUpdatePropertyEvent>(_event);

        if (event->key == UI_PKEY_ColorType)
        {
            UIInitPropertyFromUInt32(event->key, UI_SWATCHCOLORTYPE_RGB, event->newValue);
        }
        else if (event->key == UI_PKEY_Color)
        {
            UIInitPropertyFromUInt32(
                event->key,
                options->backgroundColor.ToCOLORREF(),
                event->newValue
                );
        }

        return LRESULT_DEFAULT;
    }

    inline LRESULT rcmdBgColor_onExecute(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<RibbonExecuteEvent>(_event);

        if (event->verb != UI_EXECUTIONVERB_EXECUTE) return LRESULT_DEFAULT;

        UINT color = 0;
        PROPVARIANT var;
        event->properties->GetValue(UI_PKEY_Color, &var);
        UIPropertyToUInt32(UI_PKEY_Color, var, &color);
        options->backgroundColor.SetFromCOLORREF(color);
        bmpCanvas->dispatchRedraw();

        return LRESULT_DEFAULT;
    }

    inline LRESULT rcmdShowGrid_onExecute(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        PROPVARIANT var;
        HRESULT hr = Ribbon::g_pFramework->GetUICommandProperty(IDR_CMD_SHOW_GRID, UI_PKEY_BooleanValue, &var);
        if (FAILED(hr)) return LRESULT_DEFAULT;

        BOOL val;
        hr = PropVariantToBoolean(var, &val);
        if (FAILED(hr)) return LRESULT_DEFAULT;

        options->showGrid = (val == TRUE);
        syncGridFromOption();
        bmpCanvas->dispatchRedraw();

        return LRESULT_DEFAULT;
    }

    inline LRESULT rcmdShowAxis_onExecute(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        PROPVARIANT var;
        HRESULT hr = Ribbon::g_pFramework->GetUICommandProperty(IDR_CMD_SHOW_AXIS, UI_PKEY_BooleanValue, &var);
        if (FAILED(hr)) return LRESULT_DEFAULT;

        BOOL val;
        hr = PropVariantToBoolean(var, &val);
        if (FAILED(hr)) return LRESULT_DEFAULT;

        options->showAxis = (val == TRUE);
        syncGridFromOption();
        bmpCanvas->dispatchRedraw();

        return LRESULT_DEFAULT;
    }

    inline LRESULT rcmdShowGrid_onUpdateProperty(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<RibbonUpdatePropertyEvent>(_event);

        if (event->key == UI_PKEY_BooleanValue)
        {
            UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, options->showGrid, event->newValue);
        }

        return LRESULT_DEFAULT;
    }

    inline LRESULT rcmdShowAxis_onUpdateProperty(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<RibbonUpdatePropertyEvent>(_event);

        if (event->key == UI_PKEY_BooleanValue)
        {
            UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, options->showAxis, event->newValue);
        }

        return LRESULT_DEFAULT;
    }

    inline LRESULT rcmdEditFormula_onExecute(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        lstFormulas->setSelected(-1, false);
        lstFormulas->beginEdit(cmFormulaIdx);

        return LRESULT_DEFAULT;
    }

    inline LRESULT rcmdRemoveFormula_onExecute(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        removeFormulaByIndex(cmFormulaIdx);

        return LRESULT_DEFAULT;
    }

    inline LRESULT rcmdSave_onExecute(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        WCHAR filename[MAX_PATH] = L"";

        OPENFILENAMEW ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWnd;
        ofn.hInstance = 0;
        ofn.lpstrFilter = L"BMP (*.bmp)\0*.bmp\0JPEG (*.jpg)\0*.jpg\0PNG (*.png)\0*.png\0All\0*.*\0";
        ofn.nFilterIndex = 3;
        ofn.lpstrTitle = L"Export viewport";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_EXPLORER;
        ofn.lpstrDefExt = L"png";

        BOOL bResult = GetSaveFileNameW(&ofn);

        if (bResult)
        {
            ImageFormat format;
            wstring fn(filename);
            wstring ext = fn.substr(fn.find_last_of(L".") + 1);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

            if (ext.compare(L"bmp") == 0)
            {
                format = ImageFormat::BMP;
            }
            else if (ext.compare(L"jpg") || ext.compare(L"jpeg"))
            {
                format = ImageFormat::JPEG;
            }
            else
            {
                format = ImageFormat::PNG;
            }

            drawer->hoverExpIdx = -1;
            drawer->drawPlot();
            bool saveStatus = ImageExporter::save(drawer->bitmap, format, wstring(filename));
            if (!saveStatus)
            {
                MessageBoxW(hWnd, L"Failed to write file.", L"StupidPlot", MB_ICONWARNING);
            }
            else
            {
                MessageBoxW(hWnd, L"Export successfully.", L"StupidPlot", MB_ICONINFORMATION);
            }
        }

        return LRESULT_DEFAULT;
    }
#pragma endregion

#pragma region ListView Handlers
    inline LRESULT lstFormulas_onEndEdit(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<ListViewEndEditEvent>(_event);

        if (event->displayInfo->item.pszText == NULL)
        {
            return FALSE;
        }

        auto idx = event->displayInfo->item.iItem;
        auto gExp = GraphicPtr(new ExpDrawer(
            wstring(event->displayInfo->item.pszText),
            mathConstants,
            options->graphics[idx]->color
            ));

        options->graphics[idx] = gExp;

        if (!gExp->isValid)
        {
            MessageBoxW(
                hWnd,
                gExp->errorMessage.c_str(),
                L"StupidPlot",
                MB_ICONWARNING
                );
        }

        bmpCanvas->dispatchRedraw();

        return TRUE;
    }

    inline LRESULT lstFormulas_onCustomDraw(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<CustomDrawEvent>(_event);
        auto nmcd = event->customDraw->nmcd;

        switch (nmcd.dwDrawStage)
        {
        case CDDS_PREPAINT:
            return CDRF_NOTIFYITEMDRAW;
        case CDDS_ITEMPREPAINT:
        {
            int nItem = static_cast<int>(nmcd.dwItemSpec);
            if (nItem >= 0 && nItem < static_cast<int>(options->graphics.size()))
            {
                if (!options->graphics[nItem]->isValid)
                {
                    event->customDraw->clrText = RGB(128, 128, 128);
                }
            }
            return CDRF_NOTIFYPOSTPAINT;
        }
        case CDDS_ITEMPOSTPAINT:
        {
            int nItem = static_cast<int>(nmcd.dwItemSpec);
            if (nItem >= 0 && nItem < static_cast<int>(options->graphics.size()))
            {
                RECT rc;
                lstFormulas->getRect(nItem, &rc, LVIR_ICON);
                InflateRect(&rc, -3, -3);

                HBRUSH brush = CreateSolidBrush(options->graphics[nItem]->color.ToCOLORREF());
                FillRect(nmcd.hdc, &rc, brush);
                DeleteObject(brush);

                return CDRF_SKIPDEFAULT;
            }
        }
        default:
            return LRESULT_DEFAULT;
        }
    }

    inline LRESULT lstFormulas_onNotify(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<NotifyEvent>(_event);

        switch (event->nmh.code)
        {
        case NM_DBLCLK:
        {
            auto item = reinterpret_cast<NMITEMACTIVATE *>(event->lParam);
            lstFormulas->beginEdit(item->iItem);
            break;
        }
        case LVN_ITEMCHANGED:
        {
            auto idx = lstFormulas->getSelectedIndex();
            if (idx != -1)
            {
                drawer->activeExpIdx = idx;
                btnRemove->setEnabled(TRUE);
            }
            else
            {
                btnRemove->setEnabled(FALSE);
            }
            break;
        }
        case NM_RCLICK:
        {
            auto item = reinterpret_cast<NMITEMACTIVATE *>(event->lParam);
            cmFormulaIdx = item->iItem;
            showRibbonContextUI(IDR_CMD_CONTEXT_FORMULA);
        }
        }

        return LRESULT_DEFAULT;
    }

#pragma endregion

#pragma region ControlPad Handlers

    inline LRESULT btnAdd_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        auto color = FORMULA_COLORS[options->graphics.size() % FORMULA_COLORS.size()];
        options->graphics.push_back(GraphicPtr(new ExpDrawer(L"", mathConstants, color)));
        auto index = lstFormulas->insertRow(L"");
        lstFormulas->setSelected(-1, false);
        lstFormulas->beginEdit(index);

        return LRESULT_DEFAULT;
    }

    inline LRESULT btnRemove_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        auto index = lstFormulas->getSelectedIndex();
        removeFormulaByIndex(index);

        return LRESULT_DEFAULT;
    }

    inline LRESULT chkShowGrid_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        txtGridSize->setEnabled(chkShowGrid->isChecked());
        options->showGrid = chkShowGrid->isChecked();
        Ribbon::g_pFramework->InvalidateUICommand(IDR_CMD_SHOW_GRID, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
        bmpCanvas->dispatchRedraw();

        return LRESULT_DEFAULT;
    }

    inline LRESULT txtGridSize_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        options->gridSpacing = std::stoi(txtGridSize->getText());
        bmpCanvas->dispatchRedraw();

        return LRESULT_DEFAULT;
    }

    inline LRESULT chkShowAxis_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        txtAxisSize->setEnabled(chkShowAxis->isChecked());
        options->showAxis = chkShowAxis->isChecked();
        Ribbon::g_pFramework->InvalidateUICommand(IDR_CMD_SHOW_AXIS, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
        bmpCanvas->dispatchRedraw();

        return LRESULT_DEFAULT;
    }

    inline LRESULT txtAxisSize_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        options->axisTickInterval = std::stoi(txtAxisSize->getText());
        bmpCanvas->dispatchRedraw();

        return LRESULT_DEFAULT;
    }

    inline LRESULT txtRangeXFrom_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        double val = std::stod(txtRangeXFrom->getText());
        if (val >= options->vpRight)
        {
            _syncRangeFromOption();
            return LRESULT_DEFAULT;
        }
        options->vpLeft = val;
        options->calculateOuterBoundaryInCenter(CANVAS_ENLARGE);
        _syncRangeFromOption();
        bmpCanvas->dispatchRedraw();

        return LRESULT_DEFAULT;
    }

    inline LRESULT txtRangeXTo_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        double val = std::stod(txtRangeXTo->getText());
        if (val <= options->vpLeft)
        {
            _syncRangeFromOption();
            return LRESULT_DEFAULT;
        }
        options->vpRight = val;
        options->calculateOuterBoundaryInCenter(CANVAS_ENLARGE);
        _syncRangeFromOption();
        bmpCanvas->dispatchRedraw();

        return LRESULT_DEFAULT;
    }

    inline LRESULT txtRangeYFrom_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        double val = std::stod(txtRangeYFrom->getText());
        if (val >= options->vpTop)
        {
            _syncRangeFromOption();
            return LRESULT_DEFAULT;
        }
        options->vpBottom = val;
        options->calculateOuterBoundaryInCenter(CANVAS_ENLARGE);
        _syncRangeFromOption();
        bmpCanvas->dispatchRedraw();

        return LRESULT_DEFAULT;
    }

    inline LRESULT txtRangeYTo_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        double val = std::stod(txtRangeYTo->getText());
        if (val <= options->vpBottom)
        {
            _syncRangeFromOption();
            return LRESULT_DEFAULT;
        }
        options->vpTop = val;
        options->calculateOuterBoundaryInCenter(CANVAS_ENLARGE);
        _syncRangeFromOption();
        bmpCanvas->dispatchRedraw();

        return LRESULT_DEFAULT;
    }

    inline LRESULT chkAntialias_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        drawer->setAntialias(chkAntialias->isChecked());
        bmpCanvas->dispatchRedraw();

        return LRESULT_DEFAULT;
    }
#pragma endregion

#pragma region Canvas handlers
    inline LRESULT bmpCanvas_onRedrawBuffer(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        // We need to make sure that vp x <-> formula left
        double dx = drawer->translateCanvasW(bmpCanvas->vpX);
        double dy = drawer->translateCanvasH(bmpCanvas->vpY);
        double fcw = drawer->translateCanvasW(bmpCanvas->canvasW);
        double fch = drawer->translateCanvasH(bmpCanvas->canvasH);
        double fvw = drawer->translateCanvasW(bmpCanvas->width);
        double fvh = drawer->translateCanvasH(bmpCanvas->height);
        options->vpRight = options->vpLeft + fvw;
        options->vpTop = options->vpBottom + fvh;
        options->drawLeft = options->vpLeft - dx;
        options->drawBottom = options->vpBottom - dy;
        options->drawRight = options->drawLeft + fcw;
        options->drawTop = options->drawBottom + fch;
        syncRangeFromOption();
        updateDrawerRange();
        drawer->drawPlot();

        return LRESULT_DEFAULT;
    }

    inline LRESULT bmpCanvas_onCanvasBeginMove(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        saveAndApplyCursor(hCursorDefault);
        vpTakeShapshot();

        return LRESULT_DEFAULT;
    }

    inline LRESULT bmpCanvas_onCanvasEndMove(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        restoreCursor();

        return LRESULT_DEFAULT;
    }

    inline LRESULT bmpCanvas_onCanvasMove(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<CanvasMoveEvent>(_event);

        double dx = drawer->translateCanvasW(event->dx);
        double dy = drawer->translateCanvasH(event->dy);
        vpRevertSnapshot();
        options->vpLeft -= dx;
        options->vpRight -= dx;
        options->vpTop += dy;
        options->vpBottom += dy;
        syncRangeFromOption();

        return LRESULT_DEFAULT;
    }

    inline void updateHotTrackPosition()
    {
        if (drawer->activeExpIdx >= static_cast<int>(options->graphics.size()))
        {
            drawer->htCanvasX = -1;
            drawer->htCanvasY = -1;
            return;
        }

        auto g = options->graphics[drawer->activeExpIdx];

        if (!g->isValid)
        {
            drawer->htCanvasX = -1;
            drawer->htCanvasY = -1;
            return;
        }
        if (g->type != GraphicType::FORMULA_EXPRESSION)
        {
            drawer->htCanvasX = -1;
            drawer->htCanvasY = -1;
            return;
        }

        auto exp = std::dynamic_pointer_cast<ExpDrawer>(g);

        drawer->htCanvasX = currentCursorX;
        currentHTX = currentCursorFormulaX;
        currentHTY = exp->expression->eval(currentHTX);
        drawer->htCanvasY = static_cast<int>(drawer->translateFormulaY(currentHTY));
    }

    inline LRESULT bmpCanvas_onMouseMove(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

        cmHasMouseMoved = true;

        // current cursor position
        currentCursorX = bmpCanvas->vpX + event->x;
        currentCursorY = bmpCanvas->vpY + event->y;
        currentCursorFormulaX = drawer->translateCanvasX(currentCursorX);
        currentCursorFormulaY = drawer->translateCanvasY(currentCursorY);
        updateCursorHitTest();

        // current plot position
        if (options->enableHotTrack
            && drawer->activeExpIdx >= 0
            && drawer->activeExpIdx < static_cast<int>(options->graphics.size())
            )
        {
            updateHotTrackPosition();

            drawer->refresh();
            bmpCanvas->refresh();
        }

        syncCursorPosition();

        return LRESULT_DEFAULT;
    }

    void scaleReset()
    {
        scrollValue -= completedScrollValue;
        scaleFactor = 1.0;
        completedScrollValue = 0;
        vpTakeShapshot();
    }

    inline void bmpCanvas_onScaleBegin(const shared_ptr<MouseWheelEvent> & event)
    {
        if (bmpCanvas->isMoving) return;
        bmpCanvas->canMove = false;
        saveAndApplyCursor(hCursorDefault);
        vpTakeShapshot();
        scaleOriginX = drawer->translateCanvasX(event->x + bmpCanvas->vpX);
        scaleOriginY = drawer->translateCanvasY(event->y + bmpCanvas->vpY);
        drawer->clipToViewport();
    }

    inline void bmpCanvas_onScaleMore(const shared_ptr<MouseWheelEvent> & event)
    {
        if (bmpCanvas->isMoving) return;
        UNREFERENCED_PARAMETER(event);
        scaleReset();
    }

    inline void bmpCanvas_onScale(const shared_ptr<MouseWheelEvent> & event)
    {
        if (bmpCanvas->isMoving) return;
        scrollValue -= event->delta;
        animation->reset();
    }

    inline void bmpCanvas_onScaleEnd()
    {
        scaleReset();
        restoreCursor();
        bmpCanvas->canMove = true;
        drawer->resetClipToCanvas();
        bmpCanvas->dispatchRedraw();
    }

    inline void bmpCanvas_onScaleAnimationProgress(double k)
    {
        double scaleFactor;

        completedScrollValue = static_cast<int>(k * scrollValue);
        if (completedScrollValue > 0)
        {
            scaleFactor = 1.0 + static_cast<double>(completedScrollValue) / SCROLL_FACTOR;
        }
        else if (completedScrollValue < 0)
        {
            scaleFactor = 1.0 / (1 + static_cast<double>(-completedScrollValue) / SCROLL_FACTOR);
        }
        else
        {
            scaleFactor = 1.0;
        }

        vpRevertSnapshot();
        options->scaleViewportBoundary(scaleOriginX, scaleOriginY, scaleFactor);
        options->calculateOuterBoundaryInCenter(CANVAS_ENLARGE);
        syncRangeFromOption();

        if (options->enableHotTrack
            && drawer->activeExpIdx >= 0
            && drawer->activeExpIdx < static_cast<int>(options->graphics.size())
            )
        {
            updateHotTrackPosition();
        }

        bmpCanvas->dispatchRedraw();
        bmpCanvas->refresh();
    }

    inline void bmpCanvas_onScaleAnimationComplete()
    {
        bmpCanvas_onScaleEnd();
    }

    inline LRESULT bmpCanvas_onMouseWheel(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<MouseWheelEvent>(_event);

        if (!animation->isRunning())
        {
            bmpCanvas_onScaleBegin(event);
        }
        else
        {
            bmpCanvas_onScaleMore(event);
        }

        bmpCanvas_onScale(event);

        return LRESULT_DEFAULT;
    }

    inline LRESULT bmpCanvas_onCanvasRebuild(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<CanvasRebuildEvent>(_event);

        drawer->rebuildBuffer(event->canvasWidth, event->canvasHeight);

        return LRESULT_DEFAULT;
    }

    inline LRESULT bmpCanvas_onSetCursor(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        SetCursor(hCurrentCursor);

        return LRESULT_DEFAULT;
    }

    inline LRESULT bmpCanvas_onMouseUp(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

        // left click on formula: show formula context menu
        if (!cmHasMouseMoved && event->button == MouseButton::LEFT && drawer->hoverExpIdx != -1)
        {
            cmFormulaIdx = drawer->hoverExpIdx;
            drawer->activeExpIdx = cmFormulaIdx;
            showRibbonContextUI(IDR_CMD_CONTEXT_FORMULA);
        }

        return LRESULT_DEFAULT;
    }

    inline LRESULT bmpCanvas_onMouseDown(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

        cmHasMouseMoved = false;

        if (event->button == MouseButton::RIGHT)
        {
            cmFormulaIdx = drawer->hoverExpIdx;
            showRibbonContextUI(
                cmFormulaIdx == -1 ?
                IDR_CMD_CONTEXT_PLOT :
                IDR_CMD_CONTEXT_PLOT_WITH_FORMULA
                );
        }

        return LRESULT_DEFAULT;
    }
#pragma endregion

#pragma endregion

#pragma region Callback Function Implementations

    void _updateCursorHitTest()
    {
        // hit test cursor on each formula
        int closestFormulaIdx = -1;
        double closestDistance = (std::numeric_limits<double>::max)();

        for (int i = currentCursorX - HITTEST_RADIUS / 4, imax = currentCursorX + HITTEST_RADIUS / 4; i < imax; ++i)
        {
            double formulaX = drawer->translateCanvasX(i);
            double initial = Util::pow2(formulaX - currentCursorFormulaX);

            for (size_t j = 0, jmax = options->graphics.size(); j < jmax; ++j)
            {
                auto g = options->graphics[j];
                if (g->type == GraphicType::FORMULA_EXPRESSION)
                {
                    auto exp = std::dynamic_pointer_cast<ExpDrawer>(g);
                    double y = exp->expression->eval(formulaX);
                    double dis = initial + Util::pow2(y - currentCursorFormulaY);
                    if (dis < closestDistance)
                    {
                        closestDistance = dis;
                        closestFormulaIdx = j;
                    }
                }
            }
        }

        double expect = drawer->translateCanvasW(HITTEST_RADIUS) * drawer->translateCanvasH(HITTEST_RADIUS);
        int result = closestDistance < expect ? closestFormulaIdx : -1;

        if (result != drawer->hoverExpIdx)
        {
            drawer->hoverExpIdx = result;
            bmpCanvas->dispatchRedraw();
            saveAndApplyCursor((result == -1 ? hCursorCross : hCursorHand));
        }
    }

    void _syncRangeFromOption()
    {
        txtRangeXFrom->setText(Util::to_string_with_precision(options->vpLeft, 5));
        txtRangeXTo->setText(Util::to_string_with_precision(options->vpRight, 5));
        txtRangeYFrom->setText(Util::to_string_with_precision(options->vpBottom, 5));
        txtRangeYTo->setText(Util::to_string_with_precision(options->vpTop, 5));
    }

    void _syncGridFromOption()
    {
        chkShowGrid->setChecked(options->showGrid);
        txtGridSize->setText(std::to_wstring(options->gridSpacing));
    }

    void _syncAxisFromOption()
    {
        chkShowAxis->setChecked(options->showAxis);
        txtAxisSize->setText(std::to_wstring(options->axisTickInterval));
    }

    void _syncCursorPosition()
    {
        // cursor
        lblInfoCursorXValue->setText(Util::to_string_with_precision(currentCursorFormulaX, 8));
        lblInfoCursorYValue->setText(Util::to_string_with_precision(currentCursorFormulaY, 8));

        // hot track
        if (options->enableHotTrack)
        {
            lblInfoFormulaXValue->setText(Util::to_string_with_precision(currentHTX, 8));
            lblInfoFormulaYValue->setText(Util::to_string_with_precision(currentHTY, 8));
        }
        else
        {
            lblInfoFormulaXValue->setText(L"--");
            lblInfoFormulaYValue->setText(L"--");
        }
    }

#pragma endregion

    void setup()
    {
        mathConstants[L"PI"] = std::atan(1) * 4;

        animation = AnimationPtr(new Animation(
            hWnd, IDT_TIMER_PLOT,
            Easing::cubicOut,
            bmpCanvas_onScaleAnimationProgress,
            bmpCanvas_onScaleAnimationComplete,
            300
            ));

        options = PlotOptionsPtr(new OptionBag());
        options->calculateOuterBoundaryInCenter(CANVAS_ENLARGE);

        syncRangeFromOption();
        syncGridFromOption();
        syncAxisFromOption();

        drawer = PlotDrawerPtr(new Drawer(options, bmpCanvas->memDC, chkAntialias->isChecked()));
        updateDrawerRange();
        drawer->rebuildBuffer(bmpCanvas->canvasW, bmpCanvas->canvasH);

        chkAntialias->addEventHandler(EventName::EVENT_CLICK, chkAntialias_onClick);
        chkShowGrid->addEventHandler(EventName::EVENT_CLICK, chkShowGrid_onClick);
        chkShowAxis->addEventHandler(EventName::EVENT_CLICK, chkShowAxis_onClick);
        txtGridSize->addEventHandler(EventName::EVENT_LOSING_FOCUS, txtGridSize_onLosingFocus);
        txtAxisSize->addEventHandler(EventName::EVENT_LOSING_FOCUS, txtAxisSize_onLosingFocus);
        txtRangeXFrom->addEventHandler(EventName::EVENT_LOSING_FOCUS, txtRangeXFrom_onLosingFocus);
        txtRangeXTo->addEventHandler(EventName::EVENT_LOSING_FOCUS, txtRangeXTo_onLosingFocus);
        txtRangeYFrom->addEventHandler(EventName::EVENT_LOSING_FOCUS, txtRangeYFrom_onLosingFocus);
        txtRangeYTo->addEventHandler(EventName::EVENT_LOSING_FOCUS, txtRangeYTo_onLosingFocus);

        // Init list view with image list
        HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR, 1, 0);
        ListView_SetImageList(lstFormulas->hControl, hImageList, LVSIL_SMALL);
        lstFormulas->addEventHandler(EventName::EVENT_LISTVIEW_ENDEDIT, lstFormulas_onEndEdit);
        lstFormulas->addEventHandler(EventName::EVENT_CUSTOMDRAW, lstFormulas_onCustomDraw);
        lstFormulas->addEventHandler(EventName::EVENT_NOTIFY, lstFormulas_onNotify);
        lstFormulas->insertColumn(L"Expression", 180);

        btnAdd->addEventHandler(EventName::EVENT_CLICK, btnAdd_onClick);
        btnRemove->addEventHandler(EventName::EVENT_CLICK, btnRemove_onClick);

        // Ribbon menu item
        rcmdFormulaColor->addEventHandler(EventName::EVENT_RIBBON_UPDATE_PROPERTY, rcmdFormulaColor_onUpdateProperty);
        rcmdFormulaColor->addEventHandler(EventName::EVENT_RIBBON_EXECUTE, rcmdFormulaColor_onExecute);
        rcmdBgColor->addEventHandler(EventName::EVENT_RIBBON_UPDATE_PROPERTY, rcmdBgColor_onUpdateProperty);
        rcmdBgColor->addEventHandler(EventName::EVENT_RIBBON_EXECUTE, rcmdBgColor_onExecute);
        rcmdShowGrid->addEventHandler(EventName::EVENT_RIBBON_UPDATE_PROPERTY, rcmdShowGrid_onUpdateProperty);
        rcmdShowAxis->addEventHandler(EventName::EVENT_RIBBON_UPDATE_PROPERTY, rcmdShowAxis_onUpdateProperty);
        rcmdShowGrid->addEventHandler(EventName::EVENT_RIBBON_EXECUTE, rcmdShowGrid_onExecute);
        rcmdShowAxis->addEventHandler(EventName::EVENT_RIBBON_EXECUTE, rcmdShowAxis_onExecute);
        rcmdEditFormula->addEventHandler(EventName::EVENT_RIBBON_EXECUTE, rcmdEditFormula_onExecute);
        rcmdRemoveFormula->addEventHandler(EventName::EVENT_RIBBON_EXECUTE, rcmdRemoveFormula_onExecute);
        rcmdSave->addEventHandler(EventName::EVENT_RIBBON_EXECUTE, rcmdSave_onExecute);

        SetClassLongW(bmpCanvas->hControl, GCL_HCURSOR, NULL);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_REBUILD, bmpCanvas_onCanvasRebuild);
        bmpCanvas->addEventHandler(EventName::EVENT_BUFFER_REDRAW, bmpCanvas_onRedrawBuffer);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_BEGINMOVE, bmpCanvas_onCanvasBeginMove);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_MOVE, bmpCanvas_onCanvasMove);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_ENDMOVE, bmpCanvas_onCanvasEndMove);
        bmpCanvas->addEventHandler(EventName::EVENT_MOUSEWHEEL, bmpCanvas_onMouseWheel);
        bmpCanvas->addEventHandler(EventName::EVENT_MOUSEMOVE, bmpCanvas_onMouseMove);
        bmpCanvas->addEventHandler(EventName::EVENT_SETCURSOR, bmpCanvas_onSetCursor);
        bmpCanvas->addEventHandler(EventName::EVENT_MOUSEDOWN, bmpCanvas_onMouseDown);
        bmpCanvas->addEventHandler(EventName::EVENT_MOUSEUP, bmpCanvas_onMouseUp);
        bmpCanvas->dispatchRedraw();
    }
}