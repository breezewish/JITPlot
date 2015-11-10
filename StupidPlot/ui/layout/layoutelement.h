#pragma once

#include <memory>

#include <windows.h>

#include <util.h>
#include <ui/controls/win32control.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Layout
        {
            class LayoutElement
            {
            protected:
                BOOL                isEmpty;
                Win32Control        * control;

                BOOL                mLeft, mTop, mRight, mBottom;

                RECT                distanceToBorder;
                SIZE                size;

            public:
                LayoutElement()
                {
                    isEmpty = true;
                }

                LayoutElement(Win32Control * _control)
                {
                    control = _control;
                    isEmpty = false;
                }

                // Set size by capturing the client rect of the control
                BOOL setSize(SIZE parentSize)
                {
                    if (isEmpty)
                    {
                        return false;
                    }

                    HWND hWndParent = GetParent(control->hControl);
                    POINT position = { 0, 0 };
                    MapWindowPoints(control->hControl, hWndParent, &position, 1);

                    RECT rect;
                    GetWindowRect(control->hControl, &rect);

                    POINT _pos;
                    SIZE _size;

                    _pos.x = position.x;
                    _pos.y = position.y;
                    _size.cx = rect.right - rect.left;
                    _size.cy = rect.bottom - rect.top;

                    return setSize(_pos, _size, parentSize);
                }

                // Set size by specifying position and size of the control
                BOOL setSize(POINT _pos, SIZE _size, SIZE parentSize)
                {
                    RECT _d;

                    _d.left = _pos.x;
                    _d.top = _pos.y;
                    _d.right = parentSize.cx - _d.left - _size.cx;
                    _d.bottom = parentSize.cy - _d.top - _size.cy;

                    return setSize(_size, _d);
                }

                // Set size by giving the distance to the parent border
                BOOL setSize(SIZE _size, RECT _distanceToBorder)
                {
                    size = _size;
                    distanceToBorder = _distanceToBorder;
                    return true;
                }

                // Whether to attach to left/top/right/bottom border
                BOOL setMagnet(BOOL left = true, BOOL top = true, BOOL right = false, BOOL bottom = false)
                {
                    mLeft = left;
                    mTop = top;
                    mRight = right;
                    mBottom = bottom;
                    return true;
                }

                BOOL relayout(HDWP hDefer, SIZE parentSize, POINT offset)
                {
                    if (isEmpty)
                    {
                        return false;
                    }

                    LONG left = distanceToBorder.left;
                    LONG top = distanceToBorder.top;
                    LONG width = size.cx;
                    LONG height = size.cy;

                    if (mLeft || mRight)
                    {
                        if (mLeft && !mRight)
                        {
                            // Only attach to left border
                            //left = distanceToBorder.left;
                            //width = size.cx;
                        }
                        else if (mRight && !mLeft)
                        {
                            // Only attach to right border
                            left = parentSize.cx - distanceToBorder.right - size.cx;
                            //width = size.cx;
                        }
                        else
                        {
                            // Attach to both border
                            left = distanceToBorder.left;
                            width = parentSize.cx - distanceToBorder.left - distanceToBorder.right;
                        }
                    }

                    if (mTop || mBottom)
                    {
                        if (mTop && !mBottom)
                        {
                            // Only attach to top border
                            //top = distanceToBorder.top;
                            //height = size.cy;
                        }
                        else if (mBottom && !mTop)
                        {
                            // Only attach to bottom border
                            top = parentSize.cy - distanceToBorder.bottom - size.cy;
                            //height = size.cy;
                        }
                        else
                        {
                            // Attach to both border
                            top = distanceToBorder.top;
                            height = parentSize.cy - distanceToBorder.top - distanceToBorder.bottom;
                        }
                    }

                    if (width < 0) width = 0;
                    if (height < 0) height = 0;

                    DeferWindowPos(
                        hDefer,
                        control->hControl,
                        NULL,
                        left + offset.x,
                        top + offset.y,
                        width,
                        height,
                        SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER
                        );

                    return true;
                }
            };

            typedef std::shared_ptr<LayoutElement> LayoutElementPtr;
        }
    }
}