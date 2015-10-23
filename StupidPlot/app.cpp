#include <app.h>

GdiplusStartupInput      gdiplusStartupInput;
ULONG_PTR                gdiplusToken;

HWND                     hWnd;

Container                * container;
Layout::LayoutManager    * lm;
Event::EventManager      * em;

Control::Control         * groupCanvas;
Control::Checkbox        * checkShowGrid;
Control::Control         * editGridSize;
Control::Canvas          * canvas;

void CheckShowGrid_onClick(Control::Control * _control, shared_ptr<Event::Event> _event);
void setup();

void StupidPlot::App::init(HWND _hWnd)
{
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    hWnd = _hWnd;

    container = new Container();
    lm = new Layout::LayoutManager(hWnd);
    em = new Event::EventManager(container);

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

    if (lm) delete lm;
    if (em) delete em;
    if (container) delete container;

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
    checkShowGrid->addEventHandler(Event::EVENT_CLICK, CheckShowGrid_onClick);
    checkShowGrid->setChecked(true);
}

void CheckShowGrid_onClick(Control::Control * _control, shared_ptr<Event::Event> _event)
{
    UNREFERENCED_PARAMETER(_control);
    UNREFERENCED_PARAMETER(_event);
    editGridSize->setEnabled(checkShowGrid->isChecked());
}