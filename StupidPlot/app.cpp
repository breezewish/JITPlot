#include <map>
#include <string>
#include <cmath>

#include <app.h>
#include <resource.h>
#include <ui/controls/control.h>
#include <ui/controls/checkbox.h>
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

    ContainerPtr            container;
    LayoutManagerPtr        lm;
    EventManagerPtr         em;

    Control                 * groupCanvas;
    Checkbox                * checkShowGrid;
    Control                 * editGridSize;
    BufferedCanvas          * canvas;

    PlotOptionsPtr          options;
    PlotDrawerPtr           drawer;
    double                  initialLeft, initialRight, initialTop, initialBottom;

    AnimationPtr            animation;

    double                  scaleFactor = 1.0;
    int                     scrollValue = 0;
    int                     completedScrollValue = 0;
    double                  scaleOriginX, scaleOriginY;

    void setup();

    void App::init(HWND _hWnd)
    {
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        hWnd = _hWnd;

        container = ContainerPtr(new Container());
        lm = LayoutManagerPtr(new LayoutManager(hWnd));
        em = EventManagerPtr(new EventManager(container));

        canvas = new BufferedCanvas(hWnd, IDC_STATIC_CANVAS, CANVAS_ENLARGE);
        groupCanvas = new Control(hWnd, IDC_STATIC_GROUP_CANVAS);
        checkShowGrid = new Checkbox(hWnd, IDC_CHECK_SHOW_GRID);
        editGridSize = new Control(hWnd, IDC_EDIT_GRID_SIZE);

        container
            ->addControl(canvas)
            ->addControl(groupCanvas)
            ->addControl(checkShowGrid)
            ->addControl(editGridSize);

        lm
            ->enableMagnet(canvas, true, true, true, true)
            ->enableMagnet(groupCanvas, false, true, true, false)
            ->enableMagnet(checkShowGrid, false, true, true, false)
            ->enableMagnet(editGridSize, false, true, true, false);

        updateSize();
        setup();
    }

    void App::terminate()
    {
        if (editGridSize) delete editGridSize;
        if (checkShowGrid) delete checkShowGrid;
        if (groupCanvas) delete groupCanvas;
        if (canvas) delete canvas;

        GdiplusShutdown(gdiplusToken);
    }

    void App::updateSize()
    {
        lm->updateSize();
        lm->relayout();
    }

    BOOL App::handleEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (uMsg == WM_TIMER && wParam == IDT_TIMER_PLOT)
        {
            animation->update();
            return false;
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
    }

    inline void CheckShowGrid_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        editGridSize->setEnabled(checkShowGrid->isChecked());
    }

    inline void PlotCanvas_onRedrawBuffer(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);
        // We need to make sure that vp x <-> formula left
        double dx = drawer->translateCanvasW(canvas->vpX);
        double dy = drawer->translateCanvasH(canvas->vpY);
        double fcw = drawer->translateCanvasW(canvas->canvasW);
        double fch = drawer->translateCanvasH(canvas->canvasH);
        double fvw = drawer->translateCanvasW(canvas->width);
        double fvh = drawer->translateCanvasH(canvas->height);
        options->vpRight = options->vpLeft + fvw;
        options->vpTop = options->vpBottom + fvh;
        options->drawLeft = options->vpLeft - dx;
        options->drawBottom = options->vpBottom - dy;
        options->drawRight = options->drawLeft + fcw;
        options->drawTop = options->drawBottom + fch;
        drawer->setSize(canvas->canvasW, canvas->canvasH, canvas->vpX, canvas->vpY, canvas->width, canvas->height);
        drawer->draw();
    }

    inline void PlotCanvas_onCanvasBeginMove(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);
        vpTakeShapshot();
    }

    inline void PlotCanvas_onCanvasMove(Control * _control, const EventPtr & _event)
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
    }

    void scaleReset()
    {
        scrollValue -= completedScrollValue;
        scaleFactor = 1.0;
        completedScrollValue = 0;
        vpTakeShapshot();
    }

    inline void PlotCanvas_onScaleBegin(const shared_ptr<MouseWheelEvent> & event)
    {
        if (canvas->isMoving) return;
        canvas->canMove = false;
        vpTakeShapshot();
        scaleOriginX = drawer->translateCanvasX(event->x + canvas->vpX);
        scaleOriginY = drawer->translateCanvasY(event->y + canvas->vpY);
        drawer->clipToViewport();
    }

    inline void PlotCanvas_onScaleMore(const shared_ptr<MouseWheelEvent> & event)
    {
        if (canvas->isMoving) return;
        UNREFERENCED_PARAMETER(event);
        scaleReset();
    }

    inline void PlotCanvas_onScale(const shared_ptr<MouseWheelEvent> & event)
    {
        if (canvas->isMoving) return;
        scrollValue -= event->delta;
        animation->reset();
    }

    inline void PlotCanvas_onScaleEnd()
    {
        scaleReset();
        canvas->canMove = true;
        drawer->resetClipToCanvas();
        canvas->forceRedraw();
    }

    inline void PlotCanvas_onScaleAnimationProgress(double k)
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
        canvas->forceRedraw();
        canvas->forceCopyBuffer();
    }

    inline void PlotCanvas_onScaleAnimationComplete()
    {
        PlotCanvas_onScaleEnd();
    }

    inline void PlotCanvas_onMouseWheel(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<MouseWheelEvent>(_event);

        if (!animation->isRunning())
        {
            PlotCanvas_onScaleBegin(event);
        }
        else
        {
            PlotCanvas_onScaleMore(event);
        }

        PlotCanvas_onScale(event);
    }

    void setup()
    {
        mathConstants[L"PI"] = std::atan(1) * 4;

        animation = AnimationPtr(new Animation(
            hWnd, IDT_TIMER_PLOT,
            Easing::cubicOut,
            PlotCanvas_onScaleAnimationProgress,
            PlotCanvas_onScaleAnimationComplete,
            300
            ));

        options = PlotOptionsPtr(new OptionBag());
        options->calculateOuterBoundaryInCenter(CANVAS_ENLARGE);

        options->formulaColors.push_back(Color(255, 47, 197, 255));
        options->formulaObjects.push_back(ExpDrawerPtr(new ExpDrawer(L"sin(x)", mathConstants)));

        drawer = PlotDrawerPtr(new Drawer(options, canvas->memDC));
        drawer->setSize(canvas->canvasW, canvas->canvasH, canvas->vpX, canvas->vpY, canvas->width, canvas->height);

        checkShowGrid->addEventHandler(EventName::EVENT_CLICK, CheckShowGrid_onClick);
        checkShowGrid->setChecked(true);

        canvas->addEventHandler(EventName::EVENT_BUFFER_REDRAW, PlotCanvas_onRedrawBuffer);
        canvas->addEventHandler(EventName::EVENT_CANVAS_BEGINMOVE, PlotCanvas_onCanvasBeginMove);
        canvas->addEventHandler(EventName::EVENT_CANVAS_MOVE, PlotCanvas_onCanvasMove);
        canvas->addEventHandler(EventName::EVENT_MOUSEWHEEL, PlotCanvas_onMouseWheel);
        canvas->forceRedraw();
    }
}