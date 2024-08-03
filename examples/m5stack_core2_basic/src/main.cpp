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
// SVG converted to header using
// https://honeythecodewitch.com/gfx/converter
#define BEE_ICON_IMPLEMENTATION
#include <assets/bee_icon.hpp>
// downloaded from fontsquirrel.com and header generated with
// https://honeythecodewitch.com/gfx/generator
#define TELEGRAMA_IMPLEMENTATION
#include <assets/telegrama.hpp>
static const open_font& text_font = telegrama;

// for Core2 power management
m5core2_power power;
ft6336<320,280> touch(Wire1);

// declare the format of the screen
using screen_t = screen<rgb_pixel<16>>;
using color_t = color<typename screen_t::pixel_type>;
// for access to RGB8888 colors which controls use
using color32_t = color<rgba_pixel<32>>;

extern screen_t main_screen;

template<typename ControlSurfaceType>
class svg_box_touch : public svg_box<ControlSurfaceType> {
public:
    using base_type = svg_box<ControlSurfaceType>;
    using pixel_type = typename base_type::pixel_type;
    using palette_type = typename base_type::palette_type;
    svg_box_touch(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
        : base_type(parent, palette) {
    }
    svg_box_touch(svg_box_touch&& rhs) {
        do_move_control(rhs);
    }
    svg_box_touch& operator=(svg_box_touch&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    svg_box_touch(const svg_box_touch& rhs) {
        do_copy_control(rhs);
    }
    svg_box_touch& operator=(const svg_box_touch& rhs) {
        do_copy_control(rhs);
        return *this;
    }
    virtual bool on_touch(size_t locations_size,const spoint16* locations) override {
        main_screen.background_color(color_t::light_green);
        return true;
    }
    virtual void on_release() override {
        main_screen.background_color(color_t::white);
    }
};

// declare the control types to match the screen
using button_t = push_button<typename screen_t::control_surface_type>;
using svg_box_t = svg_box_touch<typename screen_t::control_surface_type>;

// UIX allows you to use two buffers for maximum DMA efficiency
// you don't have to, but performance is significantly better
// declare 64KB across two buffers for transfer
constexpr static const int lcd_buffer_size = 32 * 1024;
uint8_t lcd_buffer1[lcd_buffer_size];
uint8_t lcd_buffer2[lcd_buffer_size];
// this is the handle from the esp panel api
esp_lcd_panel_handle_t lcd_handle;

// the main screen
screen_t main_screen({LCD_HRES, LCD_VRES}, sizeof(lcd_buffer1), lcd_buffer1, lcd_buffer2);

// the controls
button_t test_button(main_screen);
svg_box_t test_svg(main_screen);

// tell UIX the DMA transfer is complete
static bool display_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t* edata, void* user_ctx) {
    main_screen.flush_complete();
    return true;
}
// tell the lcd panel api to transfer data via DMA
static void uix_on_flush(const rect16& bounds, const void* bmp, void* state) {
    int x1 = bounds.x1, y1 = bounds.y1, x2 = bounds.x2 + 1, y2 = bounds.y2 + 1;
    esp_lcd_panel_draw_bitmap(lcd_handle, x1, y1, x2, y2, (void*)bmp);
}
// report touch values to UIX
void uix_on_touch(point16* out_locations, 
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
void lcd_panel_init() {
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
// initialize the screen and controls
void screen_init() {
    const rgba_pixel<32> transparent(0,0,0,0);
    test_button.bounds(srect16(spoint16(0, 10), ssize16(200, 60)).center_horizontal(main_screen.bounds()));
    test_button.text_color(color32_t::blue,true);
    test_button.border_color(transparent,true);
    test_button.background_color(transparent,true);
    test_button.text_open_font(&text_font);
    test_button.text_line_height(45);
    test_button.text_justify(uix_justify::center);
    test_button.round_ratio(NAN);
    test_button.padding({8, 8});
    test_button.text("Hello!");
    test_button.on_pressed_changed_callback([](bool pressed, void *state ) {
        if(pressed) {
            test_button.text_color(color32_t::red,true);
        } else {
            test_button.text_color(color32_t::blue,true);
        }
    });
    test_svg.bounds(srect16(spoint16(0, 70), ssize16(60, 60)).center_horizontal(main_screen.bounds()));
    test_svg.doc(&bee_icon);
    main_screen.background_color(color_t::white);
    main_screen.register_control(test_button);
    main_screen.register_control(test_svg);
    main_screen.on_flush_callback(uix_on_flush);
    main_screen.on_touch_callback(uix_on_touch);
}
// set up the hardware
void setup() {
    Serial.begin(115200);
    // dump the SVG just for display purposes
    Serial.write(bee_icon_data, sizeof(bee_icon_data));
    Serial.println();
    power.initialize();
    touch.initialize();
    touch.rotation(0);
    // init the display
    lcd_panel_init();
    // init the UI screen
    screen_init();

}
// keep our stuff up to date and responsive
void loop() {
    // update the screen
    main_screen.update();
}