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

    ThrottlerPtr            throttler;

    ContainerPtr            container;
    LayoutManagerPtr        lm;
    EventManagerPtr         em;

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

    CallbackFunction        syncRangeFromOption;
    CallbackFunction        syncGridFromOption;

    PlotOptionsPtr          options;
    PlotDrawerPtr           drawer;
    double                  initialLeft, initialRight, initialTop, initialBottom;

    AnimationPtr            animation;

    double                  scaleFactor = 1.0;
    int                     scrollValue = 0;
    int                     completedScrollValue = 0;
    double                  scaleOriginX, scaleOriginY;

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

    void App::init(HWND _hWnd)
    {
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        hWnd = _hWnd;

        throttler = ThrottlerPtr(new Throttler(hWnd));
        syncRangeFromOption = throttler->applyThrottle(300, CallbackFunction(_syncRangeFromOption));
        syncGridFromOption = throttler->applyThrottle(300, CallbackFunction(_syncGridFromOption));

        container = ContainerPtr(new Container());
        lm = LayoutManagerPtr(new LayoutManager(hWnd));
        em = EventManagerPtr(new EventManager(container));

        bmpCanvas = new BufferedCanvas(hWnd, IDC_STATIC_CANVAS, CANVAS_ENLARGE);
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

    inline void chkShowGrid_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        txtGridSize->setEnabled(chkShowGrid->isChecked());
        options->showGrid = chkShowGrid->isChecked();
        bmpCanvas->forceRedraw();
    }

    inline void txtGridSize_onLosingFocus(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        options->gridSpacing = std::stoi(txtGridSize->getText());
        bmpCanvas->forceRedraw();
    }

    inline void chkShowAxis_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        txtAxisSize->setEnabled(chkShowAxis->isChecked());
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
        bmpCanvas->forceRedraw();
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
        bmpCanvas->forceRedraw();
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
        bmpCanvas->forceRedraw();
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
        bmpCanvas->forceRedraw();
    }

    inline void chkAntialias_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        drawer->setAntialias(chkAntialias->isChecked());
        bmpCanvas->forceRedraw();
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
        drawer->setSize(bmpCanvas->canvasW, bmpCanvas->canvasH, bmpCanvas->vpX, bmpCanvas->vpY, bmpCanvas->width, bmpCanvas->height);
        drawer->draw();
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
        bmpCanvas->forceRedraw();
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
        bmpCanvas->forceRedraw();
        bmpCanvas->forceCopyBuffer();
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

        options->formulaColors.push_back(Color(255, 47, 197, 255));
        options->formulaObjects.push_back(ExpDrawerPtr(new ExpDrawer(L"sin(x)", mathConstants)));
        //options->formulaObjects.push_back(ExpDrawerPtr(new ExpDrawer(L"x+1", mathConstants)));

        syncRangeFromOption();
        syncGridFromOption();

        drawer = PlotDrawerPtr(new Drawer(options, bmpCanvas->memDC, chkAntialias->isChecked()));
        drawer->setSize(bmpCanvas->canvasW, bmpCanvas->canvasH, bmpCanvas->vpX, bmpCanvas->vpY, bmpCanvas->width, bmpCanvas->height);

        chkAntialias->addEventHandler(EventName::EVENT_CLICK, chkAntialias_onClick);
        chkShowGrid->addEventHandler(EventName::EVENT_CLICK, chkShowGrid_onClick);
        chkShowAxis->addEventHandler(EventName::EVENT_CLICK, chkShowAxis_onClick);
        txtGridSize->addEventHandler(EventName::EVENT_LOSING_FOCUS, txtGridSize_onLosingFocus);
        txtRangeXFrom->addEventHandler(EventName::EVENT_LOSING_FOCUS, txtRangeXFrom_onLosingFocus);
        txtRangeXTo->addEventHandler(EventName::EVENT_LOSING_FOCUS, txtRangeXTo_onLosingFocus);
        txtRangeYFrom->addEventHandler(EventName::EVENT_LOSING_FOCUS, txtRangeYFrom_onLosingFocus);
        txtRangeYTo->addEventHandler(EventName::EVENT_LOSING_FOCUS, txtRangeYTo_onLosingFocus);

        bmpCanvas->addEventHandler(EventName::EVENT_BUFFER_REDRAW, bmpCanvas_onRedrawBuffer);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_BEGINMOVE, bmpCanvas_onCanvasBeginMove);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_MOVE, bmpCanvas_onCanvasMove);
        bmpCanvas->addEventHandler(EventName::EVENT_MOUSEWHEEL, bmpCanvas_onMouseWheel);
        bmpCanvas->forceRedraw();
    }
}