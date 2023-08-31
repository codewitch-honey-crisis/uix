##### honey the codewitch
# [UIX](https://honeythecodewitch.com/uix)

UIX is a cross-platform draw-on-demand library that builds on [GFX](https://honeythecodewitch.com/gfx). It provides a "control" based framework for laying out screen elements and responding to input. Controls include things like labels, images, canvases and push buttons. Since it builds on GFX, it brings with it all of the advanced features of GFX, including TrueType/OpenType, JPG, PNG and SVG support.


UIX is copyright (C) 2023-2024 by honey the codewitch. UIX is licensed under the MIT license.

---
Get started using GFX by referencing it from your PlatformIO project's INI file:

For C++17
```ini
[env:esp32-uix]
platform = espressif32 ; other platforms are supported too
board = node32s ; other boards are also supported
framework = arduino ; other frameworks work too
monitor_speed = 115200
upload_speed = 921600
lib_deps = 
	codewitch-honey-crisis/htcw_uix
lib_ldf_mode = deep
build_unflags=-std=gnu++11
build_flags=-std=gnu++17
```

For C++14
```ini
[env:esp32-uix-cpp14]
platform = espressif32
board = node32s
framework = arduino
monitor_speed = 115200
upload_speed = 921600
lib_deps = 
	codewitch-honey-crisis/htcw_uix
lib_ldf_mode = deep
build_unflags=-std=gnu++11
build_flags=-std=gnu++14
```
Above the last 4 entries starting at `lib_deps` are what include UIX and make it available which will also include GFX in your project. The build flags are to update C++ to GNU C++17 or GNU C++14 which is necessary for GFX (and thus UIX) to compile.

Finally include the main header:
```cpp
#include <uix.hpp>
```
and then access the `uix` namespace, optionally importing it:
```cpp
using namespace uix;
```
Note: It's often desirable to include `gfx.hpp` and import the `gfx` `namespace` as well because they are typically used in tandem.

## Overview

UIX provides demand drawing functionality to your projects. This means instead of deciding to draw on the screen, the framework calls you to tell you when to draw. One of the advantages is that you do not have to manage updating the screen yourself. UIX always knows what to update on the on the screen and where.

Another advantage of this is the use of controls to construct your screen. Controls are rectangular widgets that do a specific thing. Examples include a label, a push button and an image control. These abstract the necessary drawing and input handling (where applicable) to perform their task.

UIX uses a developer supplied transfer buffer to perform drawing operations. This is a memory region (32kB or more per buffer is recommended in most cases) that is used to create a backing bitmap upon which all drawing operations are performed. As the bitmaps are drawn they are flushed to the display. When the transfer buffer is not large enough to hold an entire control, the control is drawn multiple times - different portions to different bitmaps. Therefore, the larger the transfer buffer, the fewer redraws and the better your code will perform. UIX can use two buffers which allows for full utilization of DMA when available. That means where DMA is being used, UIX can draw while it's sending at the same time for better performance at the cost of twice the memory for keeping two transfer buffers instead of one.

All drawing operations are performed using [GFX](https://honeythecodewitch.com/gfx). Each control has an `on_paint()` method that is called when the control needs to be redrawn. Within this routine all drawing operations are specified using coordinates local to the control itself, and are translated to physical screen coordinates by the framework. This translation also applies to `on_touch()` which handles touch events.

Neither UIX nor GFX have built in facilities for writing to displays. They are cross-platform, while display hardware is device and platform specific. GFX has many drivers written for it which can be used with UIX. In addition Espressif ESP32s have an ESP LCD Panel API that fits UIX like a glove, in some cases not even requiring additional dependencies since the display drivers are built in to the ESP-IDF. Finally, you can use any 3rd party code that can send raw bitmaps to a display.
___

## Contents

1. [Screens](screens.md)
    - 1.1 [Creating the transfer buffer](screens.md#1.1)
    - 1.2 [Declaring the screen](screens.md#1.2)
    - 1.3 [Creating the flush callback](screens.md#1.3)
    - 1.4 [Creating the touch callback](screens.md#1.4)
    - 1.5 [Defining and configuring the screen](screens.md#1.5)
    - 1.6 [Registering controls](screens.md#1.6)
    - 1.7 [Updating the screen](screens.md#1.7)
2. [Controls](controls.md)
    - 2.1 [Labels](controls.md#2.1)
    - 2.2 [Push buttons](controls.md#2.2)
    - 2.3 [Images](controls.md#2.3)
    - 2.4 [SVG boxes](controls.md#2.4)
    - 2.5 [Canvases](controls.md#2.5)
    - 2.6 [Custom controls](controls.md#2.6)

[Screens](screens.md) →