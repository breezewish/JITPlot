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

    Control                 * grpCanvas;
    Checkbox                * chkShowGrid;
    Checkbox                * chkAntialias;
    Control                 * txtGridSize;
    BufferedCanvas          * bmpCanvas;

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

        bmpCanvas = new BufferedCanvas(hWnd, IDC_STATIC_CANVAS, CANVAS_ENLARGE);
        grpCanvas = new Control(hWnd, IDC_STATIC_GROUP_CANVAS);
        chkShowGrid = new Checkbox(hWnd, IDC_CHECK_SHOW_GRID);
        chkAntialias = new Checkbox(hWnd, IDC_CHECK_ANTIALIAS);
        txtGridSize = new Control(hWnd, IDC_EDIT_GRID_SIZE);

        chkShowGrid->setChecked(true);
        chkAntialias->setChecked(true);

        container
            ->addControl(bmpCanvas)
            ->addControl(grpCanvas)
            ->addControl(chkShowGrid)
            ->addControl(chkAntialias)
            ->addControl(txtGridSize);

        lm
            ->enableMagnet(bmpCanvas, true, true, true, true)
            ->enableMagnet(grpCanvas, false, true, true, false)
            ->enableMagnet(chkShowGrid, false, true, true, false)
            ->enableMagnet(chkAntialias, false, true, true, false)
            ->enableMagnet(txtGridSize, false, true, true, false);

        updateSize();
        setup();
    }

    void App::terminate()
    {
        delete txtGridSize;
        delete chkShowGrid;
        delete chkAntialias;
        delete grpCanvas;
        delete bmpCanvas;

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

    inline void chkShowGrid_onClick(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        txtGridSize->setEnabled(chkShowGrid->isChecked());
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

        drawer = PlotDrawerPtr(new Drawer(options, bmpCanvas->memDC, chkAntialias->isChecked()));
        drawer->setSize(bmpCanvas->canvasW, bmpCanvas->canvasH, bmpCanvas->vpX, bmpCanvas->vpY, bmpCanvas->width, bmpCanvas->height);

        chkShowGrid->addEventHandler(EventName::EVENT_CLICK, chkShowGrid_onClick);
        chkAntialias->addEventHandler(EventName::EVENT_CLICK, chkAntialias_onClick);

        bmpCanvas->addEventHandler(EventName::EVENT_BUFFER_REDRAW, bmpCanvas_onRedrawBuffer);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_BEGINMOVE, bmpCanvas_onCanvasBeginMove);
        bmpCanvas->addEventHandler(EventName::EVENT_CANVAS_MOVE, bmpCanvas_onCanvasMove);
        bmpCanvas->addEventHandler(EventName::EVENT_MOUSEWHEEL, bmpCanvas_onMouseWheel);
        bmpCanvas->forceRedraw();
    }
}