#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include <windows.h>

#include <ribbon.h>
#include <ui/container.h>
#include <ui/controls/control.h>
#include <ui/layout/layoutelement.h>

using std::vector;
using std::unordered_map;

namespace StupidPlot
{
    namespace UI
    {
        using namespace Controls;

        namespace Layout
        {
            class LayoutManager
            {
            protected:
                HWND                                    hWnd;
                SIZE                                    size;
                POINT                                   offset;

                // collection of layout elements
                vector<LayoutElement *>                 elements;

            public:
                // Initialize a layout manager, fully occupy the window
                LayoutManager(HWND _hWnd)
                {
                    hWnd = _hWnd;
                    updateSize();
                }

                ~LayoutManager()
                {
                    for (LayoutElement * element : elements)
                    {
                        delete element;
                    }
                }

                void updateSize()
                {
                    RECT rect;
                    GetClientRect(hWnd, &rect);

                    UINT32 ribbonHeight;
                    Ribbon::g_pRibbon->GetHeight(&ribbonHeight);

                    size.cx = rect.right - rect.left;
                    size.cy = rect.bottom - rect.top - ribbonHeight;
                    offset.x = 0;
                    offset.y = ribbonHeight;
                }

                void relayout()
                {
                    HDWP hDefer;
                    hDefer = BeginDeferWindowPos(elements.size());

                    for (LayoutElement * element : elements)
                    {
                        element->relayout(hDefer, size, offset);
                    }

                    EndDeferWindowPos(hDefer);
                }

                LayoutManager * enableMagnet(Win32Control * control, BOOL mLeft = true, BOOL mTop = true, BOOL mRight = false, BOOL mBottom = false)
                {
                    LayoutElement * element = new LayoutElement(control);
                    element->setMagnet(mLeft, mTop, mRight, mBottom);
                    element->setSize(size);
                    elements.push_back(element);
                    return this;
                }
            };

            typedef std::shared_ptr<LayoutManager> LayoutManagerPtr;
        }
    }
}