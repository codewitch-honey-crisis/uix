#include <Arduino.h>
#include <lilygot54in7.hpp>
#include <button.hpp>
#include <gfx.hpp>
#include <uix.hpp>
using namespace arduino;
using namespace gfx;
using namespace uix;

lilygot54in7 epd;

using button_a_t = arduino::basic_button;
using button_b_t = arduino::basic_button;
using button_c_t = arduino::basic_button;

button_a_t button_a(39,10,true);
button_b_t button_b(34,10,true);
button_c_t button_c(35,10,true);
// SVG converted to header using
// https://honeythecodewitch.com/gfx/converter
#define BEE_ICON_IMPLEMENTATION
#include <assets/bee_icon.hpp>
// downloaded from fontsquirrel.com and header generated with
// https://honeythecodewitch.com/gfx/generator
#define TELEGRAMA_IMPLEMENTATION
#include <assets/telegrama.hpp>
static const open_font& text_font = telegrama;

// declare the format of the screen
using screen_t = screen<rgb_pixel<16>>;
// declare the control types to match the screen
using label_t = label<typename screen_t::control_surface_type>;
using svg_box_t = svg_box<typename screen_t::control_surface_type>;
using color_t = color<typename screen_t::pixel_type>;
// for access to RGB8888 colors which controls use
using color32_t = color<rgba_pixel<32>>;

// UIX allows you to use two buffers for maximum DMA efficiency
// you don't have to, but performance is significantly better
// declare 64KB across two buffers for transfer
constexpr static const int lcd_buffer_size = 32 * 1024;
uint8_t lcd_buffer[lcd_buffer_size];

// the main screen
screen_t main_screen({960,540}, sizeof(lcd_buffer), lcd_buffer);

// the controls
label_t test_label(main_screen);
svg_box_t test_svg(main_screen);

// tell the lcd panel api to transfer data via DMA
static void uix_flush(const rect16& bounds, const void* bmp, void* state) {
    const_bitmap<typename screen_t::pixel_type> cbmp(bounds.dimensions(),bmp,main_screen.palette());
    draw::bitmap(epd,bounds,cbmp,cbmp.bounds());
    main_screen.flush_complete();
}
// initialize the screen and controls
void screen_init() {
    test_label.bounds(srect16(spoint16(0, 10), ssize16(200, 60)).center_horizontal(main_screen.bounds()));
    test_label.text_color(color32_t::blue);
    test_label.text_open_font(&text_font);
    test_label.text_line_height(45);
    test_label.text_justify(uix_justify::center);
    test_label.round_ratio(NAN);
    test_label.padding({8, 8});
    test_label.text("Hello!");
    // make the backcolor transparent
    auto bg = color32_t::black;
    bg.channel<channel_name::A>(0);
    test_label.background_color(bg);
    // and the border
    test_label.border_color(bg);

    test_svg.bounds(srect16(spoint16(0, 70), ssize16(60, 60)).center_horizontal(main_screen.bounds()));
    test_svg.doc(&bee_icon);
    main_screen.background_color(color_t::white);
    main_screen.register_control(test_label);
    main_screen.register_control(test_svg);
    main_screen.on_flush_callback(uix_flush);
}
// set up the hardware
void setup() {
    Serial.begin(115200);
    // dump the SVG just for display purposes
    Serial.write(bee_icon_data, sizeof(bee_icon_data));
    Serial.println();
    epd.initialize();
    epd.rotation(1);
    // initialize the buttons
    button_a.initialize();
    button_b.initialize();
    button_c.initialize();
    // init the UI screen
    screen_init();
    // set button callbacks
    button_a.on_pressed_changed([](bool pressed, void* state) {
        Serial.println("button_a");
        if (pressed) {
            test_label.text_color(color32_t::red);
        } else {
            test_label.text_color(color32_t::blue);
        }
    });
    button_b.on_pressed_changed([](bool pressed, void* state) {
        Serial.println("button_b");
        if (pressed) {
            main_screen.background_color(color_t::light_green);
        } else {
            main_screen.background_color(color_t::white);
        }
    });
    button_c.on_pressed_changed([](bool pressed, void* state) {
        Serial.println("button_c");
        if (pressed) {
            test_label.text_color(color32_t::red);
            main_screen.background_color(color_t::light_green);
        } else {
            main_screen.background_color(color_t::white);
            test_label.text_color(color32_t::blue);
        }
    });
}
// keep our stuff up to date and responsive
void loop() {
    button_a.update();
    button_b.update();
    button_c.update();
    // update the screen
    // e-paper should suspend 
    // the draw, and also 
    // potentially disable 
    // partial refresh 
    // (driver specific)
    bool dirty = main_screen.dirty();
    if(dirty) {
        gfx::draw::suspend(epd);
#ifndef PARTIAL_REFRESH
        epd.invalidate();
#endif
    }
    main_screen.update();
    if(dirty) {
        gfx::draw::resume(epd);
    }
}