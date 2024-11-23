// use spans for fire portion to increase performance
#define USE_SPANS
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

#include "lcd_config.h"

#include <gfx.hpp>
#include <uix.hpp>

#define ARCHITECTS_DAUGHTER_IMPLEMENTATION
#include "assets/architects_daughter.h"

// these libs work with the ESP-IDF and Arduino
#ifdef M5STACK_CORE2
#include <esp_i2c.hpp>
#include <ft6336.hpp>
#include <m5core2_power.hpp>
#endif

#ifdef TTGO_T1
#include <button.hpp>
#define PIN_BUTTON_A 35
#define PIN_BUTTON_B 0
#endif
#ifdef M5STACK_FIRE
#include <button.hpp>
#define PIN_BUTTON_A 39
#define PIN_BUTTON_B 38
#define PIN_BUTTON_C 37
#endif
#ifdef ARDUINO
namespace arduino {}
using namespace arduino;
#else
namespace esp_idf {}
using namespace esp_idf;
static uint32_t millis() {
    return ((uint32_t)pdTICKS_TO_MS(xTaskGetTickCount()));
}
#endif


using namespace gfx;
using namespace uix;

// lcd data
static const size_t lcd_transfer_buffer_size = 16*1024;
// for sending data to the display
static uint8_t *lcd_transfer_buffer = nullptr;
static uint8_t *lcd_transfer_buffer2 = nullptr;
// 0 = no flushes in progress, otherwise flushing
static esp_lcd_panel_handle_t lcd_handle = nullptr;

//static uix::display disp;

// declare the format of the screen
using screen_t = screen<rgb_pixel<LCD_BIT_DEPTH>>;

// the main screen
static screen_t anim_screen;


#ifdef M5STACK_CORE2
static ft6336<320, 280> touch(esp_i2c<1,21,22>::instance);
static m5core2_power power(esp_i2c<1,21,22>::instance);
#endif
#ifdef TTGO_T1
using button_t = multi_button;
static basic_button button_a_raw(PIN_BUTTON_A, 10, true);
static basic_button button_b_raw(PIN_BUTTON_B, 10, true);
static button_t button_a(button_a_raw);
static button_t button_b(button_b_raw);
#endif
#ifdef M5STACK_FIRE
using button_t = multi_button;
static basic_button button_a_raw(PIN_BUTTON_A, 10, true);
static basic_button button_b_raw(PIN_BUTTON_B, 10, true);
static basic_button button_c_raw(PIN_BUTTON_C, 10, true);
static button_t button_a(button_a_raw);
static button_t button_b(button_b_raw);
static button_t button_c(button_c_raw);
#endif

// indicates the LCD DMA transfer is complete
static bool lcd_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                            esp_lcd_panel_io_event_data_t *edata,
                            void *user_ctx) {
    anim_screen.flush_complete();
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
#ifdef M5STACK_CORE2
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
#endif
// fire stuff
#define V_WIDTH (LCD_WIDTH / 4)
#define V_HEIGHT (LCD_HEIGHT / 4)
#define BUF_WIDTH (LCD_WIDTH / 4)
#define BUF_HEIGHT ((LCD_HEIGHT / 4) + 6)
#define PALETTE_SIZE (256 * 3)
#define INT_SIZE 2
#ifdef USE_SPANS
#define PAL_TYPE uint16_t
// store preswapped uint16_ts for performance
#define RGB(r,g,b) (bits::swap(rgb_pixel<LCD_BIT_DEPTH>(r,g,b)))
#else
// store rgb_pixel<16> instances
#define PAL_TYPE rgb_pixel<LCD_BIT_DEPTH>
#define RGB(r,g,b) rgb_pixel<LCD_BIT_DEPTH>(r,g,b)
#endif

// color palette for flames
static PAL_TYPE fire_palette[] = {
    RGB(0, 0, 0),    RGB(0, 0, 3),    RGB(0, 0, 3),
    RGB(0, 0, 3),    RGB(0, 0, 4),    RGB(0, 0, 4),
    RGB(0, 0, 4),    RGB(0, 0, 5),    RGB(1, 0, 5),
    RGB(2, 0, 4),    RGB(3, 0, 4),    RGB(4, 0, 4),
    RGB(5, 0, 3),    RGB(6, 0, 3),    RGB(7, 0, 3),
    RGB(8, 0, 2),    RGB(9, 0, 2),    RGB(10, 0, 2),
    RGB(11, 0, 2),   RGB(12, 0, 1),   RGB(13, 0, 1),
    RGB(14, 0, 1),   RGB(15, 0, 0),   RGB(16, 0, 0),
    RGB(16, 0, 0),   RGB(16, 0, 0),   RGB(17, 0, 0),
    RGB(17, 0, 0),   RGB(18, 0, 0),   RGB(18, 0, 0),
    RGB(18, 0, 0),   RGB(19, 0, 0),   RGB(19, 0, 0),
    RGB(20, 0, 0),   RGB(20, 0, 0),   RGB(20, 0, 0),
    RGB(21, 0, 0),   RGB(21, 0, 0),   RGB(22, 0, 0),
    RGB(22, 0, 0),   RGB(23, 1, 0),   RGB(23, 1, 0),
    RGB(24, 2, 0),   RGB(24, 2, 0),   RGB(25, 3, 0),
    RGB(25, 3, 0),   RGB(26, 4, 0),   RGB(26, 4, 0),
    RGB(27, 5, 0),   RGB(27, 5, 0),   RGB(28, 6, 0),
    RGB(28, 6, 0),   RGB(29, 7, 0),   RGB(29, 7, 0),
    RGB(30, 8, 0),   RGB(30, 8, 0),   RGB(31, 9, 0),
    RGB(31, 9, 0),   RGB(31, 10, 0),  RGB(31, 10, 0),
    RGB(31, 11, 0),  RGB(31, 11, 0),  RGB(31, 12, 0),
    RGB(31, 12, 0),  RGB(31, 13, 0),  RGB(31, 13, 0),
    RGB(31, 14, 0),  RGB(31, 14, 0),  RGB(31, 15, 0),
    RGB(31, 15, 0),  RGB(31, 16, 0),  RGB(31, 16, 0),
    RGB(31, 17, 0),  RGB(31, 17, 0),  RGB(31, 18, 0),
    RGB(31, 18, 0),  RGB(31, 19, 0),  RGB(31, 19, 0),
    RGB(31, 20, 0),  RGB(31, 20, 0),  RGB(31, 21, 0),
    RGB(31, 21, 0),  RGB(31, 22, 0),  RGB(31, 22, 0),
    RGB(31, 23, 0),  RGB(31, 24, 0),  RGB(31, 24, 0),
    RGB(31, 25, 0),  RGB(31, 25, 0),  RGB(31, 26, 0),
    RGB(31, 26, 0),  RGB(31, 27, 0),  RGB(31, 27, 0),
    RGB(31, 28, 0),  RGB(31, 28, 0),  RGB(31, 29, 0),
    RGB(31, 29, 0),  RGB(31, 30, 0),  RGB(31, 30, 0),
    RGB(31, 31, 0),  RGB(31, 31, 0),  RGB(31, 32, 0),
    RGB(31, 32, 0),  RGB(31, 33, 0),  RGB(31, 33, 0),
    RGB(31, 34, 0),  RGB(31, 34, 0),  RGB(31, 35, 0),
    RGB(31, 35, 0),  RGB(31, 36, 0),  RGB(31, 36, 0),
    RGB(31, 37, 0),  RGB(31, 38, 0),  RGB(31, 38, 0),
    RGB(31, 39, 0),  RGB(31, 39, 0),  RGB(31, 40, 0),
    RGB(31, 40, 0),  RGB(31, 41, 0),  RGB(31, 41, 0),
    RGB(31, 42, 0),  RGB(31, 42, 0),  RGB(31, 43, 0),
    RGB(31, 43, 0),  RGB(31, 44, 0),  RGB(31, 44, 0),
    RGB(31, 45, 0),  RGB(31, 45, 0),  RGB(31, 46, 0),
    RGB(31, 46, 0),  RGB(31, 47, 0),  RGB(31, 47, 0),
    RGB(31, 48, 0),  RGB(31, 48, 0),  RGB(31, 49, 0),
    RGB(31, 49, 0),  RGB(31, 50, 0),  RGB(31, 50, 0),
    RGB(31, 51, 0),  RGB(31, 52, 0),  RGB(31, 52, 0),
    RGB(31, 52, 0),  RGB(31, 52, 0),  RGB(31, 52, 0),
    RGB(31, 53, 0),  RGB(31, 53, 0),  RGB(31, 53, 0),
    RGB(31, 53, 0),  RGB(31, 54, 0),  RGB(31, 54, 0),
    RGB(31, 54, 0),  RGB(31, 54, 0),  RGB(31, 54, 0),
    RGB(31, 55, 0),  RGB(31, 55, 0),  RGB(31, 55, 0),
    RGB(31, 55, 0),  RGB(31, 56, 0),  RGB(31, 56, 0),
    RGB(31, 56, 0),  RGB(31, 56, 0),  RGB(31, 57, 0),
    RGB(31, 57, 0),  RGB(31, 57, 0),  RGB(31, 57, 0),
    RGB(31, 57, 0),  RGB(31, 58, 0),  RGB(31, 58, 0),
    RGB(31, 58, 0),  RGB(31, 58, 0),  RGB(31, 59, 0),
    RGB(31, 59, 0),  RGB(31, 59, 0),  RGB(31, 59, 0),
    RGB(31, 60, 0),  RGB(31, 60, 0),  RGB(31, 60, 0),
    RGB(31, 60, 0),  RGB(31, 60, 0),  RGB(31, 61, 0),
    RGB(31, 61, 0),  RGB(31, 61, 0),  RGB(31, 61, 0),
    RGB(31, 62, 0),  RGB(31, 62, 0),  RGB(31, 62, 0),
    RGB(31, 62, 0),  RGB(31, 63, 0),  RGB(31, 63, 0),
    RGB(31, 63, 1),  RGB(31, 63, 1),  RGB(31, 63, 2),
    RGB(31, 63, 2),  RGB(31, 63, 3),  RGB(31, 63, 3),
    RGB(31, 63, 4),  RGB(31, 63, 4),  RGB(31, 63, 5),
    RGB(31, 63, 5),  RGB(31, 63, 5),  RGB(31, 63, 6),
    RGB(31, 63, 6),  RGB(31, 63, 7),  RGB(31, 63, 7),
    RGB(31, 63, 8),  RGB(31, 63, 8),  RGB(31, 63, 9),
    RGB(31, 63, 9),  RGB(31, 63, 10), RGB(31, 63, 10),
    RGB(31, 63, 10), RGB(31, 63, 11), RGB(31, 63, 11),
    RGB(31, 63, 12), RGB(31, 63, 12), RGB(31, 63, 13),
    RGB(31, 63, 13), RGB(31, 63, 14), RGB(31, 63, 14),
    RGB(31, 63, 15), RGB(31, 63, 15), RGB(31, 63, 15),
    RGB(31, 63, 16), RGB(31, 63, 16), RGB(31, 63, 17),
    RGB(31, 63, 17), RGB(31, 63, 18), RGB(31, 63, 18),
    RGB(31, 63, 19), RGB(31, 63, 19), RGB(31, 63, 20),
    RGB(31, 63, 20), RGB(31, 63, 21), RGB(31, 63, 21),
    RGB(31, 63, 21), RGB(31, 63, 22), RGB(31, 63, 22),
    RGB(31, 63, 23), RGB(31, 63, 23), RGB(31, 63, 24),
    RGB(31, 63, 24), RGB(31, 63, 25), RGB(31, 63, 25),
    RGB(31, 63, 26), RGB(31, 63, 26), RGB(31, 63, 26),
    RGB(31, 63, 27), RGB(31, 63, 27), RGB(31, 63, 28),
    RGB(31, 63, 28), RGB(31, 63, 29), RGB(31, 63, 29),
    RGB(31, 63, 30), RGB(31, 63, 30), RGB(31, 63, 31),
    RGB(31, 63, 31)};


// for access to colors in the native screen's format
using color_t = color<typename screen_t::pixel_type>;
// for access to RGB8888 colors which controls use
using color32_t = color<rgba_pixel<32>>;

// prepare the ttf array into a stream
const_buffer_stream font_stm(architects_daughter,sizeof(architects_daughter));
// load a font with it, size 40px, and initialize it
tt_font fps_fnt(font_stm,40,gfx::font_size_units::px,true);
// load a font with it. We replace this later, so the data isn't important
tt_font summary_fnt(font_stm,1);

using label_t = label<typename screen_t::control_surface_type>;
static label_t fps;
static label_t summaries[] = {
    label_t(),
    label_t(),
    label_t(),
    label_t(),
    label_t(),
    label_t()};

template <typename ControlSurfaceType>
class fire_box : public control<ControlSurfaceType> {
    int draw_state = 0;
    uint8_t p1[BUF_HEIGHT][BUF_WIDTH];  // VGA buffer, quarter resolution w/extra lines
    unsigned int i, j, k, l, delta;     // looping variables, counters, and data
    char ch;

   public:
    using control_surface_type = ControlSurfaceType;
    using base_type = control<control_surface_type>;
    using pixel_type = typename base_type::pixel_type;
    using palette_type = typename base_type::palette_type;
    fire_box(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
        : base_type(parent, palette) {
    }
    fire_box()
        : base_type() {
    }
    fire_box(fire_box&& rhs) {
        do_move_control(rhs);
        draw_state = 0;
    }
    fire_box& operator=(fire_box&& rhs) {
        do_move_control(rhs);
        draw_state = 0;
        return *this;
    }
    fire_box(const fire_box& rhs) {
        do_copy_control(rhs);
        draw_state = 0;
    }
    fire_box& operator=(const fire_box& rhs) {
        do_copy_control(rhs);
        draw_state = 0;
        return *this;
    }
protected:
    virtual void on_before_paint() override {
        switch (draw_state) {
            case 0:
                // Initialize the buffer to 0s
                for (i = 0; i < BUF_HEIGHT; i++) {
                    for (j = 0; j < BUF_WIDTH; j++) {
                        p1[i][j] = 0;
                    }
                }
                draw_state = 1;
                // fall through
            case 1:
                // Transform current buffer
                for (i = 1; i < BUF_HEIGHT; ++i) {
                    for (j = 0; j < BUF_WIDTH; ++j) {
                        if (j == 0)
                            p1[i - 1][j] = (p1[i][j] +
                                            p1[i - 1][BUF_WIDTH - 1] +
                                            p1[i][j + 1] +
                                            p1[i + 1][j]) >>
                                           2;
                        else if (j == 79)
                            p1[i - 1][j] = (p1[i][j] +
                                            p1[i][j - 1] +
                                            p1[i + 1][0] +
                                            p1[i + 1][j]) >>
                                           2;
                        else
                            p1[i - 1][j] = (p1[i][j] +
                                            p1[i][j - 1] +
                                            p1[i][j + 1] +
                                            p1[i + 1][j]) >>
                                           2;

                        if (p1[i][j] > 11)
                            p1[i][j] = p1[i][j] - 12;
                        else if (p1[i][j] > 3)
                            p1[i][j] = p1[i][j] - 4;
                        else {
                            if (p1[i][j] > 0) p1[i][j]--;
                            if (p1[i][j] > 0) p1[i][j]--;
                            if (p1[i][j] > 0) p1[i][j]--;
                        }
                    }
                }
                delta = 0;
                for (j = 0; j < BUF_WIDTH; j++) {
                    if (rand() % 10 < 5) {
                        delta = (rand() & 1) * 255;
                    }
                    p1[BUF_HEIGHT - 2][j] = delta;
                    p1[BUF_HEIGHT - 1][j] = delta;
                }
        }
    }
    virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
        for (int y = clip.y1; y <= clip.y2; ++y) {
#ifdef USE_SPANS
        // must use rgb_pixel<16>
        static_assert(gfx::helpers::is_same<rgb_pixel<16>,typename screen_t::pixel_type>::value,"USE_SPANS only works with RGB565");
        // get the spans for the current row
        gfx_span row = destination.span(point16(clip.x1,y));
        // get the pointer
        uint16_t *prow = (uint16_t*)row.data;
#endif

            for (int x = clip.x1; x <= clip.x2; ++x) {
                int i = y >> 2;
                int j = x >> 2;
                PAL_TYPE px = fire_palette[p1[i][j]];
 #ifdef USE_SPANS
                *(prow++)=px;
#else
                destination.point(point16(x, y), px);
#endif               
                
            }
        }
    }
    virtual bool on_touch(size_t locations_size, const spoint16* locations) override {
        fps.visible(true);
        return true;
    }
    virtual void on_release() override {
        fps.visible(false);
    }
};
using fire_box_t = fire_box<typename screen_t::control_surface_type>;

template <typename ControlSurfaceType>
class alpha_box : public control<ControlSurfaceType> {
    constexpr static const int horizontal_alignment = 32;
    constexpr static const int vertical_alignment = 32;
    int draw_state = 0;
    constexpr static const size_t count = 10;
    constexpr static const int16_t radius = 25;
    spoint16 pts[count];        // locations
    spoint16 dts[count];        // deltas
    rgba_pixel<32> cls[count];  // colors
    template <typename T>
    constexpr static T h_align_up(T value) {
        if (value % horizontal_alignment != 0)
            value += (T)(horizontal_alignment - value % horizontal_alignment);
        return value;
    }
    template <typename T>
    constexpr static T h_align_down(T value) {
        value -= value % horizontal_alignment;
        return value;
    }
    template <typename T>
    constexpr static T v_align_up(T value) {
        if (value % vertical_alignment != 0)
            value += (T)(vertical_alignment - value % vertical_alignment);
        return value;
    }
    template <typename T>
    constexpr static T v_align_down(T value) {
        value -= value % vertical_alignment;
        return value;
    }
    constexpr static srect16 align(const srect16& value) {
        int x2 = h_align_up(value.x2);
        if (horizontal_alignment != 1) {
            --x2;
        }
        int y2 = v_align_up(value.y2);
        if (vertical_alignment != 1) {
            --y2;
        }
        return srect16(h_align_down(value.x1), v_align_down(value.y1), x2, y2);
    }

   public:
    using control_surface_type = ControlSurfaceType;
    using base_type = control<control_surface_type>;
    using pixel_type = typename base_type::pixel_type;
    using palette_type = typename base_type::palette_type;
    alpha_box(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
        : base_type(parent, palette) {
    }
    alpha_box()
        : base_type() {
    }
    alpha_box(alpha_box&& rhs) {
        do_move_control(rhs);
        draw_state = 0;
    }
    alpha_box& operator=(alpha_box&& rhs) {
        do_move_control(rhs);
        draw_state = 0;
        return *this;
    }
    alpha_box(const alpha_box& rhs) {
        do_copy_control(rhs);
        draw_state = 0;
    }
    alpha_box& operator=(const alpha_box& rhs) {
        do_copy_control(rhs);
        draw_state = 0;
        return *this;
    }
protected:
    virtual void on_before_paint() override {
        switch (draw_state) {
            case 0:
                for (size_t i = 0; i < count; ++i) {
                    // start at the center
                    pts[i] = spoint16(this->dimensions().width / 2, this->dimensions().height / 2);
                    dts[i] = {0, 0};
                    // random deltas. Retry on (dx=0||dy=0)
                    while (dts[i].x == 0 || dts[i].y == 0) {
                        dts[i].x = (rand() % 5) - 2;
                        dts[i].y = (rand() % 5) - 2;
                    }
                    // random color RGBA8888
                    cls[i] = rgba_pixel<32>((rand() % 255), (rand() % 255), (rand() % 255), (rand() % 224) + 32);
                }
                draw_state = 1;
                // fall through
        }
    }
    virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
        srect16 clip_align = align(clip);
        for (int y = clip_align.y1; y <= clip_align.y2; y += vertical_alignment) {
            for (int x = clip_align.x1; x <= clip_align.x2; x += horizontal_alignment) {
                rect16 r(x, y, x + horizontal_alignment, y + vertical_alignment);
                bool w = ((x + y) % (horizontal_alignment + vertical_alignment));
                if (w) {
                    destination.fill(r, color_t::white);
                }
            }
        }
        // draw the circles
        for (size_t i = 0; i < count; ++i) {
            spoint16& pt = pts[i];
            spoint16& d = dts[i];
            srect16 r(pt, radius);
            if (clip.intersects(r)) {
                rgba_pixel<32>& col = cls[i];
                draw::filled_ellipse(destination, r, col, &clip);
            }
            // move the circle
            pt.x += d.x;
            pt.y += d.y;
            // if it is about to hit the edge, invert
            // the respective deltas
            if (pt.x + d.x + -radius <= 0 || pt.x + d.x + radius >= destination.bounds().x2) {
                d.x = -d.x;
            }
            if (pt.y + d.y + -radius <= 0 || pt.y + d.y + radius >= destination.bounds().y2) {
                d.y = -d.y;
            }
        }
    }
    virtual bool on_touch(size_t locations_size, const spoint16* locations) override {
        fps.visible(true);
        return true;
    }
    virtual void on_release() override {
        fps.visible(false);
    }
};
using alpha_box_t = alpha_box<typename screen_t::control_surface_type>;

template <typename ControlSurfaceType>
class plaid_box : public control<ControlSurfaceType> {
    int draw_state = 0;
    constexpr static const size_t count = 10;
    constexpr static const int16_t width = 25;
    spoint16 pts[count];        // locations
    spoint16 dts[count];        // deltas
    rgba_pixel<32> cls[count];  // colors

   public:
    using control_surface_type = ControlSurfaceType;
    using base_type = control<control_surface_type>;
    using pixel_type = typename base_type::pixel_type;
    using palette_type = typename base_type::palette_type;
    plaid_box(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
        : base_type(parent, palette) {
    }
    plaid_box()
        : base_type() {
    }
    plaid_box(plaid_box&& rhs) {
        do_move_control(rhs);
        draw_state = 0;
    }
    plaid_box& operator=(plaid_box&& rhs) {
        do_move_control(rhs);
        draw_state = 0;
        return *this;
    }
    plaid_box(const plaid_box& rhs) {
        do_copy_control(rhs);
        draw_state = 0;
    }
    plaid_box& operator=(const plaid_box& rhs) {
        do_copy_control(rhs);
        draw_state = 0;
        return *this;
    }
protected:
    virtual void on_before_paint() override {
        switch (draw_state) {
            case 0:
                size_t i;
                for (i = 0; i < count; ++i) {
                    if ((i & 1)) {
                        pts[i] = spoint16(0, (rand() % (this->dimensions().height - width)) + width / 2);
                        dts[i] = {0, 0};
                        // random deltas. Retry on (dy=0)
                        while (dts[i].y == 0) {
                            dts[i].y = (rand() % 5) - 2;
                        }
                    } else {
                        pts[i] = spoint16((rand() % (this->dimensions().width - width)) + width / 2, 0);
                        dts[i] = {0, 0};
                        // random deltas. Retry on (dx=0)
                        while (dts[i].x == 0) {
                            dts[i].x = (rand() % 5) - 2;
                        }
                    }
                    // random color RGBA8888
                    cls[i] = rgba_pixel<32>((rand() % 255), (rand() % 255), (rand() % 255), (rand() % 224) + 32);
                }
                draw_state = 1;
                // fall through
        }
    }
    virtual void on_after_paint() override {
        switch (draw_state) {
            case 0:
                break;
            case 1:
                for (size_t i = 0; i < count; ++i) {
                    spoint16& pt = pts[i];
                    spoint16& d = dts[i];
                    // move the bar
                    pt.x += d.x;
                    pt.y += d.y;
                    // if it is about to hit the edge, invert
                    // the respective deltas
                    if (pt.x + d.x + -width / 2 < 0 || pt.x + d.x + width / 2 > this->bounds().x2) {
                        d.x = -d.x;
                    }
                    if (pt.y + d.y + -width / 2 < 0 || pt.y + d.y + width / 2 > this->bounds().y2) {
                        d.y = -d.y;
                    }
                }
                break;
        }
    }
    virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
        // draw the bars
        for (size_t i = 0; i < count; ++i) {
            spoint16& pt = pts[i];
            spoint16& d = dts[i];
            srect16 r;
            if (d.y == 0) {
                r = srect16(pt.x - width / 2, 0, pt.x + width / 2, this->bounds().y2);
            } else {
                r = srect16(0, pt.y - width / 2, this->bounds().x2, pt.y + width / 2);
            }
            if (clip.intersects(r)) {
                rgba_pixel<32>& col = cls[i];
                draw::filled_rectangle(destination, r, col, &clip);
            }
        }
    }
    virtual bool on_touch(size_t locations_size, const spoint16* locations) override {
        fps.visible(true);
        return true;
    }
    virtual void on_release() override {
        fps.visible(false);
    }
};
using plaid_box_t = plaid_box<screen_t::control_surface_type>;


// the controls
static fire_box_t fire;
static alpha_box_t alpha;
static plaid_box_t plaid;
// use caches to make FPS label draw faster
static font_measure_cache fps_measure_cache;
static font_draw_cache fps_draw_cache;
// initialize the screens and controls
static void screen_init() {
    anim_screen.dimensions({LCD_WIDTH,LCD_HEIGHT});

    const rgba_pixel<32> transparent(0, 0, 0, 0);
    alpha.bounds(anim_screen.bounds());
    fire.bounds(anim_screen.bounds());
    fire.visible(false);
    plaid.bounds(anim_screen.bounds());
    plaid.visible(false);
    // since codepoint pairs are stored
    // may need more entries than the
    // draw cache
    fps_measure_cache.max_entries(32);
    fps_draw_cache.max_entries(16);
    fps_measure_cache.initialize();
    fps_draw_cache.initialize();
    fps.measure_cache(fps_measure_cache);
    fps.draw_cache(fps_draw_cache);
    fps.color(color32_t::red);
    fps.font(fps_fnt);
    fps.padding({0, 0});
    rgba_pixel<32> bg(0, 0, 0, 224);
    fps.text_justify(uix_justify::bottom_right);
    fps.bounds(srect16(0, anim_screen.bounds().y2 - fps_fnt.line_height() + 2, anim_screen.bounds().x2, anim_screen.bounds().y2));
    fps.visible(false);
    int y = 0;
    int lh = anim_screen.dimensions().height / 6 - 2;
    // reestablish and initialize the summary_fnt
    summary_fnt = tt_font(font_stm,lh,font_size_units::px,true);
    for (size_t i = 0; i < (sizeof(summaries) / sizeof(label_t)); ++i) {
        label_t& summary = summaries[i];
        summary.padding({0, 0});
        summary.bounds(srect16(0, y, anim_screen.bounds().x2, y + lh + 2));
        summary.font(summary_fnt);
        summary.color(color32_t::red);
        summary.text_justify(uix_justify::top_left);
        summary.visible(false);
        y += lh + 2;
    }
    anim_screen.register_control(alpha);
    anim_screen.register_control(fire);
    anim_screen.register_control(plaid);
    anim_screen.register_control(fps);
    for (size_t i = 0; i < (sizeof(summaries) / sizeof(label_t)); ++i) {
        label_t& summary = summaries[i];
        anim_screen.register_control(summary);
    }
    anim_screen.background_color(color_t::black);
    
}
#if __has_include(<htcw_button.hpp>)
static void buttons_on_pressed_changed(bool pressed, void* state) {
    fps.visible(pressed);
}
#endif

#ifdef ARDUINO
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
extern "C" void app_main() {
#endif
#ifdef M5STACK_CORE2
    power.initialize();
    touch.initialize();
    touch.rotation(0);
#endif
#ifdef TTGO_T1
    button_a.initialize();
    button_b.initialize();
    button_a.on_pressed_changed(buttons_on_pressed_changed);
    button_b.on_pressed_changed(buttons_on_pressed_changed);
#endif
#if defined(M5STACK_FIRE)
    button_a.initialize();
    button_b.initialize();
    button_c.initialize();
    button_a.on_pressed_changed(buttons_on_pressed_changed);
    button_b.on_pressed_changed(buttons_on_pressed_changed);
    button_c.on_pressed_changed(buttons_on_pressed_changed);
#endif
    lcd_panel_init();
    anim_screen.buffer_size(lcd_transfer_buffer_size);
    anim_screen.buffer1(lcd_transfer_buffer);
    anim_screen.buffer2(lcd_transfer_buffer2);
    anim_screen.on_flush_callback(uix_on_flush);
#ifdef M5STACK_CORE2
    anim_screen.on_touch_callback(uix_on_touch);
#endif
    screen_init();
    //disp.active_screen(anim_screen);
#ifndef ARDUINO
    TaskHandle_t loop_handle;
    xTaskCreate(loop_task,"loop_task",4096,nullptr,20,&loop_handle);
#endif
}

void loop() {
#ifdef TTGO_T1
    button_a.update();
    button_b.update();
#endif
#if defined(M5STACK_FIRE) 
    button_a.update();
    button_b.update();
    button_c.update();
#endif
    constexpr static const int run_seconds = 5;
    constexpr static const int summary_seconds = 10;
    static char szsummaries[sizeof(summaries) / sizeof(label_t)][128];
    static int seconds = 0;
    static int frames = 0;
    static int total_frames_alpha = 0;
    static int total_frames_fire = 0;
    static int total_frames_plaid = 0;
    static int fps_alpha[run_seconds];
    static int fps_fire[run_seconds];
    static int fps_plaid[run_seconds];
    static bool showed_summary = false;
    static int fps_index = 0;
    static char szfps[32];
    static uint32_t fps_ts = 0;
    uint32_t ms = millis();

    ++frames;

    if (ms > fps_ts + 1000) {
        fps_ts = ms;
        snprintf(szfps, sizeof(szfps), "fps: %d", frames);
#ifdef ARDUINO
        Serial.println(szfps);
#else
        printf(szfps);
        putchar('\n');
#endif
        fps.text(szfps);
        if (alpha.visible()) {
            fps_alpha[fps_index++] = frames;
        } else if (fire.visible()) {
            fps_fire[fps_index++] = frames;
        } else if (plaid.visible()) {
            fps_plaid[fps_index++] = frames;
        }
        frames = 0;
        ++seconds;
    }
    if (alpha.visible()) {
        alpha.invalidate();
        ++total_frames_alpha;
    } else if (fire.visible()) {
        fire.invalidate();
        ++total_frames_fire;
    } else if (plaid.visible()) {
        plaid.invalidate();
        ++total_frames_plaid;
    }
    if (seconds == (run_seconds * 1)) {
        alpha.visible(false);
        fire.visible(true);
        plaid.visible(false);
        for (size_t i = 0; i < (sizeof(summaries) / sizeof(label_t)); ++i) {
            label_t& summary = summaries[i];
            summary.visible(false);
        }
        fps_index = 0;
    } else if (seconds == (run_seconds * 2)) {
        alpha.visible(false);
        fire.visible(false);
        plaid.visible(true);
        for (size_t i = 0; i < (sizeof(summaries) / sizeof(label_t)); ++i) {
            label_t& summary = summaries[i];
            summary.visible(false);
        }
        fps_index = 0;
    } else if (seconds == (run_seconds * 3)) {

        if (!showed_summary) {
            showed_summary = true;
            alpha.visible(false);
            fire.visible(false);
            plaid.visible(false);
            for (size_t i = 0; i < (sizeof(summaries) / sizeof(label_t)); ++i) {
                label_t& summary = summaries[i];
                summary.visible(true);
            }
            int alpha_fps_max = 0, alpha_fps_sum = 0;
            for (size_t i = 0; i < run_seconds; ++i) {
                int fps = fps_alpha[i];
                if (fps > alpha_fps_max) {
                    alpha_fps_max = fps;
                }
                alpha_fps_sum += fps;
            }
            int fire_fps_max = 0, fire_fps_sum = 0;
            for (size_t i = 0; i < run_seconds; ++i) {
                int fps = fps_fire[i];
                if (fps > fire_fps_max) {
                    fire_fps_max = fps;
                }
                fire_fps_sum += fps;
            }
            int plaid_fps_max = 0, plaid_fps_sum = 0;
            for (size_t i = 0; i < run_seconds; ++i) {
                int fps = fps_plaid[i];
                if (fps > plaid_fps_max) {
                    plaid_fps_max = fps;
                }
                plaid_fps_sum += fps;
            }
            strcpy(szsummaries[0], "alpha max/avg/frames");
#ifdef ARDUINO
            Serial.println(szsummaries[0]);
#else
            printf(szsummaries[0]);
            putchar('\n');
#endif
            summaries[0].text(szsummaries[0]);
            sprintf(szsummaries[1], "%d/%d/%d", alpha_fps_max,
                    (int)roundf((float)alpha_fps_sum / (float)run_seconds),
                    total_frames_alpha);
#ifdef ARDUINO
            Serial.println(szsummaries[1]);
#else
            printf(szsummaries[1]);
            putchar('\n');
#endif
            summaries[1].text(szsummaries[1]);
            strcpy(szsummaries[2], "fire max/avg/frames");
#ifdef ARDUINO
            Serial.println(szsummaries[2]);
#else
            printf(szsummaries[2]);
            putchar('\n');
#endif
            summaries[2].text(szsummaries[2]);
            sprintf(szsummaries[3], "%d/%d/%d", fire_fps_max,
                    (int)roundf((float)fire_fps_sum / (float)run_seconds),
                    total_frames_fire);
            summaries[3].text(szsummaries[3]);
#ifdef ARDUINO
            Serial.println(szsummaries[3]);
#else
            printf(szsummaries[3]);
            putchar('\n');
#endif
            strcpy(szsummaries[4], "plaid max/avg/frames");
            summaries[4].text(szsummaries[4]);
#ifdef ARDUINO
            Serial.println(szsummaries[4]);
#else
            printf(szsummaries[4]);
            putchar('\n');
#endif
            sprintf(szsummaries[5], "%d/%d/%d", plaid_fps_max,
                    (int)roundf((float)plaid_fps_sum / (float)run_seconds),
                    total_frames_plaid);
#ifdef ARDUINO
            Serial.println(szsummaries[5]);
#else
            printf(szsummaries[5]);
            putchar('\n');
#endif
            summaries[5].text(szsummaries[5]);
            fps_index = 0;
        }
    } else if (seconds >= (run_seconds * 3) + summary_seconds) {
        seconds = 0;
        total_frames_alpha = 0;
        total_frames_fire = 0;
        total_frames_plaid = 0;
        alpha.visible(true);
        fire.visible(false);
        plaid.visible(false);
        for (size_t i = 0; i < (sizeof(summaries) / sizeof(label_t)); ++i) {
            label_t& summary = summaries[i];
            summary.visible(false);
        }
        fps_index = 0;
        showed_summary = false;
    }

    anim_screen.update();
}
