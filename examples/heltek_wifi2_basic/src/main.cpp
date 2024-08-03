#include <Arduino.h>

#include "driver/i2c.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include <ssd1306_surface_adapter.hpp>
#define LCD_I2C_HOST    0
#define LCD_DMA
#define LCD_I2C_ADDR 0x3C
#define LCD_CONTROL_PHASE_BYTES 1
#define LCD_DC_BIT_OFFSET 6
#define LCD_BIT_DEPTH 1
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_PIN_NUM_SCL 15
#define LCD_PIN_NUM_SDA 4
#define LCD_PIN_NUM_RST 16
#define LCD_PANEL esp_lcd_new_panel_ssd1306
#define LCD_HRES 128
#define LCD_VRES 64
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_MONOCHROME
#define LCD_PIXEL_CLOCK_HZ (400 * 1000)
#define LCD_GAP_X 0
#define LCD_GAP_Y 0
#define LCD_MIRROR_X true
#define LCD_MIRROR_Y true
#define LCD_INVERT_COLOR false
#define LCD_SWAP_XY false
#define PIN_BUTTON_A 0
#include <button.hpp>
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

// declare the format of the screen
using screen_t = screen_ex<ssd1306_surface_adapter,1,8>;
// declare the control types to match the screen
using label_t = label<typename screen_t::control_surface_type>;
using svg_box_t = svg_box<typename screen_t::control_surface_type>;
using color_t = color<typename screen_t::pixel_type>;
// for access to RGB8888 colors which controls use
using color32_t = color<rgba_pixel<32>>;
// declare the PRG button
using button_a_t = int_button<PIN_BUTTON_A, 10, true>;

// UIX allows you to use two buffers for maximum DMA efficiency
// you don't have to, but performance is significantly better
// declare 2KB across two buffers for transfer
constexpr static const int lcd_buffer_size = 1 * 1024;
uint8_t lcd_buffer1[lcd_buffer_size];
uint8_t lcd_buffer2[lcd_buffer_size];
// this is the handle from the esp panel api
esp_lcd_panel_handle_t lcd_handle;

// the PRG button
button_a_t button_a;

// the main screen
screen_t main_screen({LCD_HRES, LCD_VRES}, sizeof(lcd_buffer1), lcd_buffer1, lcd_buffer2);

// the controls
label_t test_label(main_screen);
svg_box_t test_svg(main_screen);

// tell UIX the DMA transfer is complete
static bool display_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t* edata, void* user_ctx) {
    main_screen.flush_complete();
    return true;
}
// tell the lcd panel api to transfer data via DMA
static void uix_flush(const rect16& bounds, const void* bmp, void* state) {
    int x1 = bounds.x1, y1 = bounds.y1, x2 = bounds.x2 + 1, y2 = bounds.y2 + 1;
    esp_lcd_panel_draw_bitmap(lcd_handle, x1, y1, x2, y2, (void*)bmp);
}
// initialize the screen using the esp panel API
void lcd_panel_init() {
#ifdef LCD_PIN_NUM_BCKL
    pinMode(LCD_PIN_NUM_BCKL, OUTPUT);
#endif
    i2c_config_t i2c_conf;
    memset(&i2c_conf,0,sizeof(i2c_config_t));
    
    i2c_conf.mode = I2C_MODE_MASTER,
    i2c_conf.sda_io_num = LCD_PIN_NUM_SDA;
    i2c_conf.scl_io_num = LCD_PIN_NUM_SCL;
    i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.master.clk_speed = LCD_PIXEL_CLOCK_HZ;
    
    i2c_param_config(LCD_I2C_HOST, &i2c_conf);
    i2c_driver_install(LCD_I2C_HOST, I2C_MODE_MASTER, 0, 0, 0);

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config;
    memset(&io_config,0,sizeof(esp_lcd_panel_io_i2c_config_t));
    io_config.dev_addr = LCD_I2C_ADDR;
#ifdef LCD_CONTROL_PHASE_BYTES
    io_config.control_phase_bytes = LCD_CONTROL_PHASE_BYTES;
#else
    io_config.control_phase_bytes = 0;
#endif
    io_config.lcd_cmd_bits = 8;   
    io_config.lcd_param_bits = 8; 
    io_config.on_color_trans_done = display_flush_ready;
    io_config.dc_bit_offset = LCD_DC_BIT_OFFSET;  
#if defined(LCD_ENABLE_CONTROL_PHASE) && LCD_ENABLE_CONTROL_PHASE != 0
    io_config.flags.disable_control_phase = 1;
#endif
    esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)LCD_I2C_HOST, &io_config, &io_handle);
    lcd_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config;
    memset(&panel_config, 0, sizeof(panel_config));
#ifdef LCD_PIN_NUM_RST
    panel_config.reset_gpio_num = LCD_PIN_NUM_RST;
#else
    panel_config.reset_gpio_num = -1;
#endif
    panel_config.color_space = LCD_COLOR_SPACE;
    panel_config.bits_per_pixel = 1;

    // Initialize the LCD configuration
    esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &lcd_handle);

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
    test_label.bounds(main_screen.bounds());
    test_label.text_color(color32_t::white);
    test_label.text_open_font(&text_font);
    test_label.text_line_height(32);
    test_label.text_justify(uix_justify::center);
    test_label.round_ratio(NAN);
    test_label.padding({0, 0});
    test_label.text("Hello!");
    test_label.background_color(transparent);
    test_label.border_color(transparent);

    test_svg.bounds(srect16(spoint16(0, 0), ssize16(60, 60)).center(main_screen.bounds()));
    test_svg.doc(&bee_icon);
    test_svg.visible(false);
    main_screen.background_color(color_t::black);
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
    // init the display
    lcd_panel_init();
    // init the UI screen
    screen_init();
    // init the buttons
    button_a.initialize();
    // set button callbacks
    button_a.on_pressed_changed([](bool pressed, void* state) {
        Serial.println("button_a");
        test_label.visible(!pressed);
        test_svg.visible(pressed);
    });
}
// keep our stuff up to date and responsive
void loop() {
    // update the buttons
    button_a.update();
    // update the screen
    main_screen.update();
}