#ifndef HTCW_UIX_PUSH_BUTTON_HPP
#define HTCW_UIX_PUSH_BUTTON_HPP
#include "uix_core.hpp"
namespace uix {
    /// @brief Represents a push button control
    /// @tparam ControlSurfaceType The type of control surface, usually taken from the screen
    template<typename ControlSurfaceType>
    class push_button final : public control<ControlSurfaceType> {
    public:
        using type = push_button;
        using base_type = control<ControlSurfaceType>;
        using pixel_type = typename ControlSurfaceType::pixel_type;
        using palette_type = typename ControlSurfaceType::palette_type;
        using control_surface_type = ControlSurfaceType;
        // the callback type when the button has been pressed or released
        typedef void(*on_pressed_changed_callback_type)(bool pressed,void* state);
    private:
        bool m_pressed;
        on_pressed_changed_callback_type m_on_pressed_changed_callback;
        void* m_on_pressed_changed_callback_state;
        float m_round_ratio;
        ssize16 m_padding;
        const char* m_text;
        const gfx::open_font* m_ofnt;
        const gfx::vlw_font* m_vfnt;
        const gfx::font* m_fnt;
        size_t m_text_line_height;
        uix_justify m_text_justify;
        uix_encoding m_text_encoding;
        srect16 m_text_rect;
        gfx::rgba_pixel<32> m_background_color, m_border_color,m_text_color;
        gfx::rgba_pixel<32> m_pressed_background_color, m_pressed_border_color,m_pressed_text_color;
    protected:
        void do_move_control(push_button& rhs) {
            this->base_type::do_move_control(rhs);
            m_pressed = rhs.m_pressed;
            m_on_pressed_changed_callback = rhs.m_on_pressed_changed_callback;
            rhs.m_on_pressed_changed_callback = nullptr;
            m_on_pressed_changed_callback_state = rhs.m_on_pressed_changed_callback_state;
            m_round_ratio = rhs.m_round_ratio;
            m_padding = rhs.m_padding;
            m_text = rhs.m_text;
            m_ofnt = rhs.m_ofnt;
            m_fnt = rhs.m_fnt;
            m_vfnt = rhs.m_vfnt;
            m_text_line_height = rhs.m_text_line_height;
            m_text_justify = rhs.m_text_justify;
            m_text_encoding = rhs.m_text_encoding;
            m_text_rect = rhs.m_text_rect;
            m_background_color = rhs.m_background_color;
            m_border_color = rhs.m_border_color;
            m_text_color = rhs.m_text_color;
            m_pressed_background_color = rhs.m_pressed_background_color;
            m_pressed_border_color = rhs.m_pressed_border_color;
            m_pressed_text_color = rhs.m_pressed_text_color;
        }
        void do_copy_control(const push_button& rhs) {
            this->base_type::do_copy_control(rhs);
            m_pressed = rhs.m_pressed;
            m_on_pressed_changed_callback = rhs.m_on_pressed_changed_callback;
            m_on_pressed_changed_callback_state = rhs.m_on_pressed_changed_callback_state;
            m_round_ratio = rhs.m_round_ratio;
            m_padding = rhs.m_padding;
            m_text = rhs.m_text;
            m_ofnt = rhs.m_ofnt;
            m_vfnt = rhs.m_vfnt;
            m_fnt = rhs.m_fnt;
            m_text_line_height = rhs.m_text_line_height;
            m_text_justify = rhs.m_text_justify;
            m_text_encoding = rhs.text_encoding;
            m_text_rect = rhs.m_text_rect;
            m_background_color = rhs.m_background_color;
            m_border_color = rhs.m_border_color;
            m_text_color = rhs.m_text_color;
            m_pressed_background_color = rhs.m_pressed_background_color;
            m_pressed_border_color = rhs.m_pressed_border_color;
            m_pressed_text_color = rhs.m_pressed_text_color;
        }
    public:
        /// @brief Moves a push_button
        /// @param rhs The push_button
        push_button(push_button&& rhs) {
            do_move_control(rhs);
        }
        /// @brief Moves a push_button
        /// @param rhs The push_button
        /// @return this
        push_button& operator=(push_button&& rhs) {
            do_move_control(rhs);
            return *this;
        }
        /// @brief Copies a push_button
        /// @param rhs The push_button
        push_button(const push_button& rhs) {
            do_copy_control(rhs);
        }
        /// @brief Copies a push_button
        /// @param rhs The push_button
        /// @return this
        push_button& operator=(const push_button& rhs) {
            do_copy_control(rhs);
            return *this;
        }
        /// @brief Constructs a new instance of a label with the specified parent and optional palette
        /// @param parent The parent. Usually this is the screen
        /// @param palette The palette, if any. This is usually taken from the screen
        push_button(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent,palette), m_pressed(false),m_on_pressed_changed_callback(nullptr),m_on_pressed_changed_callback_state(nullptr), m_round_ratio(NAN),m_padding(4,4), m_text_line_height(25),m_text_justify(uix_justify::center),m_text_encoding(uix_encoding::utf8) {
            const auto white = gfx::rgba_pixel<32>(0xFF,0xFF,0xFF,0xFF);
            const auto black = gfx::rgba_pixel<32>(0x00,0x00,0x00,0xFF);
            background_color(white,true);
            border_color(black,true);
            text_color(black,true);
            m_ofnt = nullptr;
            m_vfnt = nullptr;
            m_fnt = nullptr;
        }
        /// @brief Constructs a new instance of a label with the specified parent and optional palette
        push_button() : base_type(), m_pressed(false),m_on_pressed_changed_callback(nullptr),m_on_pressed_changed_callback_state(nullptr), m_round_ratio(NAN),m_padding(4,4), m_text_line_height(25),m_text_justify(uix_justify::center),m_text_encoding(uix_encoding::utf8) {
            const auto white = gfx::rgba_pixel<32>(0xFF,0xFF,0xFF,0xFF);
            const auto black = gfx::rgba_pixel<32>(0x00,0x00,0x00,0xFF);
            background_color(white,true);
            border_color(black,true);
            text_color(black,true);
            m_ofnt = nullptr;
            m_vfnt = nullptr;
            m_fnt = nullptr;
        }
        /// @brief Indicates whether or not the button is currently pressed
        /// @return 
        bool pressed() const {
            return m_pressed;
        }
        /// @brief Indicates the text of the button
        /// @return The text
        const char* text() const {
            return m_text;
        }
        /// @brief Sets the text of the button
        /// @param value The text
        void text(const char* value) {
            m_text = value;
            this->invalidate();
        }
        /// @brief Indicates the round ratio of the border
        /// @return The round ratio
        float round_ratio() const { 
            return m_round_ratio;
        }
        /// @brief Sets the round ratio of the border
        /// @param value The round ratio
        void round_ratio(float value) {
            m_round_ratio = value;
            this->invalidate();
        }
        /// @brief Indicates the padding around the text
        /// @return A ssize16 indicating the padding
        ssize16 padding() const { 
            return m_padding;
        }
        /// @brief Sets the padding around the text
        /// @param value a ssize16 indicating the padding
        void padding(ssize16 value) {
            m_padding = value;
            m_text_rect = {0,0,0,0};
            this->invalidate();
        }
        /// @brief Gets the height of the font in pixels
        /// @return the line height in pixels
        size_t text_line_height() const {
            if(m_ofnt!=nullptr) {
                return m_text_line_height;
            } else if(m_vfnt!=nullptr) {
                return m_vfnt->y_advance();
            } else if(m_fnt!=nullptr) {
                return m_fnt->height();
            }
            return 0;
        }
        /// @brief Sets the height of the font in pixels (vector only)
        /// @param value the line height in pixels
        void text_line_height(size_t value) {
            m_text_line_height = value;
            m_text_rect = {0,0,0,0};
            this->invalidate();
        }
        /// @brief Indicates the text justification
        /// @return The justification of the text
        uix_justify text_justify() const { 
            return m_text_justify;
        }
        /// @brief Indicates the character encoding (Unicode capable fonts only)
        /// @return The character encoding
        uix_encoding text_encoding() const {
            return m_text_encoding;
        }
        /// @brief Sets the text justification
        /// @param value The justification of the text
        void text_justify(uix_justify value) {
            m_text_justify = value;
            m_text_rect = {0,0,0,0};
            this->invalidate();
        }
        /// @brief Sets the character encoding (Unicode capable fonts only)
        /// @param value The character encoding
        void text_encoding(uix_encoding value) {
            if(m_text_encoding!=value) {
                m_text_encoding = value;
                m_text_rect = {0,0,0,0};
                this->invalidate();
            }
        }
        /// @brief Indicates the TTF/OTF font being used, if any
        /// @return A pointer to the font
        const gfx::open_font* text_open_font() const { 
            return m_ofnt;
        }
        /// @brief Sets the TTF/OTF font being used
        /// @param value The font, or null to clear the setting
        void text_open_font(const gfx::open_font* value) {
            m_ofnt = value;
            m_text_rect = {0,0,0,0};
            this->invalidate();
        }
        /// @brief Indicates the VLW font being used, if any
        /// @return A pointer to the font
        const gfx::vlw_font* text_vlw_font() const { 
            return m_vfnt;
        }
        /// @brief Sets the VLW font being used
        /// @param value The font, or null to clear the setting
        void text_vlw_font(const gfx::vlw_font* value) {
            m_vfnt = value;
            m_text_rect = {0,0,0,0};
            this->invalidate();
        }
        /// @brief Indicates the Win 3.1 raster font being used, if any
        /// @return A pointer to the font
        const gfx::font* text_font() const { 
            return m_fnt;
        }
        /// @brief Sets the Win 3.1 raster font being used, if any
        /// @param value The font, or null to clear the setting
        void text_font(const gfx::font* value) {
            m_fnt = value;
            m_text_rect = {0,0,0,0};
            this->invalidate();
        }
        /// @brief Indicates the background color of the button
        /// @return The RGBA8888 color
        gfx::rgba_pixel<32> background_color() const {
            return m_background_color;
        }
        /// @brief Sets the background color of the button
        /// @param value The RGBA8888 color
        /// @param set_pressed true to set the pressed corollary
        void background_color(gfx::rgba_pixel<32> value, bool set_pressed = false) {
            m_background_color = value;
            if(set_pressed) {
                m_pressed_background_color = value;
            }
            this->invalidate();
        }
        /// @brief Indicates the border color of the button
        /// @return The RGBA8888 color
        gfx::rgba_pixel<32> border_color() const {
            return m_border_color;
        }
        
        /// @brief Sets the border color of the button
        /// @param value The RGBA8888 color
        /// @param set_pressed true to set the pressed corollary
        void border_color(gfx::rgba_pixel<32> value, bool set_pressed = false) {
            m_border_color = value;
            if(set_pressed) {
                m_pressed_border_color = value;
            }
            this->invalidate();
        }
        /// @brief Indicates the text color of the button
        /// @return The RGBA8888 color
        gfx::rgba_pixel<32> text_color() const {
            return m_text_color;
        }
        /// @brief Sets the text color of the button
        /// @param value The RGBA8888 color
        /// @param set_pressed true to set the pressed corollary
        void text_color(gfx::rgba_pixel<32> value, bool set_pressed = false) {
            m_text_color = value;
            if(set_pressed) {
                m_pressed_text_color = value;
            }
            this->invalidate();
        }
        /// @brief Indicates the background color of the button when pressed
        /// @return The RGBA8888 color
        gfx::rgba_pixel<32> pressed_background_color() const {
            return m_pressed_background_color;
        }
        /// @brief Sets the background color of the button when pressed
        /// @param value The RGBA8888 color
        void pressed_background_color(gfx::rgba_pixel<32> value) {
            m_pressed_background_color = value;
            this->invalidate();
        }
        /// @brief Indicates the border color of the button when pressed
        /// @return The RGBA8888 color
        gfx::rgba_pixel<32> pressed_border_color() const {
            return m_pressed_border_color;
        }
        /// @brief Sets the border color of the button when pressed
        /// @param value The RGBA8888 color
        void pressed_border_color(gfx::rgba_pixel<32> value) {
            m_pressed_border_color = value;
            this->invalidate();
        }
        /// @brief Indicates the text color of the button when pressed
        /// @return The RGBA8888 color
        gfx::rgba_pixel<32> pressed_text_color() const {
            return m_pressed_text_color;
        }
        /// @brief Sets the text color of the button when pressed
        /// @param value The RGBA8888 color
        void pressed_text_color(gfx::rgba_pixel<32> value) {
            m_pressed_text_color = value;
            this->invalidate();
        }
        /// @brief Retrieves the pressed changed callback
        /// @return A pointer to the callback
        on_pressed_changed_callback_type on_pressed_changed_callback() const {
            return m_on_pressed_changed_callback;
        }
        /// @brief Retrieves the pressed changed callback state
        /// @return The callback state
        void* on_pressed_changed_callback_state() const {
            return m_on_pressed_changed_callback_state;
        }
        /// @brief Sets the pressed changed callback, which is triggered when the button is pressed or released
        /// @param callback The callback
        /// @param state A user defined value passed to the callback
        void on_pressed_changed_callback(on_pressed_changed_callback_type callback, void* state = nullptr) {
            m_on_pressed_changed_callback = callback;
            m_on_pressed_changed_callback_state = state;
        }
        /// @brief Paints the button
        /// @param destination The draw target to draw to
        /// @param clip The clipping rectangle
        virtual void on_paint(control_surface_type& destination,const srect16& clip) override {
            base_type::on_paint(destination,clip);
            srect16 text_rect;
            int16_t offset_x,offset_y;
            gfx::rgba_pixel<32> background_color,border_color,text_color;
            srect16 b=(srect16)this->dimensions().bounds();
            b=srect16(b.x1,b.y1,b.x2-2,b.y2-2);
            if(m_pressed) {
                background_color = m_pressed_background_color;
                border_color = m_pressed_border_color;
                text_color = m_pressed_text_color;
                offset_x = 1;
                offset_y = 1;
            } else {
                background_color = m_background_color;
                border_color = m_border_color;
                text_color = m_text_color;
                offset_x = 0;
                offset_y = 0;
            }
            b.inflate_inplace(-1,-1);
            b.offset_inplace(offset_x,offset_y);
            
            if(m_round_ratio!=m_round_ratio) {
                gfx::draw::filled_rectangle(destination,b,background_color,&clip);
            } else {
                gfx::draw::filled_rounded_rectangle(destination,b,m_round_ratio,background_color,&clip);
            }
            
            if(b.height()>m_padding.height*2 && b.width()>m_padding.width*2) {
                srect16 bb = b.inflate(-m_padding.width,-m_padding.height);
                if(bb.height()>=1 && m_text!=nullptr && *m_text!='\0') {
                    if(m_ofnt!=nullptr && m_text_line_height>0) {
                        gfx::open_text_info oti;
                        oti.font = m_ofnt;
                        oti.text = m_text;
                        oti.encoding = m_text_encoding;
                        oti.scale = oti.font->scale(m_text_line_height);
                        if(m_text_rect==srect16(0,0,0,0)) {
                            text_rect = oti.font->measure_text(ssize16::max(),spoint16::zero(),oti.text,oti.scale,oti.scaled_tab_width,oti.encoding,oti.cache).bounds();
                            switch(m_text_justify) {
                                case uix_justify::top_middle:
                                    text_rect.center_horizontal_inplace((srect16)bb);
                                    break;
                                case uix_justify::top_right:
                                    text_rect.offset_inplace(bb.width()-text_rect.width(),0);
                                    break;
                                case uix_justify::center_left:
                                    text_rect.center_vertical_inplace((srect16)bb);
                                    break;
                                case uix_justify::center:
                                    text_rect.center_inplace((srect16)bb);
                                    break;
                                case uix_justify::center_right:
                                    text_rect.center_vertical_inplace((srect16)bb);
                                    text_rect.offset_inplace(bb.width()-text_rect.width(),0);
                                    break;
                                case uix_justify::bottom_left:
                                    text_rect.offset_inplace(0,bb.height()-text_rect.height());
                                    break;
                                case uix_justify::bottom_middle:
                                    text_rect.center_horizontal_inplace((srect16)bb);
                                    text_rect.offset_inplace(0,bb.height()-text_rect.height());
                                    break;
                                case uix_justify::bottom_right:
                                    text_rect.offset_inplace(bb.width()-text_rect.width(),bb.height()-text_rect.height());
                                    break;
                                default: // top left
                                    break;
                            }
                            m_text_rect = text_rect;
                        }
                        gfx::draw::text(destination,m_text_rect,oti,text_color,gfx::rgba_pixel<32>(),&clip);
                    } else if(m_vfnt!=nullptr) {
                        gfx::vlw_text_info vti;
                        vti.font = m_vfnt;
                        vti.text = m_text;
                        vti.encoding = m_text_encoding;
                        vti.tab_width = 4;
                        vti.transparent_background = true;
                        if(m_text_rect==srect16(0,0,0,0)) {
                            text_rect = vti.font->measure_text(ssize16::max(),vti.text,vti.tab_width,vti.encoding).bounds();
                            switch(m_text_justify) {
                                case uix_justify::top_middle:
                                    text_rect.center_horizontal_inplace((srect16)bb);
                                    break;
                                case uix_justify::top_right:
                                    text_rect.offset_inplace(bb.width()-text_rect.width(),0);
                                    break;
                                case uix_justify::center_left:
                                    text_rect.center_vertical_inplace((srect16)bb);
                                    break;
                                case uix_justify::center:
                                    text_rect.center_inplace((srect16)bb);
                                    break;
                                case uix_justify::center_right:
                                    text_rect.center_vertical_inplace((srect16)bb);
                                    text_rect.offset_inplace(bb.width()-text_rect.width(),0);
                                    break;
                                case uix_justify::bottom_left:
                                    text_rect.offset_inplace(0,bb.height()-text_rect.height());
                                    break;
                                case uix_justify::bottom_middle:
                                    text_rect.center_horizontal_inplace((srect16)bb);
                                    text_rect.offset_inplace(0,bb.height()-text_rect.height());
                                    break;
                                case uix_justify::bottom_right:
                                    text_rect.offset_inplace(bb.width()-text_rect.width(),bb.height()-text_rect.height());
                                    break;
                                default: // top left
                                    break;
                            }
                            m_text_rect = text_rect;
                        }
                        gfx::draw::text(destination,m_text_rect,vti,text_color,gfx::rgba_pixel<32>(),&clip);
                    } else if(m_fnt!=nullptr) {
                        gfx::text_info oti;
                        oti.font = m_fnt;
                        oti.text = m_text;
                        if(m_text_rect==srect16(0,0,0,0)) {
                            text_rect = oti.font->measure_text(ssize16::max(),oti.text,oti.tab_width).bounds();
                            switch(m_text_justify) {
                                case uix_justify::top_middle:
                                    text_rect.center_horizontal_inplace((srect16)b);
                                    break;
                                case uix_justify::top_right:
                                    text_rect.offset_inplace(b.width()-text_rect.width(),0);
                                    break;
                                case uix_justify::center_left:
                                    text_rect.center_vertical_inplace((srect16)b);
                                    break;
                                case uix_justify::center:
                                    text_rect.center_inplace((srect16)b);
                                    break;
                                case uix_justify::center_right:
                                    text_rect.center_vertical_inplace((srect16)b);
                                    text_rect.offset_inplace(b.width()-text_rect.width(),0);
                                    break;
                                case uix_justify::bottom_left:
                                    text_rect.offset_inplace(0,b.height()-text_rect.height());
                                    break;
                                case uix_justify::bottom_middle:
                                    text_rect.center_horizontal_inplace((srect16)b);
                                    text_rect.offset_inplace(0,b.height()-text_rect.height());
                                    break;
                                case uix_justify::bottom_right:
                                    text_rect.offset_inplace(b.width()-text_rect.width(),b.height()-text_rect.height());
                                    break;
                                default: // top left
                                    break;
                            }
                            m_text_rect = text_rect;
                        }
                        gfx::draw::text(destination,m_text_rect,oti,text_color,gfx::rgba_pixel<32>(),&clip);
                    }
                }
            }
            b.inflate_inplace(1,1);
            if(m_round_ratio!=m_round_ratio) {
                gfx::draw::rectangle(destination,b,border_color,&clip);
            } else {
                gfx::draw::rounded_rectangle(destination,b,m_round_ratio,border_color,&clip);
            }
        }
        /// @brief Called when the button is touched
        /// @param locations_size The count of locations
        /// @param locations The locations
        /// @return True if handled, otherwise false
        virtual bool on_touch(size_t locations_size,const spoint16* locations) override {
            if(m_pressed==false) {
                m_pressed = true;
                if(m_on_pressed_changed_callback!=nullptr) {
                    m_on_pressed_changed_callback(m_pressed,m_on_pressed_changed_callback_state);
                }
                if(base_type::visible()) {
                    this->invalidate();
                }
            }
            return true;
        }
        /// @brief Called when the button is released.
        virtual void on_release() override {
            m_pressed = false;
            if(m_on_pressed_changed_callback!=nullptr) {
                m_on_pressed_changed_callback(m_pressed,m_on_pressed_changed_callback_state);
            }
            if(base_type::visible()) {
                this->invalidate();
            }
        }
    };
}
#endif