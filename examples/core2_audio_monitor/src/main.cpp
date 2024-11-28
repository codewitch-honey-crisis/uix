// Core 2 Audio Monitor
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

#include <gfx.hpp>
#include <uix.hpp>

#include "i2s_sampler.hpp"
#include "lcd_config.h"
#include "processor.hpp"
#include "ui.hpp"

// these libs work with the ESP-IDF and Arduino
#include <esp_i2c.hpp>
#include <ft6336.hpp>
#include <m5core2_power.hpp>

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#    include <driver/i2s_pdm.h>
#else
#    include <driver/i2s.h>
#endif
#ifdef ARDUINO
using namespace arduino;
#else
using namespace esp_idf;
static uint32_t millis() {
    return ((uint32_t)pdTICKS_TO_MS(xTaskGetTickCount()));
}
#endif

using namespace gfx;
using namespace uix;

// lcd data
// This works out to be 32KB - the max DMA transfer size
static const size_t lcd_transfer_buffer_size = 32*1024;
// for sending data to the display
static uint8_t *lcd_transfer_buffer = nullptr;
static uint8_t *lcd_transfer_buffer2 = nullptr;
// 0 = no flushes in progress, otherwise flushing
static esp_lcd_panel_handle_t lcd_handle = nullptr;
static volatile int flushing = 0;
static uix::display disp;

static ft6336<320, 280> touch(esp_i2c<1,21,22>::instance);
static m5core2_power power(esp_i2c<1,21,22>::instance);


// indicates the LCD DMA transfer is complete
static bool lcd_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                            esp_lcd_panel_io_event_data_t *edata,
                            void *user_ctx) {
    disp.flush_complete();
    flushing = 0;
    return true;
}

// flush a bitmap to the display
static void uix_on_flush(const rect16& bounds,
                             const void *bitmap, void* state) {
    if(flushing) {
        puts("Flush too soon");
    }
    flushing=1;
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

// approx 30ms of audio @ 16KHz
#define WINDOW_SIZE 512
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
i2s_pdm_rx_config_t pdm_rx_cfg = {
    .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(64000),
    /* The default mono slot is the left slot (whose 'select pin' of the PDM microphone is pulled
       down) */
    .slot_cfg =
        {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = I2S_PDM_SLOT_RIGHT,
        },
    .gpio_cfg =
        {
            .clk = GPIO_NUM_0,
            .din = GPIO_NUM_34,
            .invert_flags =
                {
                    .clk_inv = false,
                },
        },
};
#else
// i2s config for reading from both m5stack mic
static i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate = 64000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 1024,
};

// i2s pins
static i2s_pin_config_t i2s_pins = {.mck_io_num = I2S_PIN_NO_CHANGE,
                                    .bck_io_num = GPIO_NUM_12,
                                    .ws_io_num = GPIO_NUM_0,
                                    .data_out_num = I2S_PIN_NO_CHANGE,
                                    .data_in_num = GPIO_NUM_34};
#endif
static i2s_sampler<WINDOW_SIZE> main_sampler;
static processor<WINDOW_SIZE> main_processor;
static TaskHandle_t processing_task_handle;
static TaskHandle_t drawing_task_handle;

// the screen/control definitions
screen_t main_screen;
analyzer_box_t main_analyzer;

static void processing_task(void *param);

static void processing_task(void *param) {
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
    while (true) {
        // wait for some samples to process
        uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
        if (ulNotificationValue > 0) {
            const int16_t *input = main_sampler.buffer();
            main_processor.update(input);
            main_analyzer.samples(main_processor.samples());
            main_analyzer.fft(main_processor.fft());
            xTaskNotify(processing_task_handle, 0, eSetValueWithOverwrite);
            xTaskNotify(drawing_task_handle, 1, eSetValueWithOverwrite);
        }
    }
}
static void drawing_task(void *param) {
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000);
    int frames = 0;
    uint32_t fps_ts = millis();
    unsigned long ms = 0;
    while (true) {
        // wait to be told to redraw
        uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
        if (ulNotificationValue != 0) {
            uint32_t start_ts = millis();
            main_analyzer.invalidate();
            disp.update();
            uint32_t end_ts = millis();
            ms += (end_ts - start_ts);
            ++frames;
            if (millis() >= fps_ts + 1000) {
                fps_ts = millis();
                if(frames==0) {
                    printf("FPS: < 1 / Avg: %d ms\n",(int)ms);    
                } else {
                    const int fps = roundf(1000.0f/((float)(ms/(float)frames)));
                    const int ms_avg = ms/frames;
                    frames = 0;
                    printf("FPS: %d / Avg: %dms\n",fps,(int)ms_avg);
                    
                }
                ms = 0;
            }
            xTaskNotify(drawing_task_handle, 0, eSetValueWithOverwrite);
        }
    }
}
#ifdef ARDUINO
void setup() {
    setCpuFrequencyMhz(240);
    Serial.begin(115200);
#else
void uix_yield_callback(void* state) {
    taskYIELD();
}
extern "C" void app_main() {
#endif
    power.initialize();
    power.lcd_voltage(3);
    lcd_panel_init();
    touch.initialize();
    disp.buffer_size(lcd_transfer_buffer_size);
    disp.buffer1(lcd_transfer_buffer);
    disp.buffer2(lcd_transfer_buffer2);
#ifndef ARDUINO
    disp.on_yield_callback(uix_yield_callback);
#endif
    disp.on_flush_callback(uix_on_flush);
    disp.on_touch_callback(uix_on_touch);
    main_screen.dimensions({320,240});
    main_screen.background_color(gfx::color<typename screen_t::pixel_type>::black);
    main_analyzer.bounds(main_screen.bounds());
    main_screen.register_control(main_analyzer);
    disp.active_screen(main_screen);

    // create a processing task to update the sample stream/fft
    xTaskCreatePinnedToCore(
        processing_task, "Processing Task", 1024, nullptr, 3, &processing_task_handle, 0);
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    main_sampler.initialize(I2S_NUM_0, chan_cfg, pdm_rx_cfg, processing_task_handle);
#else
    main_sampler.initialize(I2S_NUM_0, i2s_pins, i2s_config, processing_task_handle);
#endif
    // create a drawing task to update our UI
    // need 4096 words for printf
    xTaskCreatePinnedToCore(
        drawing_task, "Drawing Task", 4096, nullptr, 2, &drawing_task_handle, 1);
#ifndef ARDUINO
    // don't need this thread.
    vTaskDelete(NULL);
#endif
}

void loop() {
    // do nothing
}
