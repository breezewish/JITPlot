#pragma once

#include <memory>

namespace StupidPlot
{
    namespace Plot
    {
        typedef double(*EasingFunction)(double k);

        typedef void(*UpdateCallback)(double k);

        typedef void(*CompleteCallback)();

        class Animation
        {
        private:
            LARGE_INTEGER       frequency;
        protected:
            HWND                hWnd;
            EasingFunction      easing;
            UpdateCallback      fUpdate;
            CompleteCallback    fComplete;
            bool                running;

            int                 duration;   // in ms
            int                 fps;

            LARGE_INTEGER       startingTime;
            LARGE_INTEGER       stoppingTime;

            UINT_PTR            timerId;

        public:
            Animation(
                HWND wndHwnd,
                UINT_PTR id,
                EasingFunction _easing,
                UpdateCallback _update,
                CompleteCallback _complete,
                int _duration,
                int FPS = 60
                )
            {
                hWnd = wndHwnd;
                timerId = id;
                easing = _easing;
                fUpdate = _update;
                fComplete = _complete;
                fps = FPS;
                duration = _duration;
                running = false;
            }

            inline bool isRunning()
            {
                return running;
            }

            void start()
            {
                if (running) stop();

                QueryPerformanceFrequency(&frequency);
                QueryPerformanceCounter(&startingTime);

                stoppingTime.QuadPart = startingTime.QuadPart + duration * frequency.QuadPart / 1000;

                SetTimer(hWnd, timerId, 1000 / fps, NULL);

                running = true;
            }

            void stop()
            {
                if (!running) return;

                KillTimer(hWnd, timerId);
                running = false;
            }

            void update()
            {
                if (!running) return;

                LARGE_INTEGER currentTime;
                QueryPerformanceCounter(&currentTime);

                double k;
                bool complete = false;

                if (currentTime.QuadPart <= startingTime.QuadPart)
                {
                    k = 0.0;
                }
                else if (currentTime.QuadPart >= stoppingTime.QuadPart)
                {
                    k = 1.0;
                    complete = true;
                }
                else
                {
                    k = static_cast<double>(currentTime.QuadPart - startingTime.QuadPart) / (stoppingTime.QuadPart - startingTime.QuadPart);
                }

                k = easing(k);
                fUpdate(k);

                if (complete)
                {
                    fComplete();
                    stop();
                }
            }
        };

        class Easing
        {
        public:
            static double cubicOut(double k)
            {
                return --k * k * k + 1;
            }
        };

        typedef std::shared_ptr<Animation> AnimationPtr;
    }
}