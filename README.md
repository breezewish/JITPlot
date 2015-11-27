# StupidPlot

An Win32 application to draw plots. To compile the project, you need at least Visual Studio 2015 or other compilers that support C++11 features.

## Highlights

### Expression JIT Evaluation

You can input expressions like `sqrt(5*sin(x+1))/1.5`. The expression will be compiled to **native x86 machine code** to achieve best performance ([JIT](https://en.wikipedia.org/wiki/Just-in-time_compilation)). To be more exact, arithmetic operations (`add`, `subtract`, `multiply`, `divide`, `negative`, `abs`) and square root (`sqrt`) are compiled to use Intel [SSE 2](https://en.wikipedia.org/wiki/Streaming_SIMD_Extensions) instructions, while trigonometric functions (`sin`, `cos`) and other functions (`log`, `pow`) are compiled into x87 floatpoint instructions.

#### Parse

- `formula\lexer.h`: Lexer. Turn expressions into tokens.

- `formula\parser.h`: Parser. Create [RPN](https://en.wikipedia.org/wiki/Reverse_Polish_notation).

#### Compile

- `formula\functions.h`: LIR instructions definitions for functions like `sin`, `cos`, `add`, `subtract`.

- `formula\lir.h`: Convert LIR instructions into assembly instructions.

- `formula\assembler.h`: Convert assembly instructions into machine code.

#### Evaluate

- `formula\executable.h`: `VirtualAlloc`, `VirtualProtect` stuff... Execute machine code on x86 platform.

### High Performance Plot Drawing

I used several optimize techniques to boost the performance of drawing the plot. This is the foundation of smoothly dragging the plot or scaling the plot.

JIT evaluation above is one of the techniques. Others are:

- `formula\exp.h`: Range buffered expression evaluator

- `formula\expdrawer.h`: SSE optimized coordinate system transformer

- `ui\controls\bufferedcanvas.h`: Dragging-optimized canvas

### Auto Layout Library

The project contains a simple and stupid layout library, which is used to automatically layout controls when user is resizing the window. Developers only need to specify magnet settings (aka anchor) for controls. For example, `right, top` means the control will always stick at the right top corner when resizing the window, while `left top right bottom` means the control will be resized when size of window is changed.

- `ui\layout\layoutmanager.h`: The layout manager.

### Control Library

The project encapsuled Win32 API calls and messages so that developers can use event callback to handle the message of controls and use object oriented ways to change the properties (like text) of the control.

- `ui\controls\*`: Encapsuled control classes, buttons, edits, etc.

- `ui\events\*`: Encapsuled event classes, MouseEvents, KeyboardEvents, etc.

### GDI+/GDI Drawing

You can use either GDI+ or GDI to draw plots. If you are using GDI+, you will get antialiased plot lines. If you are using GDI, you will get better performance when dragging or scrolling.

- `plot\provider\gdiprovider.h`: GDI drawer

- `plot\provider\gdiplusprovider.h`: GDI+ drawer

### Smooth Scrolling

You can scale the formula plot by scrolling. The canvas will be scaled smoothly by running an easing animation for the scaling.

- `plot\animation.h`: Easing animation manager

### User Interactive

The plot is interactive. You can click the plot in the canvas to adjust options, such as plot color.

- `app.h@_updateCursorHitTest`: Plot hit testing

- `ribbon.h`: Ribbon context menu

- `StupidPlotRibbonMarkup.xml`: Ribbon Framework UI definitions

## TODO

- JIT: Support `arcsin`, `arccos`

## Thanks To

- [FPU tutorial](www.ray.masmcode.com/tutorial/index.html)

- [Advanced Shunting Yard algorithm](https://github.com/eriksank/dijkstra-shunting-yard)