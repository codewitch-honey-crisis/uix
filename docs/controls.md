#### [← Back to index](index.md)

<a name="2"></a>

# 2. Controls

Controls are rectangular objects that know how to draw themselves and may respond to touch events. You lay out controls on a screen in order to compose your user interface.

Controls have a z-order which dictates which controls are in front of other controls. The z-order is dictated by the order the controls are registered on the page, with the first one being backmost. 

You can use one of the several provided controls or derive from `control<>` and create your own.

Typically you'd declare your controls in a `ui.hpp` file and define them in `ui.cpp` but that is not required. They are normal C++ and will work anywhere, but in the interest of organization, segregating your UI to separate files can only benefit your project.

```cpp
// ui.hpp
#pragma once
#include <uix.hpp>
#include <gfx.hpp>

// declare the format of the screen
using screen_t = uix::screen<gfx::rgb_pixel<16>>;
// for access to RGB565 colors which the screen uses (native LCD)
using color_t = gfx::color<typename screen_t::pixel_type>;
// for access to RGBA8888 colors which controls use
using color32_t = gfx::color<gfx::rgba_pixel<32>>;

// declare the main screen
extern screen_t main_screen;

// declare the init routine for the screen
extern void main_screen_initialize();

// declare the control types to match the screen
using label_t = uix::label<typename screen_t::control_surface_type>;
using svg_box_t = uix::svg_box<typename screen_t::control_surface_type>;
using image_t = uix::image<typename screen_t::control_surface_type>;
using push_button_t = uix::push_button<typename screen_t::control_surface_type>;

// declare the controls
extern label_t my_label;
extern svg_box_t my_svg;
extern image_t my_image;
extern push_button_t my_button;
```

In ui.cpp you put all the meat.

```cpp
// ui.cpp
#include <ui.hpp>
using namespace uix;
using namespace gfx;

// transparent "color" for setting up controls
constexpr static const rgba_pixel<32> transparent(0,0,0,0);

// 32KB transfer buffers
constexpr static const int lcd_transfer_buffer_size = 32 * 1024;
// primary transfer buffer
static uint8_t lcd_transfer_buffer1[lcd_transfer_buffer_size];
// secondary transfer buffer for DMA performance (not required)
static uint8_t lcd_transfer_buffer2[lcd_transfer_buffer_size];
// define the main screen as 320x200 and give it the transfer buffers
screen_t main_screen({320,200},sizeof(lcd_transfer_buffer1), lcd_transfer_buffer1, lcd_transfer_buffer2);

// define the controls
label_t my_label(main_screen);
svg_box_t my_svg(main_screen);
image_t my_image(main_screen);
push_button_t my_button(main_screen);

static void uix_on_flush(const rect16& bounds, 
                    const void* bmp, 
                    void* state) {
    // omitted
    ...
}
static void uix_on_touch(point16* out_locations, 
                    size_t* in_out_locations_size, 
                    void* state) {
    // omitted
    ...
}
void main_screen_initialize() {
    main_screen.on_flush_callback(uix_on_flush);
    main_screen.on_touch_callback(uix_on_touch);
    // set up controls (omitted)
    ...
    // register controls
    main_screen.register_control(my_label);
    main_screen.register_control(my_svg);
    main_screen.register_control(my_image);
    main_screen.register_control(my_button);
}

```

<a name="2.1"></a>

## 2.1 Labels

A label is a control that displays some text. It supports every font style that GFX supports as well as justification.

Typically you'd set the design properties, and give it some static text. If the `char` pointer you give it has text that changes you will need to call `invalidate()` yourself to tell the label to redraw.

Most of the properties apply to all font styles, but vector (TrueType/OpenType) fonts allow you to set the `text_line_height()` which indicates the font height, in pixels.

```cpp
// set the design properties
my_label.text("hello world");
my_label.text_line_height(main_screen.dimensions().height / 3);
my_label.bounds(srect16(0,0,main_screen.bounds().x2,my_label.text_line_height()+2));
my_label.text_color(color32_t::red,true);
my_label.background_color(transparent,true);
my_label.border_color(transparent,true);
my_label.text_justify(uix_justify::center);
my_label.padding({0,0});
my_label.text_open_font(&text_font);
```

<a name="2.2"></a>

## 2.2 Push buttons

Push buttons work a lot like labels, but have a simple built in animation for when the button is touched and released, and have a callback that lets you know the same.

```cpp
// set the design properies
... 
// set the button callback
my_button.on_pressed_changed_callback([](bool pressed,void* state){
    printf("press changed: %s\n",pressed?"pressed":"released");
});
```

<a name="2.3"></a>

## 2.3 Images

An image control displays an image, like a .PNG on the screen. Oftentimes, such as when a stream comes from an embedded header you can simply set the `stream()` along with the other design properties and the image control will just use it. However, if an image comes from a filesystem or a network there are extra steps.

In essence, to load from an external source, you'd hook `on_load_callback()` and `on_unload_callback()` to open and close the stream:
```cpp
// this code is platform dependent:
#include <stdlib.h>
FILE* image_file;

file_stream image_stream;
...
my_image.bounds(main_screen.bounds());
my_image.on_load_callback([](void* state) {
    // this code is platform dependent:
    // open the file (stdlib)
    image_file = fopen("/sd/my_image.png","rb");
    
    // attach the stream to the file
    image_stream.set(&image_file);
    // set the stream. don't invalidate
    ((image_t*)state)->stream(&image_stream,false);
},&my_image);
my_image.on_unload_callback([](void*state){
    // this code is platform dependent:
    // close the file
    fclose(image_file);
});
main_screen.register_control(my_image);
```

Images can load slowly due to the transfer window not being able to contain the entire image, plus requiring decompression from the beginning each time it renders a portion of the image. To mitigate this, image caching occurs if there is available memory. You can use a custom allocator in the constructor if your platform has an alternative call for using PSRAM, for example. The memory required is the size of the image as an uncompressed bitmap at the same pixel format as the screen.

<a name="2.4"></a>

## 2.4 SVG boxes

SVG or Scalable Vector Graphics is a standard for creating and viewing vector based images. Being vector based, they can scale to any size required. GFX supports a simple subset of SVG suitable for displaying most graphics and as such UIX does as well, in this case via the `svg_box<>` control.

The SVG graphics will be scaled to the dimensions of the control.

```cpp
srect16 b = srect16(0,0,
                    main_screen.dimensions().height/2,
                    main_screen.dimensions().height/2)
                        .center_horizontal(main_screen.bounds())
                        .offset(0,10);
my_svg.bounds(b);
my_svg.doc(&my_svg_doc); // set the gfx::svg_doc
main_screen.register_control(my_scv);
```

<a name="2.5"></a>

## 2.5 Canvases

Canvases are general purpose draw areas that are painted by a user supplied callback.
```cpp
my_canvas.bounds(srect16(0,0,main_screen.bounds().x2,31));
my_canvas.on_paint_callback([](canvas_t::control_surface_type& destination,const uix::srect16& clip, void* state){
    // use this to keep a counter so we know what to draw
    static int counter = 0;
    // compute the steps we need to draw the animation
    int steps = destination.dimensions().width/10;
    int j = counter;
    for(int i = 0;i<steps;++i) {
        // create a pixel with red and green channels
        gfx::rgb_pixel<24> px(j,i*(255/steps),0);
        // draw it
        gfx::draw::filled_rectangle(destination,uix::srect16(i*steps,0,((i+1)*steps)-1,destination.bounds().y2),px);
        // wrap around
        ++j;
        if(j>255) {
            j=0;
        }
    }
    // increment and wrap
    ++counter;
    if(counter>255) {
        counter = 0;
    }
});

main_screen.register_control(my_canvas);
```
This will draw exactly once, which is not what we want. Instead we need to periodically refresh the control by calling `invalidate()` on it somewhere in the guts of the application loop:
```cpp
static uint32_t ts = 0;
if(get_uptime_ms()>ts+10) {
    ts = get_uptime_ms();
    // force a redraw
    my_canvas.invalidate();
}
```
This will make it redraw about ever 1/100th of a second.

Note that all draws are done using GFX to the `destination` and all coordinates are local to the canvas control, starting at (0,0) for the top left of the control.

<a name="2.6"></a>

## 2.6 Custom controls

The following is a complete control that renders icons generated using the GFX [icon pack generator](https://honeythecodewitch.com/gfx/iconPack)

It handles painting and resizing, plus a custom template parameter and a number of design properties.

Note how when design properties are set the control is invalidated, and often `m_rect` is set to `{0,0,0,0}`.

This forces a redraw, and a recompute of the justification, respectively.

```cpp
// a control that renders a GFX icon
template <typename IconType,typename ControlSurfaceType>
class icon_box : public uix::control<ControlSurfaceType> {
    // public and private type aliases
    // pixel_type and palette_type are
    // required on any control
   public:
    using type = icon_box;
    // the following two declarations are required by UIX
    using pixel_type = typename ControlSurfaceType::pixel_type;
    using palette_type = typename ControlSurfaceType::palette_type;
    // the type of icon, like const_bitmap<alpha_pixel<8>>
    using icon_type = IconType;
   private:
    // makes it easier to refer to the base
    using base_type = uix::control<ControlSurfaceType>;
    // we need it later
    using control_surface_type = ControlSurfaceType;
    // member data
    // the icon
    const icon_type* m_ico;
    uix::srect16 m_rect;
    uix::uix_justify m_justify;
    gfx::rgba_pixel<32> m_color;
protected:
    // these two methods are by
    // convention, not required
    // but makes it easier for 
    // derived classes
    void do_move_control(icon_box& rhs) {
        ((base_type*)this)->do_move_control(rhs);
        m_ico = rhs.m_ico;
        m_rect = rhs.m_rect;
        m_justify = rhs.m_justify;
        m_color = rhs.m_color;
    }
    void do_copy_control(const icon_box& rhs) {
        ((base_type*)this)->do_copy_control(rhs);
        m_ico = rhs.m_ico;
        m_rect = rhs.m_rect;
        m_justify = rhs.m_justify;
        m_color = rhs.m_color;
    }
   public:
    // move constructor
    icon_box(icon_box&& rhs) {
        do_move_control(rhs);
    }
    // move assignment
    icon_box& operator=(icon_box&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    // copy constructor
    icon_box(const icon_box& rhs) {
        do_copy_control(rhs);
    }
    // copy assignment
    icon_box& operator=(const icon_box& rhs) {
        do_copy_control(rhs);
        return *this;
    }
    // the icon to use
    const icon_type* icon() const {
        return m_ico;
    }
    // sets the icon
    void icon(const icon_type* value) {
        if (m_ico != value) {
            m_ico = value;
            m_rect = {0,0,0,0};
            this->invalidate();
        }
    }
    // the color to use
    gfx::rgba_pixel<32> color() const {
        return m_color;
    }
    // sets the color
    void color(gfx::rgba_pixel<32> value) {
        if(m_color!=value) {
            m_color = value;
            this->invalidate();
        }
    }
    // the justification
    uix::uix_justify justify() const {
        return m_justify;
    }
    // sets the justification
    void justify(uix::uix_justify value) {
        if(m_justify!=value) {
            m_justify = value;
            m_rect = {0,0,0,0};
            this->invalidate();
        }
    }
    // handle on resize
    virtual void bounds(const uix::srect16& value) override {
        base_type::bounds(value);
        m_rect = {0,0,0,0};
    }
    // standard control constructor
    icon_box(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
        : base_type(parent, palette), 
        m_ico(nullptr), 
        m_rect(0,0,0,0),
        m_justify(uix::uix_justify::top_left),
        m_color(gfx::color<gfx::rgba_pixel<32>>::white) {
    }
    // standard control constructor (alt)
    icon_box()
        : base_type(), 
        m_ico(nullptr), 
        m_rect(0,0,0,0),
        m_justify(uix::uix_justify::top_left),
        m_color(gfx::color<gfx::rgba_pixel<32>>::white) {
    }
    // paints the control
    virtual void on_paint(control_surface_type& destination, const uix::srect16& clip) override {
        Serial.println("On paint");
        // call the base on paint method
        base_type::on_paint(destination, clip);
        // if there's an icon set, render it
        // scaled to the control
        if (m_ico != nullptr) {
            // we indicate the need to recompute the location if the stored rect is (0,0)-(0,0)
            if(m_rect.x1==0&&m_rect.y1==0&&m_rect.x2==0&&m_rect.y2==0) {
                uint16_t w = m_ico->dimensions().width;
                uint16_t h = m_ico->dimensions().height;
                m_rect = uix::srect16(0,0,w-1,h-1);
                switch(m_justify) {
                    case uix::uix_justify::top_middle:
                        m_rect.center_horizontal_inplace((uix::srect16)destination.bounds());
                        break;
                    case uix::uix_justify::top_right:
                        m_rect.offset_inplace(destination.dimensions().width-w,0);
                        break;
                    case uix::uix_justify::center_left:
                        m_rect.center_vertical_inplace((uix::srect16)destination.bounds());
                        break;
                    case uix::uix_justify::center:
                        m_rect.center_inplace((uix::srect16)destination.bounds());
                        break;
                    case uix::uix_justify::center_right:
                        m_rect.center_vertical_inplace((uix::srect16)destination.bounds());
                        m_rect.offset_inplace(destination.dimensions().width-w,0);
                        break;
                    case uix::uix_justify::bottom_left:
                        m_rect.offset_inplace(0,destination.dimensions().height-h);
                        break;
                    case uix::uix_justify::bottom_middle:
                        m_rect.center_horizontal_inplace((uix::srect16)destination.bounds());
                        m_rect.offset_inplace(0,destination.dimensions().height-h);
                        break;
                    case uix::uix_justify::bottom_right:
                        m_rect.offset_inplace(destination.dimensions().width-w,
                                            destination.dimensions().height-h);
                        break;
                    default: // top_left
                    break;
                }
                
            } 
            // draw the icon
            gfx::draw::icon(destination,
                        m_rect.top_left(),
                        *m_ico,
                        m_color,
                        gfx::rgba_pixel<32>(0,0,0,255),
                        true,
                        false,
                        &clip);
        } 
    }
};
```