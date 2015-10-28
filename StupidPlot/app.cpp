#include <app.h>

using namespace Layout;
using namespace Events;

GdiplusStartupInput     gdiplusStartupInput;
ULONG_PTR               gdiplusToken;

HWND                    hWnd;

ContainerPtr            container;
LayoutManagerPtr        lm;
EventManagerPtr         em;

Control::Control        * groupCanvas;
Control::Checkbox       * checkShowGrid;
Control::Control        * editGridSize;
Control::Canvas         * canvas;

void CheckShowGrid_onClick(Control::Control * _control, const EventPtr & _event);
void setup();

void StupidPlot::App::init(HWND _hWnd)
{
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    hWnd = _hWnd;

    container = ContainerPtr(new Container());
    lm = LayoutManagerPtr(new LayoutManager(hWnd));
    em = EventManagerPtr(new EventManager(container));

    canvas = new Control::Canvas(hWnd, IDC_STATIC_CANVAS);
    groupCanvas = new Control::Control(hWnd, IDC_STATIC_GROUP_CANVAS);
    checkShowGrid = new Control::Checkbox(hWnd, IDC_CHECK_SHOW_GRID);
    editGridSize = new Control::Control(hWnd, IDC_EDIT_GRID_SIZE);

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

void StupidPlot::App::terminate()
{
    if (editGridSize) delete editGridSize;
    if (checkShowGrid) delete checkShowGrid;
    if (groupCanvas) delete groupCanvas;
    if (canvas) delete canvas;

    GdiplusShutdown(gdiplusToken);
}

void StupidPlot::App::updateSize()
{
    lm->updateSize();
    lm->relayout();
}

BOOL StupidPlot::App::handleEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return em->handle(uMsg, wParam, lParam);
}

void setup()
{
    checkShowGrid->addEventHandler(EventName::EVENT_CLICK, CheckShowGrid_onClick);
    checkShowGrid->setChecked(true);
}

void CheckShowGrid_onClick(Control::Control * _control, const EventPtr & _event)
{
    UNREFERENCED_PARAMETER(_control);
    UNREFERENCED_PARAMETER(_event);
    editGridSize->setEnabled(checkShowGrid->isChecked());
}