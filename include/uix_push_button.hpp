#ifndef HTCW_UIX_PUSH_BUTTON_HPP
#define HTCW_UIX_PUSH_BUTTON_HPP
#include "uix_core.hpp"
namespace uix {
    template<typename ControlSurfaceType>
    class push_button final : public control<ControlSurfaceType> {
    public:
        using type = push_button;
        using base_type = control<ControlSurfaceType>;
        using pixel_type = typename ControlSurfaceType::pixel_type;
        using palette_type = typename ControlSurfaceType::palette_type;
        using control_surface_type = ControlSurfaceType;

        typedef void(*on_pressed_changed_callback_type)(bool pressed,void* state);
    private:
        bool m_pressed;
        on_pressed_changed_callback_type m_on_pressed_changed_callback;
        void* m_on_pressed_changed_callback_state;
        float m_round_ratio;
        size16 m_padding;
        const char* m_text;
        const gfx::open_font* m_ofnt;
        const gfx::font* m_fnt;
        size_t m_text_line_height;
        uix_justify m_text_justify;
        gfx::rgba_pixel<32> m_background_color, m_border_color,m_text_color;
        gfx::rgba_pixel<32> m_pressed_background_color, m_pressed_border_color,m_pressed_text_color;
        void do_move(push_button& rhs) {
            this->do_move_control(rhs);
            m_pressed = rhs.m_pressed;
            m_on_pressed_changed_callback = rhs.m_on_pressed_changed_callback;
            rhs.m_on_pressed_changed_callback = nullptr;
            m_on_pressed_changed_callback_state = rhs.m_on_pressed_changed_callback_state;
            m_round_ratio = rhs.m_round_ratio;
            m_padding = rhs.m_padding;
            m_text = rhs.m_text;
            m_ofnt = rhs.m_ofnt;
            m_fnt = rhs.m_fnt;
            m_text_line_height = rhs.m_text_line_height;
            m_text_justify = rhs.m_text_justify;
            m_background_color = rhs.m_background_color;
            m_border_color = rhs.m_border_color;
            m_text_color = rhs.m_text_color;
            m_pressed_background_color = rhs.m_pressed_background_color;
            m_pressed_border_color = rhs.m_pressed_border_color;
            m_pressed_text_color = rhs.m_pressed_text_color;
        }
        void do_copy(const push_button& rhs) {
            this->do_copy_control(rhs);
            m_pressed = rhs.m_pressed;
            m_on_pressed_changed_callback = rhs.m_on_pressed_changed_callback;
            m_on_pressed_changed_callback_state = rhs.m_on_pressed_changed_callback_state;
            m_round_ratio = rhs.m_round_ratio;
            m_padding = rhs.m_padding;
            m_text = rhs.m_text;
            m_ofnt = rhs.m_ofnt;
            m_fnt = rhs.m_fnt;
            m_text_line_height = rhs.m_text_line_height;
            m_text_justify = rhs.m_text_justify;
            m_background_color = rhs.m_background_color;
            m_border_color = rhs.m_border_color;
            m_text_color = rhs.m_text_color;
            m_pressed_background_color = rhs.m_pressed_background_color;
            m_pressed_border_color = rhs.m_pressed_border_color;
            m_pressed_text_color = rhs.m_pressed_text_color;
        }
    public:
        push_button(push_button&& rhs) {
            do_move(rhs);
        }
        push_button& operator=(push_button&& rhs) {
            do_move(rhs);
            return *this;
        }
        push_button(const push_button& rhs) {
            do_copy(rhs);
        }
        push_button& operator=(const push_button& rhs) {
            do_copy(rhs);
            return *this;
        }
        push_button(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent,palette), m_pressed(false),m_on_pressed_changed_callback(nullptr),m_on_pressed_changed_callback_state(nullptr), m_round_ratio(NAN),m_padding(4,4), m_text_line_height(25),m_text_justify(uix_justify::center) {
            const auto white = gfx::rgba_pixel<32>(0xFF,0xFF,0xFF,0xFF);
            const auto black = gfx::rgba_pixel<32>(0x00,0x00,0x00,0xFF);
            background_color(white,true);
            border_color(black,true);
            text_color(black,true);
            m_ofnt = nullptr;
            m_fnt = nullptr;
        }
        bool pressed() const {
            return m_pressed;
        }
        const char* text() const {
            return m_text;
        }
        void text(const char* value) {
            m_text = value;
            this->invalidate();
        }
        float round_ratio() const { 
            return m_round_ratio;
        }
        void round_ratio(float value) {
            m_round_ratio = value;
            this->invalidate();
        }
        size16 padding() const { 
            return m_padding;
        }
        void padding(size16 value) {
            m_padding = value;
            this->invalidate();
        }
        size_t text_line_height() const { 
            return m_text_line_height;
        }
        void text_line_height(size_t value) {
            m_text_line_height = value;
            this->invalidate();
        }
        uix_justify text_justify() const { 
            return m_text_justify;
        }
        void text_justify(uix_justify value) {
            m_text_justify = value;
            this->invalidate();
        }
        const gfx::open_font* text_open_font() const { 
            return m_ofnt;
        }
        void text_open_font(const gfx::open_font* value) {
            m_ofnt = value;
            this->invalidate();
        }
        const gfx::font* text_font() const { 
            return m_fnt;
        }
        void text_font(const gfx::font* value) {
            m_fnt = value;
            this->invalidate();
        }
        gfx::rgba_pixel<32> background_color() const {
            return m_background_color;
        }
        void background_color(gfx::rgba_pixel<32> value, bool set_pressed = false) {
            m_background_color = value;
            if(set_pressed) {
                m_pressed_background_color = value;
            }
            this->invalidate();
        }
        gfx::rgba_pixel<32> border_color() const {
            return m_border_color;
        }
        void border_color(gfx::rgba_pixel<32> value, bool set_pressed = false) {
            m_border_color = value;
            if(set_pressed) {
                m_pressed_border_color = value;
            }
            this->invalidate();
        }
        gfx::rgba_pixel<32> text_color() const {
            return m_text_color;
        }
        void text_color(gfx::rgba_pixel<32> value, bool set_pressed = false) {
            m_text_color = value;
            if(set_pressed) {
                m_pressed_text_color = value;
            }
            this->invalidate();
        }
        gfx::rgba_pixel<32> pressed_background_color() const {
            return m_pressed_background_color;
        }
        void pressed_background_color(gfx::rgba_pixel<32> value) {
            m_pressed_background_color = value;
            this->invalidate();
        }
        gfx::rgba_pixel<32> pressed_border_color() const {
            return m_pressed_border_color;
        }
        void pressed_border_color(gfx::rgba_pixel<32> value) {
            m_pressed_border_color = value;
            this->invalidate();
        }
        gfx::rgba_pixel<32> pressed_text_color() const {
            return m_pressed_text_color;
        }
        void pressed_text_color(gfx::rgba_pixel<32> value) {
            m_pressed_text_color = value;
            this->invalidate();
        }
        on_pressed_changed_callback_type on_pressed_changed_callback() const {
            return m_on_pressed_changed_callback;
        }
        void* on_pressed_changed_callback_state() const {
            return m_on_pressed_changed_callback_state;
        }
        void on_pressed_changed_callback(on_pressed_changed_callback_type callback, void* state = nullptr) {
            m_on_pressed_changed_callback = callback;
            m_on_pressed_changed_callback_state = state;
        }
        virtual void on_paint(control_surface_type& destination,const srect16& clip) override {
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
                        oti.scale = oti.font->scale(m_text_line_height);
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
                        gfx::draw::text(destination,text_rect,oti,text_color,gfx::rgba_pixel<32>(),&clip);
                    } else if(m_fnt!=nullptr) {
                        gfx::text_info oti;
                        oti.font = m_fnt;
                        oti.text = m_text;
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
                        gfx::draw::text(destination,text_rect,oti,text_color,gfx::rgba_pixel<32>(),&clip);
                    }
                }
            }
            b.inflate_inplace(1,1);
            if(m_round_ratio!=m_round_ratio) {
                gfx::draw::rectangle(destination,b,border_color,&clip);
            } else {
                gfx::draw::rounded_rectangle(destination,b,m_round_ratio,border_color,&clip);
            }
            base_type::on_paint(destination,clip);
        }
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