#ifndef HTCW_UIX_BARCODE_HPP
#define HTCW_UIX_BARCODE_HPP
#include <uix_core.hpp>
namespace uix {
    enum struct barcode_parity {
        odd = 0,
        even = 1
    };
    namespace helpers {
        /* Derived from https://github.com/ryankurte/EAN13/tree/master
        The MIT License (MIT)
        Copyright (c) 2015 Ryan

        Permission is hereby granted, free of charge, to any person obtaining a copy
        of this software and associated documentation files (the "Software"), to deal
        in the Software without restriction, including without limitation the rights
        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
        copies of the Software, and to permit persons to whom the Software is
        furnished to do so, subject to the following conditions:

        The above copyright notice and this permission notice shall be included in all
        copies or substantial portions of the Software.

        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
        IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
        FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
        LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
        OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
        SOFTWARE.
        */
        template<bool Dummy>
        class ean13 {
            static constexpr const bool dummy = Dummy;
            // #define EAN13_WIDTH   (sizeof(quiet_zone)*2+sizeof(lead_trailer)*2+sizeof(separator)+EAN13_MODULE*EAN13_DIGITS)
            static constexpr const int EAN13_MODULE = 7;
            static constexpr const int EAN13_DIGITS = 12;
            static constexpr const int EAN13_WIDTH = 13;
            static constexpr const int PARITY_ODD_ = 0;
            static constexpr const int PARITY_EVEN_ = 1;
            static constexpr const bool quiet_zone[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
            static constexpr const bool lead_trailer[] = {1, 0, 1};
            static constexpr const bool separator[] = {0, 1, 0, 1, 0};

            static constexpr const bool modules_odd_left[10][7] = {
                {0, 0, 0, 1, 1, 0, 1}, {0, 0, 1, 1, 0, 0, 1}, {0, 0, 1, 0, 0, 1, 1}, {0, 1, 1, 1, 1, 0, 1},
                {0, 1, 0, 0, 0, 1, 1}, {0, 1, 1, 0, 0, 0, 1}, {0, 1, 0, 1, 1, 1, 1}, {0, 1, 1, 1, 0, 1, 1},
                {0, 1, 1, 0, 1, 1, 1}, {0, 0, 0, 1, 0, 1, 1}
            };

            static constexpr const bool modules_even_left[10][7] = {
                {0, 1, 0, 0, 1, 1, 1}, {0, 1, 1, 0, 0, 1, 1}, {0, 0, 1, 1, 0, 1, 1}, {0, 1, 0, 0, 0, 0, 1},
                {0, 0, 1, 1, 1, 0, 1}, {0, 1, 1, 1, 0, 0, 1}, {0, 0, 0, 0, 1, 0, 1}, {0, 0, 1, 0, 0, 0, 1},
                {0, 0, 0, 1, 0, 0, 1}, {0, 0, 1, 0, 1, 1, 1}
            };

            static constexpr const bool modules_right[10][7] = {
                {1, 1, 1, 0, 0, 1, 0}, {1, 1, 0, 0, 1, 1, 0}, {1, 1, 0, 1, 1, 0, 0}, {1, 0, 0, 0, 0, 1, 0},
                {1, 0, 1, 1, 1, 0, 0}, {1, 0, 0, 1, 1, 1, 0}, {1, 0, 1, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0},
                {1, 0, 0, 1, 0, 0, 0}, {1, 1, 1, 0, 1, 0, 0}
            };

            static constexpr const bool parities[10][5] = {
                {0, 0, 0, 0, 0}, {0, 1, 0, 1, 1}, {0, 1, 1, 0, 1}, {0, 1, 1, 1, 0}, {1, 0, 0, 1, 1},
                {1, 1, 0, 0, 1}, {1, 1, 1, 0, 0}, {1, 0, 1, 0, 1}, {1, 0, 1, 1, 0}, {1, 1, 0, 1, 0}
            };
            static void write_n(bool *dest, const bool *src, int n)
            {
                int i;
                for (i = 0; i < n; i++) {
                    dest[i] = src[i];
                }
            }  
        public:
            static bool build(const char *data, bool *code) {
                int index = 0;
                int num = 0;
                int start = 0;
                int i;

                //Check input
                for(i=0; i<EAN13_DIGITS; i++) {
                    if((data[i] < 0x30) || (data[i] > 0x39)) {
                        return false;
                    }
                }

                //Clear code
                for (i = 0; i < EAN13_WIDTH; i++) {
                    code[i] = 0;
                }

                //Create leading quiet zone
                write_n(code + index, quiet_zone, sizeof(quiet_zone));
                index += sizeof(quiet_zone);

                //Create leading trailer (porch)
                write_n(code + index, lead_trailer, sizeof(lead_trailer));
                index += sizeof(lead_trailer);

                //Left hand data here
                start = data[0] - 0x30;

                //First char is a special case
                num = data[1] - 0x30;
                write_n(code + index, modules_odd_left[num], EAN13_MODULE);
                index += EAN13_MODULE;

                //Write left hand data
                for (i = 2; i < 7; i++) {
                    num = data[i] - 0x30;
                    if (parities[start][i - 2] == 0) {
                        write_n(code + index, modules_odd_left[num], EAN13_MODULE);
                    } else {
                        write_n(code + index, modules_even_left[num], EAN13_MODULE);
                    }
                    index += EAN13_MODULE;
                }

                //Create center separator
                write_n(code + index, separator, sizeof(separator));
                index += sizeof(separator);

                //Right hand data here
                for (i = 7; i < 12; i++) {
                    num = data[i] - 0x30;
                    write_n(code + index, modules_right[num], 7);
                    index += EAN13_MODULE;
                }

                //Calculate Checksum
                int odds = 0;
                int evens = 0;
                for (int i = 0; i < 12; i++) {
                    if (i % 2 == 0) {
                        evens += data[i] - 0x30;
                    } else {
                        odds += data[i] - 0x30;
                    }
                }

                //Write checksum
                int checksum = 10 - (((odds * 3) + evens) % 10);
                write_n(code + index, modules_right[checksum], EAN13_MODULE);
                index += EAN13_MODULE;

                //Following trailer
                write_n(code + index, lead_trailer, sizeof(lead_trailer));
                index += sizeof(lead_trailer);

                //Trailing quiet zone here
                write_n(code + index, quiet_zone, sizeof(quiet_zone));
                index += sizeof(quiet_zone);

                return true;
            }
            static constexpr size_t size() {
                return (sizeof(quiet_zone)*2+sizeof(lead_trailer)*2+sizeof(separator)+EAN13_MODULE*EAN13_DIGITS);
            }
        }; 
    }
    template<typename ControlSurfaceType>
    class barcode : public control<ControlSurfaceType> {
        using base_type = control<ControlSurfaceType>;
        constexpr static const gfx::rgba_pixel<32> color_black = gfx::rgba_pixel<32>(0,0,0,255);
        constexpr static const gfx::rgba_pixel<32> color_transparent = gfx::rgba_pixel<32>(0,0,0,0);
        const char* m_text;
        gfx::rgba_pixel<32> m_background_color;
        gfx::rgba_pixel<32> m_color;
        bool m_dirty;
        bool m_data[helpers::ean13<false>::size()];
        void recompute() {
            if(helpers::ean13<false>::build(text(),m_data)) {
                m_dirty = false;
            }
        }
    public:
        using control_surface_type = ControlSurfaceType;
        using pixel_type = typename base_type::pixel_type;
        using palette_type = typename base_type::palette_type;
        /// @brief Constructs a new instance of the barcode
        /// @param parent The parent screen
        /// @param palette The palette, if any
        barcode(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
        : base_type(parent, palette),m_text(nullptr), m_dirty(true) {
            m_color = color_black;
            m_background_color = color_transparent;
        }
        /// @brief Constructs a new instance of the barcode
        barcode()
            : base_type(), m_text(nullptr), m_dirty(true) {
            m_color = color_black;
            m_background_color = color_transparent;
        }
        barcode(barcode&& rhs) {
            this->do_move_control(rhs);
        }
        barcode& operator=(barcode&& rhs) {
            this->do_move_control(rhs);
            return *this;
        }
        barcode(const barcode& rhs) {
            this->do_copy_control(rhs);
        }
        barcode& operator=(const barcode& rhs) {
            this->do_copy_control(rhs);
            return *this;
        }
        /// @brief Computes the minumum size needed to display this control
        ssize16 compute_min_dimensions() {
            return {helpers::ean13<false>::size(),5};
        }
        /// @brief Indicates the raw barcode text
        /// @return The text of the barcode
        const char* text() const {
            return m_text;
        }
        /// @brief Sets the text of the bar code
        /// @param value 
        /// @details The text must be 13 characters and integer numeric
        void text(const char* value) {
            m_text=value;
            m_dirty = true;
            this->invalidate();
        }
        /// @brief Indicates the color of the qrcode
        /// @return The RGBA8888 color
        gfx::rgba_pixel<32> color() const {
            return m_color;
        }
        /// @brief Sets the color of the qrcode
        /// @param value The RGBA8888 color
        void color(gfx::rgba_pixel<32> value) {
            m_color = value;
            this->invalidate();
        }
        /// @brief Indicates the background color of the qrcode
        /// @return The RGBA8888 color
        gfx::rgba_pixel<32> background_color() const {
            return m_background_color;
        }
        /// @brief Sets the color of the qrcode
        /// @param value The RGBA8888 color
        void background_color(gfx::rgba_pixel<32> value) {
            m_background_color = value;
            this->invalidate();
        }
    protected:
        void do_move_control(barcode& rhs) {
            this->base_type::do_move_control(rhs);
            m_text = rhs.m_text;
            m_dirty = true;
            m_color = rhs.m_color;
            m_background_color = rhs.m_background_color;
            
        }
        void do_copy_control(const barcode& rhs) {
            this->base_type::do_copy_control(rhs);
            m_text = rhs.m_text;
            m_dirty = true;
            m_color = rhs.m_color;
            m_background_color = rhs.m_background_color;
            
        }
        virtual void on_before_paint() override {
            if(m_dirty) {
                recompute();
            }
        }
        virtual void on_after_resize() override {
            m_dirty = true;
        }
        virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
            // if can't draw for some reason, dirty will be true
            if(m_dirty) {
                return;
            }
            pixel_type bgpx;
            bool draw_bg = false;
            pixel_type px;
            if(m_background_color.opacity()>0.f) {
                if(gfx::gfx_result::success!=gfx::convert_palette_from(destination,m_background_color,&bgpx)) {
                    return;
                }
                draw_bg=true;
            }
            if(gfx::gfx_result::success!=gfx::convert_palette_from(destination,m_color,&px)) {
                return;
            }
            const int sz = helpers::ean13<false>::size()<destination.dimensions().width?(int)helpers::ean13<false>::size():(int)destination.dimensions().width;
            int ww = destination.dimensions().width;
            int w = ww/sz;
            int h = destination.dimensions().height;
            if(w<1) w=1;
            int x = 0;
            for(int i = 0;i<ww;++i) {
                if(m_data[i] && i<=sz) {
                    destination.fill(rect16(point16(x,0),size16(w,h)),px);
                } else if(draw_bg) {
                    destination.fill(rect16(point16(x,0),size16(w,h)),bgpx);
                }
                x+=w;
            }
        }
    
    };
}
#endif
