#### [← Back to index](index.md)

<a name="1"></a>

# 1. Screens

Screens are a primary facility in graphics used for rendering and managing the screen and all of its controls.

```cpp
#include <uix.hpp>
#include <gfx.hpp>
using namespace uix;
using namespace gfx;
// declare the format of the screen
using screen_t = screen<rgb_pixel<16>>;
// for access to RGB565 colors which the screen uses (native LCD)
using color_t = color<typename screen_t::pixel_type>;
// for access to RGB8888 colors which controls use
using color32_t = color<rgba_pixel<32>>;
// declare the control types to match the screen
using label_t = label<typename screen_t::control_surface_type>;
using svg_box_t = svg_box<typename screen_t::control_surface_type>;
// 32KB transfer buffers
constexpr static const int lcd_transfer_buffer_size = 32 * 1024;
// primary transfer buffer
static uint8_t lcd_transfer_buffer1[lcd_transfer_buffer_size];
// secondary transfer buffer for DMA performance (not required)
static uint8_t lcd_transfer_buffer2[lcd_transfer_buffer_size];
// declare the main screen and give it the transfer buffers
static screen_t main_screen(sizeof(lcd_transfer_buffer1), lcd_transfer_buffer1, lcd_transfer_buffer2);

```

<a name="1.1"></a>

## 1.1 Creating the transfer buffer

A screen needs at least one transfer buffer associated with it that it uses to draw to. Each time it fills the transfer buffer, that gets written to the display, and the process repeats until the entire screen is rendered. The larger the transfer buffer(s), the better the performance due to requiring fewer redraws. Essentially, if a control cannot be entirely contained within one transfer buffer it will get drawn multiple times in order to produce the complete image.

You should declare a transfer buffer of at least 32KB in most situations. This will hold an area equivelent to 128x128 at 16-bit color. For displays that are very small, you can use less. Very large displays and/or True Color (24-bit color) displays may require more to get acceptable performance.

As above declaring it looks something like this

```cpp
// 32KB transfer buffer
constexpr static const int lcd_transfer_buffer_size = 32 * 1024;
// transfer buffer
static uint8_t lcd_transfer_buffer[lcd_transfer_buffer_size];
```
To take full advantage of DMA on DMA capable systems you should declare a second transfer buffer of the same size as the first.
```cpp
// secondary transfer buffer
static uint8_t lcd_transfer_buffer2[lcd_transfer_buffer_size];
```
You don't have to declare static arrays, as any memory that can be transfered to the display is suitable, hardware allowing.

When you declare the screen, pass in the size and the transfer buffer(s). The second buffer can be null if not used.

```cpp
static screen_t my_screen(sizeof(lcd_transfer_buffer), lcd_transfer_buffer, lcd_transfer_buffer2);
```

<a name="1.2"></a>

## 1.2 Declaring the screen

Typically, a screen can be declared by giving it simply the [pixel type](https://honeythecodewitch.com/gfx/wiki/pixels.md) you want the transfer buffer to be.
Occasionally, as in for some rare displays, such as color e-ink you may need to specify a palette type as well.

```cpp
using screen_t = uix::screen<gfx::rgb_pixel<16>>;
```
The above defines the screen to be in the RGB565 format which is a common format for IoT displays. See the [GFX documentation](https://honeythecodewitch.com/gfx/wiki/index.md) for more information.

You may need to provide additional information to the screen for use with certain displays. Some monochrome displays for example, pack each byte worth of pixels vertically, but arrange the bytes horizontally. GFX is not able to replicate that in memory layout directly, frankly because it's very strange. Furthermore, due to being monochrome, you must update those pixels along the vertical axis in numbers divisible by 8. This presents a rather sticky problem for updating the display as for example, the update rectangle (0,0)-(319,133) is invalid, due to needing to write fractions of a byte.

Fortunately UIX provides the ability to adapt strange display characteristics like the above. The `screen<>` template class is actually a shorthand alias for the complicated, but more full featured `screen_ex<>`.

```cpp
// declare the screen type
using screen_t = uix::screen_ex</*FRAME_ADAPTER=*/ssd1306_surface_adapter,/*X_ALIGN=*/1,/*Y_ALIGN=*/8>;
```
This is a declaration with a custom user supplied frame adapter - a [GFX draw target](https://honeythecodewitch.com/gfx/wiki/draw_targets.md) and a custom vertical alignment of 8 pixels. The draw target is used to translate standard coordinates into coordinates that translate to the odd memory layout described above. The code for it, and an example of using it is provided in the Heltek Wifi Kit V2 Basic example.

The following two declarations are effectively the same:
```cpp
using screen_t = uix::screen<gfx::rgb_pixel<16>>;
```
and
```cpp
using screen_t = uix::screen_ex</*FRAME_ADAPTER=*/gfx::bitmap<gfx::rgb_pixel<16>>,/*X_ALIGN=*/1,/*Y_ALIGN=*/1>;
```
As you can see the default frame adapter is simply a bitmap. By default there's no real need to adapt anything - just writing to the bitmap as is will suffice. Furthermore, the horizontal and vertical alignment are both 1.

For a grayscale or monochrome display that requires no translation you can use `gfx::gsc_pixel<BIT_DEPTH>` such as `gfx::gsc_pixel<1>`, `gfx::gsc_pixel<4>`, `gfx::gsc_pixel<8>` instead of `gfx::rgb_pixel<16>` above, though typically for monochrome, you'll need an x-alignment of 8. E-paper displays typically don't need special alignment. Since they hold their bitmaps in memory, you can write "partial bytes" to them, meaning all update rectangles are valid.

<a name="1.3"></a>

## 1.3 Creating the flush callback

The flush callback is used for sending data to the display. The implementation of this callback is provided by the developer-user of UIX and is platform and device specific. The callback typically has one task, which is to send a rectangular bitmap to a particular location on the display as quickly as possible.

The buffer to send is always in the screen's `pixel_type`. Earlier we declared `uix::screen<gfx::pixel<rgb_pixel<16>>` which will yield a 16-bit RGB bitmap. If any further translation must be done, it should be done either here, or using a custom surface adapter as shown prior.

Here's an example of implementing the flush callback with a GFX compatible driver and no DMA:

```cpp
void uix_on_flush(const gfx::rect16& bounds, 
                    const void* bmp, 
                    void* state) {
    if(active_screen!=nullptr) {
        gfx::const_bitmap<screen_t::pixel_type,screen_t::palette_type> cbmp(bounds.dimensions(),bmp,active_screen->palette());
        gfx::draw::bitmap(lcd,bounds,cbmp,cbmp.bounds());
        active_screen->flush_complete();
    }
}
```
Here we assume `screen_t* active_screen` points to the current screen being displayed, and `lcd` is the name of the GFX compatible display driver. What we've done is wrapped the raw `bmp` buffer into a `const_bitmap<>` which makes it a GFX "draw source" suitable for sending to `draw::bitmap<>`, which ultimately writes it to the display. Note how we call `active_screen->flush_complete()`. This is critical to notify the screen that the transfer to the display has been completed.

Let's do the same thing with that driver using DMA.

```cpp
void uix_on_flush(const gfx::rect16& bounds, 
                    const void* bmp, 
                    void* state) {
    if(active_screen!=nullptr) {
        gfx::const_bitmap<screen_t::pixel_type,screen_t::palette_type> cbmp(bounds.dimensions(),bmp,active_screen->palette());
        gfx::draw::bitmap_async(lcd,bounds,cbmp,cbmp.bounds());
    }
}
// only needed for DMA. Called when we need to wait for a new buffer to become available.
void uix_on_wait(void* state) {
    gfx::draw::wait_all_async(lcd);
}
```
Finally, let's look at another example - this time using the ESP LCD Panel API to implement the flush callback, with DMA:

```cpp
void uix_on_flush(const gfx::rect16& bounds, 
                    const void* bmp, 
                    void* state) {
    // quirk with this is the y2 and x2 actually need 1 added to them for the API
    esp_lcd_panel_draw_bitmap(lcd_handle,bounds.x1,bounds.y1,bounds.x2+1,bounds.y2+1,(void*)bmp);
}
// called by the ESP LCD Panel API
// only needed if DMA enabled
static bool lcd_flush_ready(esp_lcd_panel_io_handle_t panel_io, 
                            esp_lcd_panel_io_event_data_t* edata, 
                            void* user_ctx) {
    // notify the active screen that the transfer was completed
    if(active_screen!=nullptr) {
        active_screen->flush_complete();
    }
    return true;
}
```
You may notice that there are two different ways of doing DMA with UIX. In the first instance, we used a wait callback to allow UIX to wait for a pending buffer to become available. In the second instance we notified UIX using a callback sourced by the platform. You should also be aware that we don't call `flush_complete()` when the `on_wait_callback` has been set.

<a name="1.4"></a>

## 1.4 Creating the touch callback

Typically, you'll want some sort of interactivity on your screen, and this is often facilitated by a touch screen. UIX includes built-in support for simple user input. Controls can be "touched" which is typically the result of a touch screen being touched, but may be because of a mouse click, a button press or something else, if those emulate touch through the callback. How those are handled is up to the developer.

Like the flush callback, the touch callback is device and platform specific. How you implement it is between you and the device. Here's an example using the FT6336 touch controller as present in the M5Stack Core2 - using the htcw_ft6336 Platform IO library:

```cpp
void uix_on_touch(point16* out_locations, 
                    size_t* in_out_locations_size, 
                    void* state) {
    // should never happen
    if(in_out_locations_size<=0) {
        *in_out_locations_size=0;
        return;
    }
    // ESP32 specific code to delay 1ms
    vTaskDelay(pdMS_TO_TICKS(1));                                          
    // ensure the touch values are up to date
    if (touch.update()) {                    
        // see if the touch has been read                              
        if (touch.xy(&out_locations[0].x, &out_locations[0].y)) {          
            // if there's room for more than one
            if (*in_out_locations_size > 1) {                           
                *in_out_locations_size = 1; // we already have one                              
                // is there another one? (this device supports two touches)
                if (touch.xy2(&out_locations[1].x, &out_locations[1].y)) { 
                    *in_out_locations_size = 2; // now two                     
                }                                                          
            } else {                                                       
                *in_out_locations_size = 1; // only one                 
            }                                                              
        } else {                                                           
            *in_out_locations_size = 0; // no touches                    
        }                                                                  
    }
}
```
What's passed in is a pointer to an array of `uix::point16` structures that are to contain the resulting touch points. Also, the number of points available in the array is passed in. `state` is a user supplied value that is passed to the callback. On return the `out_locations` array should contain any touched points, and the `in_out_locations_size` should contain the number of out locations, but never be greater than it was when passed in.

Here's another example, this time with the GT911 touch controller which supports up to 5 touches, and the htcw_gt911 library:
```cpp
void uix_on_touch(point16* out_locations, 
                    size_t* in_out_locations_size, 
                    void* state) {
    // should never happen
    if(in_out_locations_size<=0) {
        *in_out_locations_size=0;
        return;
    }
    // ensure the touch values are up to date
    touch.update();                             
    size_t touches = touch.locations_size();    
    if (touches) { // any touches?
        // clamp the number of touches to *in_out_locations_size
        if (touches > *in_out_locations_size) { 
            touches = *in_out_locations_size;   
        }
        // up to 5 points
        decltype(touch)::point pt[5];
        touch.locations(pt, &touches);          
        // copy the locations
        for (uint8_t i = 0; i < touches; ++i) { 
            out_locations[i].x = pt[i].x;       
            out_locations[i].y = pt[i].y;       
        }                                       
    }
    // set the size                   
    *in_out_locations_size = touches;
}
```

<a name="1.5"></a>

## 1.5 Defining and configuring the screen

So far we've defined callbacks for touch and flush, but we've only glossed over defining the screen in code, and haven't covered hooking the callbacks up.

Once we've declared `screen_t` and defined the transfer buffer(s) it's simple enough to define the screen:

First don't forget to add the declaration to a .h/.hpp file so we can refer to it elsewhere in our project:
```cpp
extern screen_t main_screen;
```
And in the .cpp file to implement it:
```cpp
screen_t main_screen({LCD_WIDTH,LCD_HEIGHT},lcd_buffer_size,lcd_buffer,lcd_buffer2);
```
You can also use the default contructor:
```cpp
screen_t main_screen;
```
However, if you do, you will need to go through and set the `dimensions()`, `buffer_size()`, `buffer1()` and possible `buffer2()` properties on the screen as well.

We're not done yet. Now we need to hook up the callbacks that we made. Usually you'll do this in some sort of initialization routine for the screen such as `main_screen_initialize()`:

```cpp
void main_screen_initialize() {
    main_screen.on_flush_callback(uix_on_flush);
    // for wait based DMA
    // main_screen.on_wait_callback(uix_on_wait);
    main_screen.on_touch_callback(uix_on_touch);
}
```
If you need some persistent state to pass along with those callbacks it can be passed in as the second parameter to each method and later accessed in the callback using the `void* state` argument.

<a name="1.6"></a>

## 1.6 Registering controls

As mentioned, screens are layed out in controls, with each control being a rectangular window upon the screen that knows how to paint itself, and can respond to touch events.

In order for a control to appear on the screen it must be associated with that screen via the screen's `register_control()` method. Once registered the control's display is kept up to date* by the framework, writing to the transfer buffer and sending as necessary. Whenever any properties on a control such as a `label<>` or a `push_button<>` change, the control is invalidated, and then redrawn accordingly on the next screen update. *The exception is `canvas<>` controls which must be invalidated manually.

Here's an example of setting up and registering a `label_t hello` label. Most of this will be covered later on, but is here for completeness. The main thing is we call `main_screen.register_control(hello);` once we've configured it. You'd typically do this in the screen's initialization routine, such as `main_screen_initialize()` above

```cpp
// declare a transparent pixel/color
// RGBA8888 with a 0 alpha channel
const rgba_pixel<32> transparent(0, 0, 0, 0);

title.text("uix demo");
title.text_line_height(main_screen.dimensions().height / 3);
title.bounds(srect16(0,0,main_screen.dimensions().width-1,title.text_line_height()).center(main_screen.bounds()));
// set the design properties
title.text_color(color32_t::red);
title.background_color(transparent);
title.border_color(transparent,true);
title.text_justify(uix_justify::center);
title.padding({0,0});
title.text_open_font(&text_font);
main_screen.register_control(title);
```

<a name="1.7"></a>

## 1.7 Updating the screen

Updating the screen is simple to do. In your application's main loop you can just call `update()` on the screen instance, like `main_screen.update();`.

However, what it does could use some explaining. If you call update with no arguments, or `update(true)` all invalid areas of the screen will be redrawn.

How it works is this: The screen itself keeps track of all the rectangles that have been reported as dirty, combining overlapping rectangles into one. When `update()` is called then it goes through each dirty rect, and subdivides it vertically by the size of the transfer buffer's maximum allowable lines. For example, if a dirty rectangle is 256x384 then a 32kB transfer buffer (equiv. of 128x128 @ RGB565) would require 6 transfers to the display in order to entirely repaint.

It should be noted that `update()` is in essence a coroutine, and as such it can break up its work into multiple parts to avoid blocking for as long as it otherwise would. In this case, if you pass `false`, as in `update(false)` only one transfer to the LCD will occur in that iteration. You'd often need to call it multiple times (until `dirty()` is `false`) to do a complete refresh. This mode is useful if you're doing some other intensive task, like playing audio on the same thread and you can't have the screen blocking, at least as much as it otherwise would. Do not call `invalidate()` on anything or otherwise modify controls while updating.

This example is geared for Arduino but the code is the same regardless.
```cpp
void loop() {
    main_screen.update();
}
```

[→ Controls](controls.md)

[← Index](index.md)

