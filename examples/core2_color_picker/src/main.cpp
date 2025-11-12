//////////////////////////////////
// EXAMPLE
// Uses htcw_gfx with an esp32
// to demonstrate rendering SVGs,
// advanced canvas features, and
// DMA enabled draws
//////////////////////////////////

#if __has_include(<Arduino.h>)
#include <Arduino.h>
#else
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#endif
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <memory.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
// config for various devices
#include "lcd_config.h"
#ifdef M5STACK_CORE2
// cross platform i2c init codewitch-honey-crisis/htcw_esp_i2c
#include <esp_i2c.hpp>
// core2 power management codewitch-honey-crisis/htcw_m5core2_power
#include <m5core2_power.hpp>
#include <ft6336.hpp>
#endif
// graphics library codewitch-honey-crisis/htcw_gfx
#include <gfx.hpp>
#include <uix.hpp>
#define SVG_SVG_IMPLEMENTATION
#include "assets/svg_svg.h"
#define ARCHITECTS_DAUGHTER_IMPLEMENTATION
#include "assets/architects_daughter.h"
#define TERMINAL_IMPLEMENTATION
#include "assets/terminal.h"
// import the htcw_gfx graphics library namespace
using namespace gfx;
// import the htcw_uix UI/UX library namespace
using namespace uix;
// import the appropriate namespace for our other libraries
#ifdef ARDUINO
namespace arduino {}
using namespace arduino;
#else
namespace esp_idf {}
using namespace esp_idf;
#endif
#ifdef M5STACK_CORE2
// declare the power management driver
static m5core2_power power(esp_i2c<1, 21, 22>::instance);
static ft6336<320, 280> touch(esp_i2c<1,21,22>::instance);
#endif
// declare a bitmap type for our frame buffer type (RGB565 or rgb 16-bit color)
using fb_t = bitmap<rgb_pixel<16>>;
// get a color pseudo-enum for our bitmap type
using color_t = color<typename fb_t::pixel_type>;

// lcd data
// This works out to be 32KB - the max DMA transfer size
static const size_t lcd_transfer_buffer_size = 32*1024;
// for sending data to the display
static uint8_t *lcd_transfer_buffer = nullptr;
static uint8_t *lcd_transfer_buffer2 = nullptr;
// 0 = no flushes in progress, otherwise flushing
static esp_lcd_panel_handle_t lcd_handle = nullptr;

static uix::display disp;

// indicates the LCD DMA transfer is complete
static bool lcd_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                            esp_lcd_panel_io_event_data_t *edata,
                            void *user_ctx) {
    disp.flush_complete();
    return true;
}

// flush a bitmap to the display
static void uix_on_flush(const rect16& bounds,
                             const void *bitmap, void* state) {
    // adjust end coordinates for a quirk of Espressif's API (add 1 to each)
    esp_lcd_panel_draw_bitmap(lcd_handle, bounds.x1, bounds.y1, bounds.x2 + 1, bounds.y2 + 1,
                              (void *)bitmap);
}
// initialize the screen using the esp panel API
// htcw_gfx no longer has intrinsic display driver support
// for performance and flash size reasons
// here we use the ESP LCD Panel API for it
static void lcd_panel_init() {
#ifdef LCD_PIN_NUM_BCKL
    if(LCD_PIN_NUM_BCKL>-1) {
        gpio_set_direction((gpio_num_t)LCD_PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
        gpio_set_level((gpio_num_t)4, LCD_BCKL_OFF_LEVEL);
    }
#endif
    // configure the SPI bus
    spi_bus_config_t buscfg;
    memset(&buscfg, 0, sizeof(buscfg));
    buscfg.sclk_io_num = LCD_PIN_NUM_CLK;
    buscfg.mosi_io_num = LCD_PIN_NUM_MOSI;
    buscfg.miso_io_num = -1;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    // declare enough space for the transfer buffers + 8 bytes SPI DMA overhead
    buscfg.max_transfer_sz = lcd_transfer_buffer_size + 8;

    // Initialize the SPI bus on VSPI (SPI3)
    spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config;
    memset(&io_config, 0, sizeof(io_config));
    io_config.dc_gpio_num = LCD_PIN_NUM_DC;
    io_config.cs_gpio_num = LCD_PIN_NUM_CS;
    io_config.pclk_hz = LCD_PIXEL_CLOCK_HZ;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    io_config.spi_mode = 0;
    io_config.trans_queue_depth = 10;
    io_config.on_color_trans_done = lcd_flush_ready;
    // Attach the LCD to the SPI bus
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config,
                             &io_handle);

    lcd_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config;
    memset(&panel_config, 0, sizeof(panel_config));
    panel_config.reset_gpio_num = LCD_PIN_NUM_RST;
    if(LCD_COLOR_SPACE==ESP_LCD_COLOR_SPACE_RGB) {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        panel_config.rgb_endian = LCD_RGB_ENDIAN_RGB;
#else
        panel_config.color_space = ESP_LCD_COLOR_SPACE_RGB;
#endif
    } else {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        panel_config.rgb_endian = LCD_RGB_ENDIAN_BGR;
#else
        panel_config.color_space = ESP_LCD_COLOR_SPACE_BGR;
#endif
    }
    panel_config.bits_per_pixel = LCD_BIT_DEPTH;

    // Initialize the LCD configuration
    if (ESP_OK !=
        LCD_PANEL(io_handle, &panel_config, &lcd_handle)) {
        printf("Error initializing LCD panel.\n");
        while (1) vTaskDelay(5);
    }

    // Reset the display
    esp_lcd_panel_reset(lcd_handle);

    // Initialize LCD panel
    esp_lcd_panel_init(lcd_handle);
    //  Swap x and y axis (Different LCD screens may need different options)
    esp_lcd_panel_swap_xy(lcd_handle, LCD_SWAP_XY);
    esp_lcd_panel_set_gap(lcd_handle, LCD_GAP_X, LCD_GAP_Y);
    esp_lcd_panel_mirror(lcd_handle, LCD_MIRROR_X, LCD_MIRROR_Y);
    esp_lcd_panel_invert_color(lcd_handle, LCD_INVERT_COLOR);
    // Turn on the screen
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    esp_lcd_panel_disp_on_off(lcd_handle, true);
#else
    esp_lcd_panel_disp_off(lcd_handle, false);
#endif
#ifdef LCD_PIN_NUM_BCKL
    // Turn on backlight (Different LCD screens may need different levels)
    if(LCD_PIN_NUM_BCKL>-1) gpio_set_level((gpio_num_t)4, LCD_BCKL_ON_LEVEL);
#endif

    // initialize the transfer buffers
    lcd_transfer_buffer = (uint8_t *)malloc(lcd_transfer_buffer_size);
    if (lcd_transfer_buffer == nullptr) {
        puts("Out of memory initializing primary transfer buffer");
        while (1) vTaskDelay(5);
    }
    memset(lcd_transfer_buffer, 0, lcd_transfer_buffer_size);
    // initialize the transfer buffers
    lcd_transfer_buffer2 = (uint8_t *)malloc(lcd_transfer_buffer_size);
    if (lcd_transfer_buffer2 == nullptr) {
        puts("Out of memory initializing transfer buffer 2");
        while (1) vTaskDelay(5);
    }
    memset(lcd_transfer_buffer2, 0, lcd_transfer_buffer_size);
}

static void uix_on_touch(point16* out_locations,size_t* in_out_locations_size,void* state) {
    if(*in_out_locations_size>0) {
        uint16_t x,y;
        touch.update();
        bool pressed = touch.xy(&x,&y);
        if(pressed) {
            out_locations->x=x;
            out_locations->y=y;
            *in_out_locations_size = 1;
        } else {
            *in_out_locations_size = 0;
        }
    }
}

// the bitmap used to hold the texture
using fb_t = bitmap<rgb_pixel<16>>;
// prepare the ttf array into a stream
const_buffer_stream font_stm(architects_daughter,sizeof(architects_daughter));
tt_font text_fnt(font_stm,32,gfx::font_size_units::px,true);

using color_t = color<typename fb_t::pixel_type>;
using uix_color_t = color<rgba_pixel<32>>;
using screen_t = uix::screen<typename fb_t::pixel_type>;

using label_t = uix::label<typename screen_t::control_surface_type>;
using button_t = uix::vbutton<typename screen_t::control_surface_type>;
using switch_t = uix::vswitch<typename screen_t::control_surface_type>;
using slider_t = uix::vslider<typename screen_t::control_surface_type>;
using painter_t = uix::painter<typename screen_t::control_surface_type>;
static screen_t main_screen;
static slider_t red_slider, green_slider, blue_slider, gray_slider;
static label_t red_label, green_label, blue_label, gray_label, rgb_label, hsl_label;
static switch_t rgb_hsl_switch;
static button_t reset_button;
static painter_t color_painter;
static bool setting_gray = false;
void color_painter_on_paint(painter_t::control_surface_type& destination, const srect16& clip, void* state) { 
    draw::rectangle(destination,destination.bounds(),color_t::black);
    if(rgb_hsl_switch.value()) {
        hsl_pixel<24> px;
        px.channel<channel_name::H,channel_name::S,channel_name::L>(red_slider.value(),green_slider.value(),blue_slider.value());
        draw::filled_rectangle(destination,destination.bounds().inflate(-1,-1),px);    
    } else {
        screen_t::pixel_type px;
        px.channel<channel_name::R,channel_name::G,channel_name::B>(red_slider.value(),green_slider.value(),blue_slider.value());
        draw::filled_rectangle(destination,destination.bounds().inflate(-1,-1),px);
    }
}
void slider_on_value_chaged(uint16_t value,void* state) {
    color_painter.invalidate();
    setting_gray = true;
    if(rgb_hsl_switch.value()) {
        hsl_pixel<24> px;
        px.channel<channel_name::H,channel_name::S,channel_name::L>(red_slider.value(),green_slider.value(),blue_slider.value());
        gsc_pixel<8> gpx;
        convert(px,&gpx);
        gray_slider.value(gpx.channelr<channel_name::L>()*100);
    } else {
        screen_t::pixel_type px;
        px.channel<channel_name::R,channel_name::G,channel_name::B>(red_slider.value(),green_slider.value(),blue_slider.value());
        gsc_pixel<8> gpx;
        convert(px,&gpx);
        gray_slider.value(gpx.channelr<channel_name::L>()*100);
    }
    setting_gray = false;
}
void gray_slider_on_value_chaged(uint16_t value, void* state) {
    if(!setting_gray) {
        if(rgb_hsl_switch.value()) {
            hsl_pixel<24> px;
            gsc_pixel<8> gpx;
            gpx.channelr<channel_name::L>(((float)value)/100.0f);
            convert(gpx,&px);
            red_slider.value(px.channel<channel_name::H>());
            green_slider.value(px.channel<channel_name::S>());
            blue_slider.value(px.channel<channel_name::L>());
        } else {
            screen_t::pixel_type px;
            gsc_pixel<8> gpx;
            gpx.channelr<channel_name::L>(((float)value)/100.0f);
            convert(gpx,&px);
            red_slider.value(px.channel<channel_name::R>());
            green_slider.value(px.channel<channel_name::G>());
            blue_slider.value(px.channel<channel_name::B>());
        }
    }
}
void reset_button_on_pressed_changed(bool pressed,void*state) {
    if(!pressed) {
        red_slider.value(red_slider.minimum());
        green_slider.value(green_slider.minimum());
        blue_slider.value(blue_slider.minimum());
    }
}
void rgb_hsl_switch_on_value_chaged(bool value, void* state) {
    if(value) {
        screen_t::pixel_type px;
        hsl_pixel<24> dpx;
        px.channel<channel_name::R,channel_name::G,channel_name::B>(red_slider.value(),green_slider.value(),blue_slider.value());
        convert(px,&dpx);

        red_label.text("H");
        red_slider.color(uix_color_t::light_green);
        red_slider.knob_color(red_slider.color());
        red_slider.minimum(hsl_pixel<24>::channel_by_name<channel_name::H>::min);
        red_slider.maximum(hsl_pixel<24>::channel_by_name<channel_name::H>::max);
        red_slider.value(dpx.channel<channel_name::H>());

        green_label.text("S");
        green_slider.color(uix_color_t::light_coral);
        green_slider.knob_color(green_slider.color());
        green_slider.minimum(hsl_pixel<24>::channel_by_name<channel_name::S>::min);
        green_slider.maximum(hsl_pixel<24>::channel_by_name<channel_name::S>::max);
        green_slider.value(dpx.channel<channel_name::S>());
        
        blue_label.text("L");
        blue_slider.color(uix_color_t::cyan);
        blue_slider.knob_color(blue_slider.color());
        blue_slider.minimum(hsl_pixel<24>::channel_by_name<channel_name::L>::min);
        blue_slider.maximum(hsl_pixel<24>::channel_by_name<channel_name::L>::max);
        blue_slider.value(dpx.channel<channel_name::L>());

    } else {
        hsl_pixel<24> px;
        screen_t::pixel_type dpx;
        px.channel<channel_name::H,channel_name::S,channel_name::L>(red_slider.value(),green_slider.value(),blue_slider.value());        
        convert(px,&dpx);

        red_label.text("R");
        red_slider.color(uix_color_t::red);
        red_slider.knob_color(red_slider.color());
        red_slider.minimum(screen_t::pixel_type::channel_by_name<channel_name::R>::min);
        red_slider.maximum(screen_t::pixel_type::channel_by_name<channel_name::R>::max);
        red_slider.value(dpx.channel<channel_name::R>());
    
        green_label.text("G");
        green_slider.color(uix_color_t::green);
        green_slider.knob_color(green_slider.color());
        green_slider.minimum(screen_t::pixel_type::channel_by_name<channel_name::G>::min);
        green_slider.maximum(screen_t::pixel_type::channel_by_name<channel_name::G>::max);
        green_slider.value(dpx.channel<channel_name::G>());

        blue_label.text("B");
        blue_slider.color(uix_color_t::blue);
        blue_slider.knob_color(blue_slider.color());
        blue_slider.minimum(screen_t::pixel_type::channel_by_name<channel_name::B>::min);
        blue_slider.maximum(screen_t::pixel_type::channel_by_name<channel_name::B>::max);
        blue_slider.value(dpx.channel<channel_name::B>());
    
    }
}

#ifdef ARDUINO
// entry point (arduino)
void setup() {
    Serial.begin(115200);
#else
void loop();
static void loop_task(void* arg) {
    while(1) {
        static int count = 0;
        loop();
        // tickle the watchdog periodically
        if (count++ == 4) {
            count = 0;
            vTaskDelay(5);
        }
    }
}
// entry point (esp-idf)
extern "C" void app_main() {
#endif
#ifdef M5STACK_CORE2
    // initialize the AXP192 in the core 2
    power.initialize();
#endif
    // initialize the LCD
    lcd_panel_init();
    // start the touch
    touch.initialize();
    disp.buffer_size(lcd_transfer_buffer_size);
    disp.buffer1(lcd_transfer_buffer);
    disp.buffer2(lcd_transfer_buffer2);
    disp.on_flush_callback(uix_on_flush);
    disp.on_touch_callback(uix_on_touch);
    main_screen.dimensions({320,240});
    main_screen.background_color(color_t::white);
      text_info ti;
    ti.text_font = &text_fnt;
    ti.encoding = &text_encoding::utf8;
    ti.tab_width = 4;
    uint16_t w=0;
    size16 tsz;
    ti.text_sz("R");
    text_fnt.measure(uint16_t(-1),ti,&tsz);
    w=tsz.width;
    ti.text_sz("G");
    text_fnt.measure(uint16_t(-1),ti,&tsz);
    if(tsz.width>w) w = tsz.width;
    ti.text_sz("B");
    text_fnt.measure(uint16_t(-1),ti,&tsz);
    if(tsz.width>w) w = tsz.width;
    ti.text_sz("H");
    text_fnt.measure(uint16_t(-1),ti,&tsz);
    if(tsz.width>w) w = tsz.width;
    ti.text_sz("S");
    text_fnt.measure(uint16_t(-1),ti,&tsz);
    if(tsz.width>w) w = tsz.width;
    ti.text_sz("L");
    text_fnt.measure(uint16_t(-1),ti,&tsz);
    if(tsz.width>w) w = tsz.width;
    uint16_t gw;
    ti.text_sz("*");
    text_fnt.measure(uint16_t(-1),ti,&tsz);
    gw = tsz.width;
    uint16_t rgbw;
    ti.text_sz("RGB");
    text_fnt.measure(uint16_t(-1),ti,&tsz);
    rgbw = tsz.width;
    uint16_t hslw;
    ti.text_sz("HSL");
    text_fnt.measure(uint16_t(-1),ti,&tsz);
    hslw = tsz.width;

    srect16 b(0,0,159,31);
    b.center_horizontal_inplace(main_screen.bounds());
    b.offset_inplace(0,1+((b.height()+2)*3)/2);
    uint16_t x = b.x1;
    b.x1+=w+2;
    red_slider.bounds(b);
    red_slider.minimum(screen_t::pixel_type::channel_by_name<channel_name::R>::min);
    red_slider.maximum(screen_t::pixel_type::channel_by_name<channel_name::R>::max);
    red_slider.value(screen_t::pixel_type::channel_by_name<channel_name::R>::default_);
    red_slider.knob_shape(vslider_shape::rect);
    red_slider.border_color(uix_color_t::black);
    red_slider.color(uix_color_t::red);
    red_slider.knob_border_color(uix_color_t::black);
    red_slider.knob_color(uix_color_t::red);
    red_slider.on_value_changed_callback(slider_on_value_chaged);
    main_screen.register_control(red_slider);

    srect16 b2=srect16(0,0,w-1,text_fnt.line_height()+1);
    b2.offset_inplace(x,red_slider.bounds().y1-text_fnt.line_height()/4);
    red_label.bounds(b2);
    red_label.padding({0,0});
    red_label.font(text_fnt);
    red_label.text("R");
    red_label.color(uix_color_t::black);
    main_screen.register_control(red_label);

    b.offset_inplace(0,b.height()+1);
    green_slider.bounds(b);
    green_slider.minimum(screen_t::pixel_type::channel_by_name<channel_name::G>::min);
    green_slider.maximum(screen_t::pixel_type::channel_by_name<channel_name::G>::max);
    green_slider.value(screen_t::pixel_type::channel_by_name<channel_name::G>::default_);
    green_slider.knob_shape(vslider_shape::rect);
    green_slider.border_color(uix_color_t::black);
    green_slider.color(uix_color_t::green);
    green_slider.knob_border_color(uix_color_t::black);
    green_slider.knob_color(uix_color_t::green);
    green_slider.on_value_changed_callback(slider_on_value_chaged);
    main_screen.register_control(green_slider);

    b2=srect16(0,0,w-1,text_fnt.line_height()+1);
    b2.offset_inplace(x,green_slider.bounds().y1-text_fnt.line_height()/4);
    green_label.bounds(b2);
    green_label.padding({0,0});
    green_label.font(text_fnt);
    green_label.text("G");
    green_label.color(uix_color_t::black);
    main_screen.register_control(green_label);

    main_screen.register_control(red_label);
    b.offset_inplace(0,b.height()+1);
    blue_slider.bounds(b);
    blue_slider.minimum(screen_t::pixel_type::channel_by_name<channel_name::B>::min);
    blue_slider.maximum(screen_t::pixel_type::channel_by_name<channel_name::B>::max);
    blue_slider.value(screen_t::pixel_type::channel_by_name<channel_name::B>::default_);
    blue_slider.knob_shape(vslider_shape::rect);
    blue_slider.border_color(uix_color_t::black);
    blue_slider.color(uix_color_t::blue);
    blue_slider.knob_border_color(uix_color_t::black);
    blue_slider.knob_color(uix_color_t::blue);
    blue_slider.on_value_changed_callback(slider_on_value_chaged);
    main_screen.register_control(blue_slider);

    b2=srect16(0,0,w-1,text_fnt.line_height()+1);
    b2.offset_inplace(x,blue_slider.bounds().y1-text_fnt.line_height()/4);
    blue_label.bounds(b2);
    blue_label.padding({0,0});
    blue_label.font(text_fnt);
    blue_label.text("B");
    blue_label.color(uix_color_t::black);
    main_screen.register_control(blue_label);

    b=srect16(0,0,31,3*(red_slider.dimensions().height+2)-1);
    b.offset_inplace(x-32-5, red_slider.bounds().y1);
    gray_slider.bounds(b);
    gray_slider.orientation(uix_orientation::vertical);
    gray_slider.minimum(0);
    gray_slider.maximum(100);
    screen_t::pixel_type px;
    px.channel<channel_name::R, channel_name::G, channel_name::B>(
        red_slider.value(), blue_slider.value(), green_slider.value());
    gsc_pixel<8> gpx;
    convert(px,&gpx);
    gray_slider.value(gpx.channelr<channel_name::L>()*100);
    gray_slider.knob_shape(vslider_shape::rect);
    gray_slider.border_color(uix_color_t::black);
    gray_slider.color(uix_color_t::gray);
    gray_slider.knob_border_color(uix_color_t::black);
    gray_slider.knob_color(uix_color_t::gray);
    gray_slider.on_value_changed_callback(gray_slider_on_value_chaged);
    main_screen.register_control(gray_slider);

    b2=srect16(0,0,w-1,text_fnt.line_height()+1);
    b2.offset_inplace((gray_slider.dimensions().width-gw)/2+gray_slider.bounds().x1,gray_slider.bounds().y1-text_fnt.line_height()-2);
    gray_label.bounds(b2);
    gray_label.padding({0,0});
    gray_label.font(text_fnt);
    gray_label.text("*");
    gray_label.color(uix_color_t::black);
    main_screen.register_control(gray_label);

    b=srect16(0,0,47,23);
    b.center_horizontal_inplace(main_screen.bounds());
    b.offset_inplace(0,gray_slider.bounds().y2+10);
    rgb_hsl_switch.bounds(b);
    rgb_hsl_switch.radiuses({10,10});
    rgb_hsl_switch.background_color(uix_color_t::gray);
    rgb_hsl_switch.border_color(uix_color_t::black);
    rgb_hsl_switch.knob_color(uix_color_t::light_gray);
    rgb_hsl_switch.knob_border_color(uix_color_t::black);
    rgb_hsl_switch.knob_shape(vswitch_shape::circle);
    rgb_hsl_switch.on_value_changed_callback(rgb_hsl_switch_on_value_chaged);
    main_screen.register_control(rgb_hsl_switch);

    b2 = srect16(0,0,rgbw-1,text_fnt.line_height()+1);
    b2.offset_inplace(rgb_hsl_switch.bounds().x1-rgbw-2, (rgb_hsl_switch.dimensions().height- b2.height())/2+rgb_hsl_switch.bounds().y1-text_fnt.line_height()/4);
    rgb_label.bounds(b2);
    rgb_label.padding({0,0});
    rgb_label.font(text_fnt);
    rgb_label.text("RGB");
    rgb_label.color(uix_color_t::black);
    main_screen.register_control(rgb_label);

    b2 = srect16(0,0,hslw-1,text_fnt.line_height()+1);
    b2.offset_inplace(rgb_hsl_switch.bounds().x2+2, (rgb_hsl_switch.dimensions().height- b2.height())/2+rgb_hsl_switch.bounds().y1-text_fnt.line_height()/4);
    hsl_label.bounds(b2);
    hsl_label.padding({0,0});
    hsl_label.font(text_fnt);
    hsl_label.text("HSL");
    hsl_label.color(uix_color_t::black);
    main_screen.register_control(hsl_label);

    b=srect16(0,0,63,31).center_horizontal(main_screen.bounds()).offset(0,red_slider.bounds().y1-34);
    color_painter.bounds(b);
    color_painter.on_paint_callback(color_painter_on_paint);
    main_screen.register_control(color_painter);
    
    b=srect16(0,0,73,31).center_horizontal(main_screen.bounds()).offset(0,rgb_hsl_switch.bounds().x2+10);
    reset_button.bounds(b);
    reset_button.text("Reset");
    reset_button.background_color(uix_color_t::gray);
    reset_button.color(uix_color_t::black);
    reset_button.border_color(uix_color_t::black);
    reset_button.radiuses({10,10});
    reset_button.font(font_stm);
    reset_button.font_size(b.height()-4);
    reset_button.on_pressed_changed_callback(reset_button_on_pressed_changed);
    main_screen.register_control(reset_button);

    disp.active_screen(main_screen);
    
    
#ifndef ARDUINO
    TaskHandle_t loop_handle;
    xTaskCreate(loop_task,"loop_task",4096,nullptr,20,&loop_handle);
#endif
}
void loop() {
    disp.update();
}