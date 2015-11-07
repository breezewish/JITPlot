#pragma region Include
#include <map>
#include <string>
#include <cmath>
#include <limits>

#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>

#include <app.h>
#include <RibbonIDs.h>
#include <resource.h>
#include <throttler.h>
#include <ui/controls/control.h>
#include <ui/controls/win32control.h>
#include <ui/controls/ribboncontrol.h>
#include <ui/controls/checkbox.h>
#include <ui/controls/textbox.h>
#include <ui/controls/bufferedcanvas.h>
#include <ui/container.h>
#include <ui/layout/layoutmanager.h>
#include <ui/events/event.h>
#include <ui/events/canvasrebuildevent.h>
#include <ui/events/eventmanager.h>
#include <ui/events/ribbonupdatepropertyevent.h>
#include <ui/events/ribbonexecuteevent.h>
#include <plot/drawer.h>
#include <plot/optionbag.h>
#include <plot/animation.h>
#include <formula/exp.h>
#include <formula/expdrawer.h>
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

#pragma region Constants

    const double CANVAS_ENLARGE = 2.0;
    const double SCROLL_FACTOR = 240.0;

    const int HITTEST_RADIUS = 20;
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

    // ======== Ribbon Controls ========
    RibbonControl           * rcmdFormulaColor;
    RibbonControl           * rcmdShowGrid;
    RibbonControl           * rcmdShowAxis;
    RibbonControl           * rcmdSave;

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

        rcmdFormulaColor = new RibbonControl(IDR_CMD_FORMULA_COLORPICKER);
        rcmdShowGrid = new RibbonControl(IDR_CMD_SHOW_GRID);
        rcmdShowAxis = new RibbonControl(IDR_CMD_SHOW_AXIS);
        rcmdSave = new RibbonControl(IDR_CMD_SAVE);

        chkAntialias->setChecked(true);
        chkShowGrid->setChecked(true);
        chkShowAxis->setChecked(true);

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
            ->addRibbonControl(rcmdFormulaColor)
            ->addRibbonControl(rcmdShowGrid)
            ->addRibbonControl(rcmdShowAxis)
            ->addRibbonControl(rcmdSave);

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
            ->enableMagnet(txtRangeYTo, false, true, true, false);

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

        delete rcmdFormulaColor;
        delete rcmdShowGrid;
        delete rcmdShowAxis;
        delete rcmdSave;

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
        if (uMsg == WM_TIMER)
        {
            if (wParam == IDT_TIMER_PLOT)
            {
                animation->update();
                return false;
            }
            else if (throttler->canHandleTick(wParam))
            {
                throttler->handleTick(wParam);
                return false;
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

#pragma endregion

#pragma region Event Handlers

#pragma region Ribbon Handlers

    inline void rcmdFormulaColor_onUpdateProperty(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<RibbonUpdatePropertyEvent>(_event);

        if (event->key == UI_PKEY_ColorType)
        {
            UIInitPropertyFromUInt32(event->key, UI_SWATCHCOLORTYPE_RGB, event->newValue);
        }
        else if (event->key == UI_PKEY_Color)
        {
            if (cmFormulaIdx >= 0 && cmFormulaIdx < static_cast<int>(options->expressions.size()))
            {
                UIInitPropertyFromUInt32(
                    event->key,
                    options->expressions[cmFormulaIdx]->color.ToCOLORREF(),
                    event->newValue
                    );
            }
        }
    }

    inline void rcmdFormulaColor_onExecute(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<RibbonExecuteEvent>(_event);

        if (event->verb != UI_EXECUTIONVERB_EXECUTE) return;

        if (cmFormulaIdx >= 0 && cmFormulaIdx < static_cast<int>(options->expressions.size()))
        {
            UINT color = 0;
            PROPVARIANT var;
            event->properties->GetValue(UI_PKEY_Color, &var);
            UIPropertyToUInt32(UI_PKEY_Color, var, &color);
            options->expressions[cmFormulaIdx]->color.SetFromCOLORREF(color);
            bmpCanvas->dispatchRedraw();
        }
    }

    inline void rcmdShowGrid_onExecute(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<RibbonExecuteEvent>(_event);

        if (event->verb != UI_EXECUTIONVERB_EXECUTE) return;

        PROPVARIANT var;
        BOOL val;

        event->properties->GetValue(UI_PKEY_BooleanValue, &var);
        PropVariantToBoolean(var, &val);

        options->showGrid = (val == TRUE);
        syncGridFromOption();
        bmpCanvas->dispatchRedraw();
    }

    inline void rcmdShowAxis_onExecute(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<RibbonExecuteEvent>(_event);

        if (event->verb != UI_EXECUTIONVERB_EXECUTE) return;

        PROPVARIANT var;
        BOOL val;

        event->properties->GetValue(UI_PKEY_BooleanValue, &var);
        PropVariantToBoolean(var, &val);

        options->showAxis = (val == TRUE);
        syncAxisFromOption();
        bmpCanvas->dispatchRedraw();
    }

    inline void rcmdShowGrid_onUpdateProperty(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<RibbonUpdatePropertyEvent>(_event);

        if (event->key == UI_PKEY_BooleanValue)
        {
            UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, options->showGrid, event->newValue);
        }
    }

    inline void rcmdShowAxis_onUpdateProperty(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<RibbonUpdatePropertyEvent>(_event);

        if (event->key == UI_PKEY_BooleanValue)
        {
            UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, options->showAxis, event->newValue);
        }
    }
#pragma endregion

#pragma region ControlPad Handlers
    inline void chkShowGrid_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        txtGridSize->setEnabled(chkShowGrid->isChecked());
        options->showGrid = chkShowGrid->isChecked();
        Ribbon::g_pFramework->InvalidateUICommand(IDR_CMD_SHOW_GRID, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
        bmpCanvas->dispatchRedraw();
    }

    inline void txtGridSize_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        options->gridSpacing = std::stoi(txtGridSize->getText());
        bmpCanvas->dispatchRedraw();
    }

    inline void chkShowAxis_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        txtAxisSize->setEnabled(chkShowAxis->isChecked());
        options->showAxis = chkShowAxis->isChecked();
        Ribbon::g_pFramework->InvalidateUICommand(IDR_CMD_SHOW_AXIS, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
        bmpCanvas->dispatchRedraw();
    }

    inline void txtAxisSize_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        options->axisTickInterval = std::stoi(txtAxisSize->getText());
        bmpCanvas->dispatchRedraw();
    }

    inline void txtRangeXFrom_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        double val = std::stod(txtRangeXFrom->getText());
        if (val >= options->vpRight)
        {
            _syncRangeFromOption();
            return;
        }
        options->vpLeft = val;
        options->calculateOuterBoundaryInCenter(CANVAS_ENLARGE);
        _syncRangeFromOption();
        bmpCanvas->dispatchRedraw();
    }

    inline void txtRangeXTo_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        double val = std::stod(txtRangeXTo->getText());
        if (val <= options->vpLeft)
        {
            _syncRangeFromOption();
            return;
        }
        options->vpRight = val;
        options->calculateOuterBoundaryInCenter(CANVAS_ENLARGE);
        _syncRangeFromOption();
        bmpCanvas->dispatchRedraw();
    }

    inline void txtRangeYFrom_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        double val = std::stod(txtRangeYFrom->getText());
        if (val >= options->vpTop)
        {
            _syncRangeFromOption();
            return;
        }
        options->vpBottom = val;
        options->calculateOuterBoundaryInCenter(CANVAS_ENLARGE);
        _syncRangeFromOption();
        bmpCanvas->dispatchRedraw();
    }

    inline void txtRangeYTo_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        double val = std::stod(txtRangeYTo->getText());
        if (val <= options->vpBottom)
        {
            _syncRangeFromOption();
            return;
        }
        options->vpTop = val;
        options->calculateOuterBoundaryInCenter(CANVAS_ENLARGE);
        _syncRangeFromOption();
        bmpCanvas->dispatchRedraw();
    }

    inline void chkAntialias_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        drawer->setAntialias(chkAntialias->isChecked());
        bmpCanvas->dispatchRedraw();
    }
#pragma endregion

#pragma region Canvas handlers
    inline void bmpCanvas_onRedrawBuffer(Control * _control, const EventPtr & _event)
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
    }

    inline void bmpCanvas_onCanvasBeginMove(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        saveAndApplyCursor(hCursorDefault);
        vpTakeShapshot();
    }

    inline void bmpCanvas_onCanvasEndMove(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        restoreCursor();
    }

    inline void bmpCanvas_onCanvasMove(Control * _control, const EventPtr & _event)
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
    }

    inline void updateHotTrackPosition()
    {
        drawer->htCanvasX = currentCursorX;
        currentHTX = currentCursorFormulaX;
        currentHTY = options->expressions[drawer->activeExpIdx]->expression->eval(currentHTX);
        drawer->htCanvasY = static_cast<int>(drawer->translateFormulaY(currentHTY));
    }

    inline void bmpCanvas_onMouseMove(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

        // current cursor position
        currentCursorX = bmpCanvas->vpX + event->x;
        currentCursorY = bmpCanvas->vpY + event->y;
        currentCursorFormulaX = drawer->translateCanvasX(currentCursorX);
        currentCursorFormulaY = drawer->translateCanvasY(currentCursorY);
        updateCursorHitTest();

        // current plot position
        if (options->enableHotTrack
            && drawer->activeExpIdx >= 0
            && drawer->activeExpIdx < static_cast<int>(options->expressions.size())
            )
        {
            updateHotTrackPosition();

            drawer->refresh();
            bmpCanvas->refresh();
        }

        syncCursorPosition();
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
            && drawer->activeExpIdx < static_cast<int>(options->expressions.size())
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

    inline void bmpCanvas_onMouseWheel(Control * _control, const EventPtr & _event)
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
    }

    inline void bmpCanvas_onCanvasRebuild(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<CanvasRebuildEvent>(_event);

        drawer->rebuildBuffer(event->canvasWidth, event->canvasHeight);
    }

    inline void bmpCanvas_onSetCursor(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        SetCursor(hCurrentCursor);
    }

    inline void bmpCanvas_onMouseDown(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

        if (event->button == MouseButton::RIGHT)
        {
            POINT pt;
            pt.x = event->x;
            pt.y = event->y;
            ClientToScreen(hWnd, &pt);

            UINT32 rh;
            Ribbon::g_pRibbon->GetHeight(&rh);

            cmFormulaIdx = drawer->hoverExpIdx;

            // invalidate color
            Ribbon::g_pFramework->InvalidateUICommand(IDR_CMD_FORMULA_COLORPICKER, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_Color);

            IUIContextualUI * pContextualUI = NULL;
            Ribbon::g_pFramework->GetView(cmFormulaIdx == -1 ? IDR_CMD_CONTEXT_PLOT : IDR_CMD_CONTEXT_PLOT_WITH_FORMULA, IID_PPV_ARGS(&pContextualUI));
            pContextualUI->ShowAtLocation(pt.x, pt.y + rh);
            pContextualUI->Release();
        }
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

            for (size_t j = 0, jmax = options->expressions.size(); j < jmax; ++j)
            {
                double y = options->expressions[j]->expression->eval(formulaX);
                double dis = initial + Util::pow2(y - currentCursorFormulaY);
                if (dis < closestDistance)
                {
                    closestDistance = dis;
                    closestFormulaIdx = j;
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

        options->expressions.push_back(ExpDrawerPtr(new ExpDrawer(L"sin(x)", mathConstants, Color(255, 47, 197, 255))));
        options->expressions.push_back(ExpDrawerPtr(new ExpDrawer(L"cos(x)", mathConstants, Color(255, 243, 104, 104))));
        //options->formulaObjects.push_back(ExpDrawerPtr(new ExpDrawer(L"x+1", mathConstants)));

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

        rcmdFormulaColor->addEventHandler(EventName::EVENT_RIBBON_UPDATE_PROPERTY, rcmdFormulaColor_onUpdateProperty);
        rcmdFormulaColor->addEventHandler(EventName::EVENT_RIBBON_EXECUTE, rcmdFormulaColor_onExecute);
        rcmdShowGrid->addEventHandler(EventName::EVENT_RIBBON_UPDATE_PROPERTY, rcmdShowGrid_onUpdateProperty);
        rcmdShowAxis->addEventHandler(EventName::EVENT_RIBBON_UPDATE_PROPERTY, rcmdShowAxis_onUpdateProperty);
        rcmdShowGrid->addEventHandler(EventName::EVENT_RIBBON_EXECUTE, rcmdShowGrid_onExecute);
        rcmdShowAxis->addEventHandler(EventName::EVENT_RIBBON_EXECUTE, rcmdShowAxis_onExecute);

        SetClassLong(bmpCanvas->hWnd, GCL_HCURSOR, NULL);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_REBUILD, bmpCanvas_onCanvasRebuild);
        bmpCanvas->addEventHandler(EventName::EVENT_BUFFER_REDRAW, bmpCanvas_onRedrawBuffer);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_BEGINMOVE, bmpCanvas_onCanvasBeginMove);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_MOVE, bmpCanvas_onCanvasMove);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_ENDMOVE, bmpCanvas_onCanvasEndMove);
        bmpCanvas->addEventHandler(EventName::EVENT_MOUSEWHEEL, bmpCanvas_onMouseWheel);
        bmpCanvas->addEventHandler(EventName::EVENT_MOUSEMOVE, bmpCanvas_onMouseMove);
        bmpCanvas->addEventHandler(EventName::EVENT_SETCURSOR, bmpCanvas_onSetCursor);
        bmpCanvas->addEventHandler(EventName::EVENT_MOUSEDOWN, bmpCanvas_onMouseDown);
        bmpCanvas->dispatchRedraw();
    }
}