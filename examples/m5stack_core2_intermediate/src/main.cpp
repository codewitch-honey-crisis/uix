#include <Arduino.h>

#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include <esp_lcd_panel_ili9342.h>
#include <m5core2_power.hpp>
#define LCD_SPI_HOST    SPI3_HOST
#define LCD_DMA
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_PIN_NUM_MOSI 23
#define LCD_PIN_NUM_CLK 18
#define LCD_PIN_NUM_CS 5
#define LCD_PIN_NUM_DC 15
#define LCD_PANEL esp_lcd_new_panel_ili9342
#define LCD_HRES 320
#define LCD_VRES 240
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_BGR
#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define LCD_GAP_X 0
#define LCD_GAP_Y 0
#define LCD_MIRROR_X false
#define LCD_MIRROR_Y false
#define LCD_INVERT_COLOR true
#define LCD_SWAP_XY false
#include <ft6336.hpp>
#include <gfx.hpp>
#include <uix.hpp>
using namespace arduino;
using namespace gfx;
using namespace uix;
// downloaded from fontsquirrel.com and header generated with
// https://honeythecodewitch.com/gfx/generator
#define ARCHITECTS_DAUGHTER_IMPLEMENTATION
#include <assets/architects_daughter.hpp>
// downloaded for free from pngtree.com and header generated with
// https://honeythecodewitch.com/gfx/generator
#define CPU_PNGTREE_IMPLEMENTATION
#include <assets/cpu_pngtree.hpp>

static const open_font& text_font = architects_daughter;

// for Core2 power management
static m5core2_power power;
static ft6336<320,280> touch(Wire1);

// declare the format of the screen
using screen_t = screen<rgb_pixel<16>>;
using color_t = color<typename screen_t::pixel_type>;
// for access to RGB8888 colors which controls use
using color32_t = color<rgba_pixel<32>>;

static screen_t* active_screen;

extern screen_t main_screen;
extern screen_t anim_screen;

// custom image control that responds to touch events by changing the screen
template<typename ControlSurfaceType>
class image_touch : public image<ControlSurfaceType> {
public:
    using base_type = image<ControlSurfaceType>;
    using pixel_type = typename base_type::pixel_type;
    using palette_type = typename base_type::palette_type;
    image_touch(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
        : base_type(parent, palette) {
    }
    image_touch(image_touch&& rhs) {
        do_move_control(rhs);
    }
    image_touch& operator=(image_touch&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    image_touch(const image_touch& rhs) {
        do_copy_control(rhs);
    }
    image_touch& operator=(const image_touch& rhs) {
        do_copy_control(rhs);
        return *this;
    }
    virtual bool on_touch(size_t locations_size,const spoint16* locations) override {
        // tell UIX we handled the touch event
        return true;
    }
    virtual void on_release() override {
        // because of DMA, the current screen might still be flushing
        // it's dangerous to change screens during that process
        // make sure the main screen is done flushing
        while(main_screen.flushing()) {
            delay(10);
        }
        // when the finger is removed, change the screen
        active_screen=&anim_screen;
        // and force it to repaint
        active_screen->invalidate();
    }
};

// declare the control types to match the screen
using button_t = push_button<typename screen_t::control_surface_type>;
using image_touch_t = image_touch<typename screen_t::control_surface_type>;
using svg_t = svg_box<typename screen_t::control_surface_type>;
using canvas_t = canvas<typename screen_t::control_surface_type>;
// UIX allows you to use two buffers for maximum DMA efficiency
// you don't have to, but performance is significantly better
// declare 64KB across two buffers for transfer
constexpr static const int lcd_buffer_size = 32 * 1024;
static uint8_t lcd_buffer1[lcd_buffer_size];
static uint8_t lcd_buffer2[lcd_buffer_size];
// this is the handle from the esp panel api
static esp_lcd_panel_handle_t lcd_handle;

// the main screen
screen_t main_screen({LCD_HRES, LCD_VRES}, sizeof(lcd_buffer1), lcd_buffer1, lcd_buffer2);
screen_t anim_screen({LCD_HRES, LCD_VRES}, sizeof(lcd_buffer1), lcd_buffer1, lcd_buffer2);

// the controls
static button_t title_button(main_screen);
static image_touch_t cpu_image(main_screen);
static canvas_t anim_canvas(anim_screen);
// tell UIX the DMA transfer is complete
static bool display_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t* edata, void* user_ctx) {
    if(active_screen!=nullptr) {
        active_screen->flush_complete();
    }
    return true;
}
// tell the lcd panel api to transfer data via DMA
static void uix_on_flush(const rect16& bounds, const void* bmp, void* state) {
    int x1 = bounds.x1, y1 = bounds.y1, x2 = bounds.x2 + 1, y2 = bounds.y2 + 1;
    esp_lcd_panel_draw_bitmap(lcd_handle, x1, y1, x2, y2, (void*)bmp);
}
// report touch values to UIX
static void uix_on_touch(point16* out_locations, 
                    size_t* in_out_locations_size, 
                    void* state) {
    delay(1);
    if (touch.update()) {                                                  
        if (touch.xy(&out_locations[0].x, &out_locations[0].y)) {          
            if (*in_out_locations_size > 1) {                              
                *in_out_locations_size = 1;                                
                if (touch.xy2(&out_locations[1].x, &out_locations[1].y)) { 
                    *in_out_locations_size = 2;                            
                }                                                          
            } else {                                                       
                *in_out_locations_size = 1;                                
            }                                                              
        } else {                                                           
            *in_out_locations_size = 0;                                    
        }                                                                  
    }
}
// initialize the screen using the esp panel API
static void lcd_panel_init() {
#ifdef LCD_PIN_NUM_BCKL
    pinMode(LCD_PIN_NUM_BCKL, OUTPUT);
#endif
    spi_bus_config_t buscfg;
    memset(&buscfg, 0, sizeof(buscfg));
    buscfg.sclk_io_num = LCD_PIN_NUM_CLK;
    buscfg.mosi_io_num = LCD_PIN_NUM_MOSI;
    buscfg.miso_io_num = -1;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = sizeof(lcd_buffer1) + 8;

    // Initialize the SPI bus on VSPI (SPI3)
    spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config;
    memset(&io_config, 0, sizeof(io_config));
    io_config.dc_gpio_num = LCD_PIN_NUM_DC,
    io_config.cs_gpio_num = LCD_PIN_NUM_CS,
    io_config.pclk_hz = LCD_PIXEL_CLOCK_HZ,
    io_config.lcd_cmd_bits = 8,
    io_config.lcd_param_bits = 8,
    io_config.spi_mode = 0,
    io_config.trans_queue_depth = 10,
    io_config.on_color_trans_done = display_flush_ready;
    // Attach the LCD to the SPI bus
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI3_HOST, &io_config, &io_handle);

    lcd_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config;
    memset(&panel_config, 0, sizeof(panel_config));
#ifdef LCD_PIN_NUM_RST
    panel_config.reset_gpio_num = LCD_PIN_NUM_RST;
#else
    panel_config.reset_gpio_num = -1;
#endif
    panel_config.color_space = LCD_COLOR_SPACE;
    panel_config.bits_per_pixel = 16;

    // Initialize the LCD configuration
    esp_lcd_new_panel_ili9342(io_handle, &panel_config, &lcd_handle);

    // Turn off backlight to avoid unpredictable display on the LCD screen while initializing
    // the LCD panel driver. (Different LCD screens may need different levels)
#ifdef LCD_PIN_NUM_BCKL
    digitalWrite(LCD_PIN_NUM_BCKL, LCD_BCKL_OFF_LEVEL);
#endif
    // Reset the display
    esp_lcd_panel_reset(lcd_handle);

    // Initialize LCD panel
    esp_lcd_panel_init(lcd_handle);
    // esp_lcd_panel_io_tx_param(io_handle, LCD_CMD_SLPOUT, NULL, 0);
    //  Swap x and y axis (Different LCD screens may need different options)
    esp_lcd_panel_swap_xy(lcd_handle, LCD_SWAP_XY);
    esp_lcd_panel_set_gap(lcd_handle, LCD_GAP_X, LCD_GAP_Y);
    esp_lcd_panel_mirror(lcd_handle, LCD_MIRROR_X, LCD_MIRROR_Y);
    esp_lcd_panel_invert_color(lcd_handle, LCD_INVERT_COLOR);
    // Turn on the screen
    esp_lcd_panel_disp_off(lcd_handle, false);
    // Turn on backlight (Different LCD screens may need different levels)
#ifdef LCD_PIN_NUM_BCKL
    digitalWrite(LCD_PIN_NUM_BCKL, LCD_BCKL_ON_LEVEL);
#endif
}
// initialize the screens and controls
static void screens_init() {
    const rgba_pixel<32> transparent(0,0,0,0);
    title_button.bounds(srect16(spoint16(0, 10), ssize16(200, 60)).center_horizontal(main_screen.bounds()));
    title_button.text_color(color32_t::red,true);
    title_button.border_color(transparent,true);
    title_button.background_color(transparent,true);
    title_button.text_open_font(&text_font);
    title_button.text_line_height(45);
    title_button.text_justify(uix_justify::center);
    title_button.round_ratio(NAN);
    title_button.padding({8, 8});
    title_button.text("UIX / Core 2");
    title_button.on_pressed_changed_callback([](bool pressed, void *state ) {
        if(pressed) {
            title_button.text_color(color32_t::white,true);
        } else {
            title_button.text_color(color32_t::red,true);
        }
    });
    size16 img_sz;
    png_image::dimensions(&cpu_pngtree,&img_sz);
    cpu_image.bounds(srect16(spoint16(0, title_button.bounds().y2+1), (ssize16)img_sz).center_horizontal(main_screen.bounds()));
    cpu_image.stream(&cpu_pngtree);
    main_screen.background_color(color_t::black);
    main_screen.register_control(title_button);
    main_screen.register_control(cpu_image);
    main_screen.on_flush_callback(uix_on_flush);
    main_screen.on_touch_callback(uix_on_touch);

    anim_canvas.bounds(srect16(0,0,127,127).center(anim_screen.bounds()));
    anim_canvas.on_paint_callback([](canvas_t::control_surface_type& destination,const srect16& clip, void* state){
        // these are all kept beteen calls:
        constexpr static const size_t count = 10;
        static bool init=false;
        static spoint16 pts[count]; // locations
        static spoint16 dts[count]; // deltas
        // defined as screen_t's pixel type so 
        // a compile error will be thrown
        // further below if not RGB565
        static typename screen_t::pixel_type cls[count]; // colors
        // first time, we must initialize
        if(!init) {
            init = true;
            for(size_t i = 0;i<count;++i) {
                // start at the center
                pts[i]={63,63};
                dts[i]={0,0};
                // random deltas. Retry on (0,0)
                while(dts[i].x==0&&dts[i].y==0) {
                    dts[i].x=(rand()%5)-2;
                    dts[i].y=(rand()%5)-2;        
                }
                // random color RGB565
                cls[i]=rgb_pixel<16>((rand()%24)+8,(rand()%48)+16,(rand()%24)+8);
            }
        }
        for(size_t i = 0;i<count;++i) {
            spoint16& pt = pts[i];
            spoint16& d = dts[i];
            // throws compile error if screen_t is not RGB565
            rgb_pixel<16>& col = cls[i];
            draw::filled_ellipse(destination,srect16(pt,5),col,&clip);
            // move the circle
            pt.x+=d.x;
            pt.y+=d.y;
            // if it is about to hit the edge, invert 
            // the respective deltas
            if(pt.x+d.x+-5<=0 || pt.x+d.x+5>=destination.bounds().x2) {
                d.x=-d.x;
            } 
            if(pt.y+d.y+-5<=0 || pt.y+d.y+5>=destination.bounds().y2) {
                d.y=-d.y;
            }
        }
    });
    anim_screen.register_control(anim_canvas);
    anim_screen.background_color(color_t::black);
    anim_screen.on_flush_callback(uix_on_flush);
    anim_screen.on_touch_callback(uix_on_touch);
}
// set up the hardware
void setup() {
    Serial.begin(115200);
    power.initialize();
    touch.initialize();
    touch.rotation(0);
    // init the display
    lcd_panel_init();
    // init the UI screen
    screens_init();
    active_screen = &main_screen;
}
// keep our stuff up to date and responsive
void loop() {
    // if we're on the animation screen
    if(active_screen==&anim_screen) {
        // every 10th of a second
        static uint32_t anim_ts = 0;
        if(millis()>anim_ts+100) {
            // force the canvas to repaint
            anim_canvas.invalidate();
        }
    }
    if(active_screen!=nullptr) {
        // update the screen
        active_screen->update();
    }
}