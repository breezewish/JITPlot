#pragma once

#include <map>
#include <memory>
#include <functional>

#include <windows.h>

#include <util.h>

namespace StupidPlot
{
    using std::function;
    using std::map;
    using std::bind;

    typedef function<void()> CallbackFunction;

    struct ThrottleTimer
    {
        CallbackFunction callback;
        unsigned int timerId;
        unsigned int interval;
        bool isRunning;

        ThrottleTimer()
        {
            isRunning = false;
        }

        ThrottleTimer(unsigned int _timerId, unsigned int _interval, CallbackFunction & _callback)
        {
            timerId = _timerId;
            interval = _interval;
            callback = _callback;
            isRunning = false;
        }
    };

    class Throttler
    {
    protected:
        HWND                                hWnd;
        map<unsigned int, ThrottleTimer>    handlers;

    public:
        static int timerIdCounter;

        Throttler(HWND _hwnd)
        {
            hWnd = _hwnd;
        }

        inline bool canHandleTick(unsigned int timerId)
        {
            return handlers.find(timerId) != handlers.end();
        }

        inline void handleTick(unsigned int timerId)
        {
            auto it = handlers.find(timerId);
            if (it == handlers.end()) return;
            KillTimer(hWnd, timerId);
            it->second.isRunning = false;
            it->second.callback();
        }

        void _startTimer(unsigned int timerId)
        {
            auto it = handlers.find(timerId);
            if (it == handlers.end()) return;
            if (it->second.isRunning) return;
            it->second.isRunning = true;
            SetTimer(hWnd, timerId, it->second.interval, NULL);
        }

        inline CallbackFunction applyThrottle(int interval, CallbackFunction callback)
        {
            auto timerId = ++Throttler::timerIdCounter;
            ThrottleTimer handler(timerId, interval, callback);
            handlers[timerId] = handler;
            return bind(&Throttler::_startTimer, this, timerId);
        }
    };

    typedef std::shared_ptr<Throttler> ThrottlerPtr;
}