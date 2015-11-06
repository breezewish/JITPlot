#include <map>
#include <string>
#include <cmath>

#include <app.h>
#include <resource.h>
#include <throttler.h>
#include <ui/controls/control.h>
#include <ui/controls/checkbox.h>
#include <ui/controls/textbox.h>
#include <ui/controls/bufferedcanvas.h>
#include <ui/container.h>
#include <ui/layout/layoutmanager.h>
#include <ui/events/event.h>
#include <ui/events/canvasrebuildevent.h>
#include <ui/events/eventmanager.h>
#include <plot/drawer.h>
#include <plot/optionbag.h>
#include <plot/animation.h>
#include <formula/exp.h>
#include <formula/expdrawer.h>

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

    const double CANVAS_ENLARGE = 2.0;
    const double SCROLL_FACTOR = 240.0;

    map<wstring, double>    mathConstants;

    GdiplusStartupInput     gdiplusStartupInput;
    ULONG_PTR               gdiplusToken;

    HWND                    hWnd;

    // ======== Managers ========
    ThrottlerPtr            throttler;
    ContainerPtr            container;
    LayoutManagerPtr        lm;
    EventManagerPtr         em;

    // ======== Controls ========
    Control                 * grpInfo;
    Control                 * lblInfoCursor;
    Control                 * lblInfoCursorX;
    Control                 * lblInfoCursorY;
    Control                 * lblInfoFormula;
    Control                 * lblInfoFormulaX;
    Control                 * lblInfoFormulaY;
    Textbox                 * lblInfoCursorXValue;
    Textbox                 * lblInfoCursorYValue;
    Textbox                 * lblInfoFormulaXValue;
    Textbox                 * lblInfoFormulaYValue;
    Control                 * grpCanvas;
    Checkbox                * chkAntialias;
    Checkbox                * chkShowGrid;
    Checkbox                * chkShowAxis;
    Control                 * lblGridInterval;
    Control                 * lblAxisInterval;
    Textbox                 * txtGridSize;
    Textbox                 * txtAxisSize;
    BufferedCanvas          * bmpCanvas;
    Control                 * lblRange;
    Control                 * lblRangeXFrom;
    Control                 * lblRangeXTo;
    Control                 * lblRangeYFrom;
    Control                 * lblRangeYTo;
    Textbox                 * txtRangeXFrom;
    Textbox                 * txtRangeXTo;
    Textbox                 * txtRangeYFrom;
    Textbox                 * txtRangeYTo;

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

    // ======== Cursor Position ========
    int                     currentCursorX, currentCursorY;

    // ======== Formula Hot Track Position ========
    double                  currentHTX, currentHTY;

    void setup();

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
        double x = drawer->translateCanvasX(currentCursorX);
        double y = drawer->translateCanvasY(currentCursorY);
        lblInfoCursorXValue->setText(Util::to_string_with_precision(x, 8));
        lblInfoCursorYValue->setText(Util::to_string_with_precision(y, 8));

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

    void App::init(HWND _hWnd)
    {
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        hWnd = _hWnd;

        throttler = ThrottlerPtr(new Throttler(hWnd));
        syncRangeFromOption = throttler->applyThrottle(300, CallbackFunction(_syncRangeFromOption));
        syncGridFromOption = CallbackFunction(_syncGridFromOption);
        syncAxisFromOption = CallbackFunction(_syncAxisFromOption);
        syncCursorPosition = throttler->applyThrottle(30, CallbackFunction(_syncCursorPosition));

        container = ContainerPtr(new Container());
        lm = LayoutManagerPtr(new LayoutManager(hWnd));
        em = EventManagerPtr(new EventManager(container));

        bmpCanvas = new BufferedCanvas(hWnd, IDC_STATIC_CANVAS, CANVAS_ENLARGE);
        grpInfo = new Control(hWnd, IDC_STATIC_GROUP_INFO);
        lblInfoCursor = new Control(hWnd, IDC_STATIC_INFO_CURSOR);
        lblInfoCursorX = new Control(hWnd, IDC_STATIC_INFO_CURSOR_X);
        lblInfoCursorY = new Control(hWnd, IDC_STATIC_INFO_CURSOR_Y);
        lblInfoFormula = new Control(hWnd, IDC_STATIC_INFO_FORMULA);
        lblInfoFormulaX = new Control(hWnd, IDC_STATIC_INFO_FORMULA_X);
        lblInfoFormulaY = new Control(hWnd, IDC_STATIC_INFO_FORMULA_Y);
        lblInfoCursorXValue = new Textbox(hWnd, IDC_STATIC_INFO_CURSOR_X_VALUE);
        lblInfoCursorYValue = new Textbox(hWnd, IDC_STATIC_INFO_CURSOR_Y_VALUE);
        lblInfoFormulaXValue = new Textbox(hWnd, IDC_STATIC_INFO_FORMULA_X_VALUE);
        lblInfoFormulaYValue = new Textbox(hWnd, IDC_STATIC_INFO_FORMULA_Y_VALUE);
        grpCanvas = new Control(hWnd, IDC_STATIC_GROUP_CANVAS);
        chkAntialias = new Checkbox(hWnd, IDC_CHECK_ANTIALIAS);
        chkShowGrid = new Checkbox(hWnd, IDC_CHECK_SHOW_GRID);
        chkShowAxis = new Checkbox(hWnd, IDC_CHECK_SHOW_AXIS);
        txtGridSize = new Textbox(hWnd, IDC_EDIT_GRID_SIZE);
        txtAxisSize = new Textbox(hWnd, IDC_EDIT_AXIS_SIZE);
        lblGridInterval = new Control(hWnd, IDC_STATIC_GRID_INTERVAL);
        lblAxisInterval = new Control(hWnd, IDC_STATIC_AXIS_INTERVAL);
        lblRange = new Control(hWnd, IDC_STATIC_RANGE);
        lblRangeXFrom = new Control(hWnd, IDC_STATIC_RANGE_X_FROM);
        lblRangeXTo = new Control(hWnd, IDC_STATIC_RANGE_X_TO);
        lblRangeYFrom = new Control(hWnd, IDC_STATIC_RANGE_Y_FROM);
        lblRangeYTo = new Control(hWnd, IDC_STATIC_RANGE_Y_TO);
        txtRangeXFrom = new Textbox(hWnd, IDC_EDIT_X_FROM);
        txtRangeXTo = new Textbox(hWnd, IDC_EDIT_X_TO);
        txtRangeYFrom = new Textbox(hWnd, IDC_EDIT_Y_FROM);
        txtRangeYTo = new Textbox(hWnd, IDC_EDIT_Y_TO);

        chkAntialias->setChecked(true);
        chkShowGrid->setChecked(true);
        chkShowAxis->setChecked(true);

        container
            ->addControl(bmpCanvas)
            ->addControl(grpInfo)
            ->addControl(lblInfoCursor)
            ->addControl(lblInfoCursorX)
            ->addControl(lblInfoCursorY)
            ->addControl(lblInfoFormula)
            ->addControl(lblInfoFormulaX)
            ->addControl(lblInfoFormulaY)
            ->addControl(lblInfoCursorXValue)
            ->addControl(lblInfoCursorYValue)
            ->addControl(lblInfoFormulaXValue)
            ->addControl(lblInfoFormulaYValue)
            ->addControl(grpCanvas)
            ->addControl(chkAntialias)
            ->addControl(chkShowGrid)
            ->addControl(chkShowAxis)
            ->addControl(txtGridSize)
            ->addControl(txtAxisSize)
            ->addControl(lblGridInterval)
            ->addControl(lblAxisInterval)
            ->addControl(lblRange)
            ->addControl(lblRangeXFrom)
            ->addControl(lblRangeXTo)
            ->addControl(lblRangeYFrom)
            ->addControl(lblRangeYTo)
            ->addControl(txtRangeXFrom)
            ->addControl(txtRangeXTo)
            ->addControl(txtRangeYFrom)
            ->addControl(txtRangeYTo);

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

        return em->handle(uMsg, wParam, lParam);
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

    // ========== Event handlers =============

    inline void chkShowGrid_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        txtGridSize->setEnabled(chkShowGrid->isChecked());
        options->showGrid = chkShowGrid->isChecked();
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
        Debug::Debug() << options->vpLeft >> Debug::writeln;
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

        vpTakeShapshot();
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
        currentHTX = drawer->translateCanvasX(currentCursorX);
        currentHTY = options->expressions[options->activeExpIdx]->expression->eval(currentHTX);
        drawer->htCanvasY = static_cast<int>(drawer->translateFormulaY(currentHTY));
    }

    inline void bmpCanvas_onMouseMove(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

        // current cursor position
        currentCursorX = bmpCanvas->vpX + event->x;
        currentCursorY = bmpCanvas->vpY + event->y;

        // current plot position
        if (options->enableHotTrack
            && options->activeExpIdx >= 0
            && options->activeExpIdx < static_cast<int>(options->expressions.size())
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
            && options->activeExpIdx >= 0
            && options->activeExpIdx < static_cast<int>(options->expressions.size())
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

        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_REBUILD, bmpCanvas_onCanvasRebuild);
        bmpCanvas->addEventHandler(EventName::EVENT_BUFFER_REDRAW, bmpCanvas_onRedrawBuffer);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_BEGINMOVE, bmpCanvas_onCanvasBeginMove);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_MOVE, bmpCanvas_onCanvasMove);
        bmpCanvas->addEventHandler(EventName::EVENT_MOUSEWHEEL, bmpCanvas_onMouseWheel);
        bmpCanvas->addEventHandler(EventName::EVENT_MOUSEMOVE, bmpCanvas_onMouseMove);
        bmpCanvas->dispatchRedraw();
    }
}