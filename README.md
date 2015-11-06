# StupidPlot

An Win32 application to draw plots. To compile the project, you need at least Visual Studio 2015 or any other compilers that support C++11 features.

## Highlights

### Expression JIT Evaluation

You can input expressions like `sqrt(5*sin(x+1))/1.5`. The expression will be compiled to **native x86 machine code** to achieve best performance ([JIT](https://en.wikipedia.org/wiki/Just-in-time_compilation)). To be more exact, arithmetic operations (`add`, `subtract`, `multiply`, `divide`) and square root (`sqrt`) are compiled to use Intel [SSE 2](https://en.wikipedia.org/wiki/Streaming_SIMD_Extensions) instructions, while trigonometric functions (`sin`, `cos`) are compiled to x87 floatpoint instructions.

JIT compiler is not very well developed since it is my first time to create such thing.. I may refactor it to separate the MIR layer later.

#### Parse

- `formula\lexer.h`: Lexer. Turn expressions into tokens.

- `formula\parser.h`: Parser. Create [RPN](https://en.wikipedia.org/wiki/Reverse_Polish_notation).

#### Compile

- `formula\jit\functions.h`: Function definitions like `sin`, `cos`, `add`.

- `formula\jit\functiontable.h`: Function table and operator table. Map `+` operator to `add` function call.

- `formula\jit\assembler.h`: Convert assembler codes into machine code.

#### Evaluate

- `formula\expression.h`: `VirtualAlloc`, `VirtualProtect` stuff... Execute machine code on x86 platform.

### Auto Layout Library

The project contains a simple and stupid layout library, which is used to automatically layout controls when user is resizing the window. Developers only need to specify magnet settings (aka anchor) for controls. For example, `right, top` means the control will always stick at the right top corner when resizing the window, while `left top right bottom` means the control will be resized when size of window is changed.

- `ui\layout\layoutmanager.h`: The layout manager.

### Control Library

The project encapsuled Win32 API calls and messages so that developers can use event callback to handle the message of controls and use object oriented ways to change the properties (like text) of the control.

- `ui\control\*`: Encapsuled control classes, buttons, edits, etc.

- `ui\event\*`: Encapsuled event classes, MouseEvents, KeyboardEvents, etc.

- `ui\event\eventmanager.h`, `ui\control\control.h`: Turn messages into events.

### GDI+/GDI Drawing

You can use either GDI+ or GDI to draw plots. If you are using GDI+, you will get antialiased plot lines. If you are using GDI, you will get better performance when dragging or scrolling.

## TODO

- Control: Scroll resize

- JIT: Support more than 7 operands in XMM registers

- JIT: Support more functions like `log`, `arcsin`, `arccos`

- JIT: Support custom functions

- UI: A lot to do

## Thanks To

- [FPU tutorial](www.ray.masmcode.com/tutorial/index.html)

- [Advanced Shunting Yard algorithm](https://github.com/eriksank/dijkstra-shunting-yard)