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
#include <plot/plotdrawer.h>
#include <plot/plotoptions.h>
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
        return em->handle(uMsg, wParam, lParam);
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
        double w = drawer->translateCanvasW(canvas->canvasW);
        double h = drawer->translateCanvasH(canvas->canvasH);
        options->drawLeft = options->vpLeft - dx;
        options->drawBottom = options->vpBottom - dy;
        options->drawRight = options->drawLeft + w;
        options->drawTop = options->drawBottom + h;
        drawer->draw(canvas->canvasW, canvas->canvasH);
    }

    inline void PlotCanvas_onCanvasBeginMove(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        UNREFERENCED_PARAMETER(_event);

        initialLeft = options->vpLeft;
        initialRight = options->vpRight;
        initialTop = options->vpTop;
        initialBottom = options->vpBottom;
    }

    inline void PlotCanvas_onCanvasMove(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<CanvasMoveEvent>(_event);

        double dx = drawer->translateCanvasW(event->dx);
        double dy = drawer->translateCanvasH(event->dy);
        options->vpLeft = initialLeft - dx;
        options->vpRight = initialRight - dx;
        options->vpTop = initialTop - dy;
        options->vpBottom = initialBottom - dy;
    }

    inline void PlotCanvas_onMouseWheel(Control * _control, const EventPtr & _event)
    {
        UNREFERENCED_PARAMETER(_control);
        auto event = std::dynamic_pointer_cast<MouseWheelEvent>(_event);

        Debug::Debug() << event->delta >> Debug::writeln;
    }

    void setup()
    {
        mathConstants[L"PI"] = std::atan(1) * 4;

        options = PlotOptionsPtr(new PlotOptions());
        options->calculateEnlargedBounary(CANVAS_ENLARGE);

        options->formulaColors.push_back(Color(255, 47, 197, 255));
        options->formulaObjects.push_back(ExpDrawerPtr(new ExpDrawer(L"(sin(x+1)+1)/2", mathConstants)));
        /*options->formulaColors.push_back(Color(255, 255, 197, 255));
        options->formulaObjects.push_back(ExpDrawerPtr(new ExpDrawer(L"(cos(x-1)-1)/2", mathConstants)));
        options->formulaColors.push_back(Color(255, 47, 197, 0));
        options->formulaObjects.push_back(ExpDrawerPtr(new ExpDrawer(L"(x*10)/5", mathConstants)));*/

        drawer = PlotDrawerPtr(new PlotDrawer(options, canvas->memDC));
        drawer->setCanvasSize(canvas->canvasW, canvas->canvasH);

        checkShowGrid->addEventHandler(EventName::EVENT_CLICK, CheckShowGrid_onClick);
        checkShowGrid->setChecked(true);

        canvas->addEventHandler(EventName::EVENT_BUFFER_REDRAW, PlotCanvas_onRedrawBuffer);
        canvas->addEventHandler(EventName::EVENT_CANVAS_BEGINMOVE, PlotCanvas_onCanvasBeginMove);
        canvas->addEventHandler(EventName::EVENT_CANVAS_MOVE, PlotCanvas_onCanvasMove);
        canvas->addEventHandler(EventName::EVENT_MOUSEWHEEL, PlotCanvas_onMouseWheel);
        canvas->forceRedraw();
    }
}