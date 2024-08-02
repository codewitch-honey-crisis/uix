#ifndef HTCW_UIX_LABEL_HPP
#define HTCW_UIX_LABEL_HPP
#include "uix_core.hpp"
namespace uix {
    /// @brief Represents a text label
    /// @tparam ControlSurfaceType The control surfice type, usually from the screen
    template<typename ControlSurfaceType>
    class label final : public control<ControlSurfaceType> {
    public:
        using type = label;
        using base_type = control<ControlSurfaceType>;
        using pixel_type = typename ControlSurfaceType::pixel_type;
        using palette_type = typename ControlSurfaceType::palette_type;
        using control_surface_type = ControlSurfaceType;
        typedef void(*on_pressed_changed_callback_type)(bool pressed,void* state);
    private:
        float m_round_ratio;
        ssize16 m_padding;
        const char* m_text;
        const gfx::open_font* m_ofnt;
        const gfx::vlw_font* m_vfnt;
        const gfx::font* m_fnt;
        size_t m_text_line_height;
        uix_justify m_text_justify;
        uix_encoding m_text_encoding;
        gfx::rgba_pixel<32> m_background_color, m_border_color,m_text_color;
        srect16 m_text_rect;
        float m_text_scale;
    protected:
        void do_move_control(label& rhs) {
            this->base_type::do_move_control(rhs);
            m_round_ratio = rhs.m_round_ratio;
            m_padding = rhs.m_padding;
            m_text = rhs.m_text;
            m_ofnt = rhs.m_ofnt;
            m_vfnt = rhs.m_vfnt;
            m_fnt = rhs.m_fnt;
            m_text_line_height = rhs.m_text_line_height;
            m_text_justify = rhs.m_text_justify;
            m_text_encoding = rhs.m_text_encoding;
            m_background_color = rhs.m_background_color;
            m_border_color = rhs.m_border_color;
            m_text_color = rhs.m_text_color;
            m_text_rect = rhs.m_text_rect;
            m_text_scale = rhs.m_text_scale;
        }
        void do_copy_control(const label& rhs) {
            this->base_type::do_copy_control(rhs);
            m_round_ratio = rhs.m_round_ratio;
            m_padding = rhs.m_padding;
            m_text = rhs.m_text;
            m_ofnt = rhs.m_ofnt;
            m_vfnt = rhs.m_vfnt;
            m_fnt = rhs.m_fnt;
            m_text_line_height = rhs.m_text_line_height;
            m_text_justify = rhs.m_text_justify;
            m_text_encoding = rhs.m_text_encoding;
            m_background_color = rhs.m_background_color;
            m_border_color = rhs.m_border_color;
            m_text_color = rhs.m_text_color;
            m_text_rect = rhs.m_text_rect;
            m_text_scale = rhs.m_text_scale;
        }
    public:
        /// @brief Moves a label
        /// @param rhs The label to move
        label(label&& rhs) {
            this->do_move_control(rhs);
        }
        /// @brief Moves a label
        /// @param rhs The label to move
        /// @return this
        label& operator=(label&& rhs) {
            this->do_move_control(rhs);
            return *this;
        }
        /// @brief Copies a label
        /// @param rhs The label to copy
        label(const label& rhs) {
            this->do_copy_control(rhs);
        }
        /// @brief Copies a label
        /// @param rhs The label to copy
        /// @return this
        label& operator=(const label& rhs) {
            this->do_copy_control(rhs);
            return *this;
        }
        /// @brief Constructs a new instance of a label with the specified parent and optional palette
        /// @param parent The parent. Usually this is the screen
        /// @param palette The palette, if any. This is usually taken from the screen
        label(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent,palette), m_round_ratio(NAN),m_padding(4,4), m_text_line_height(25),m_text_justify(uix_justify::center),m_text_encoding(uix_encoding::utf8), m_text_rect(0,0,0,0),m_text_scale(NAN) {
            constexpr static const auto white = gfx::rgba_pixel<32>(0xFF,0xFF,0xFF,0xFF);
            constexpr static const auto transparent = gfx::rgba_pixel<32>(0,0,0,0);
            background_color(transparent);
            border_color(transparent);
            text_color(white);
            m_ofnt = nullptr;
            m_vfnt = nullptr;
            m_fnt = nullptr;
        }
        /// @brief Constructs a new instance of a label with the specified parent and optional palette
        label() : base_type(), m_round_ratio(NAN),m_padding(4,4), m_text_line_height(25),m_text_justify(uix_justify::center),m_text_encoding(uix_encoding::utf8), m_text_rect(0,0,0,0),m_text_scale(NAN) {
            constexpr static const auto white = gfx::rgba_pixel<32>(0xFF,0xFF,0xFF,0xFF);
            constexpr static const auto transparent = gfx::rgba_pixel<32>(0,0,0,0);
            background_color(transparent);
            border_color(transparent);
            text_color(white);
            m_ofnt = nullptr;
            m_vfnt = nullptr;
            m_fnt = nullptr;
        }
        /// @brief Indicates the label text
        /// @return The text of the label
        const char* text() const {
            return m_text;
        }
        /// @brief Sets the text of the label
        /// @param value 
        void text(const char* value) {
            m_text = value;
            m_text_rect = {0,0,0,0};
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
            m_text_scale = NAN;
            this->invalidate();
        }
        /// @brief Indicates the text justification
        /// @return The justification of the text
        uix_justify text_justify() const { 
            return m_text_justify;
        }
        /// @brief Sets the text justification
        /// @param value The justification of the text
        void text_justify(uix_justify value) {
            m_text_justify = value;
            m_text_rect = {0,0,0,0};
            this->invalidate();
        }
        /// @brief Indicates the character encoding (Unicode capable fonts only)
        /// @return The character encoding
        uix_encoding text_encoding() const {
            return m_text_encoding;
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
            m_text_scale = NAN;
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
        /// @brief Indicates the background color of the label
        /// @return The RGBA8888 color
        gfx::rgba_pixel<32> background_color() const {
            return m_background_color;
        }
        /// @brief Sets the background color of the label
        /// @param value The RGBA8888 color
        void background_color(gfx::rgba_pixel<32> value) {
            m_background_color = value;
            this->invalidate();
        }
        /// @brief Indicates the border color of the label
        /// @return The RGBA8888 color
        gfx::rgba_pixel<32> border_color() const {
            return m_border_color;
        }
        /// @brief Sets the border color of the label
        /// @param value The RGBA8888 color
        void border_color(gfx::rgba_pixel<32> value) {
            m_border_color = value;
            this->invalidate();
        }
        /// @brief Indicates the text color of the label
        /// @return The RGBA8888 color
        gfx::rgba_pixel<32> text_color() const {
            return m_text_color;
        }
        /// @brief Sets the text color of the label
        /// @param value The RGBA8888 color
        void text_color(gfx::rgba_pixel<32> value) {
            m_text_color = value;
            this->invalidate();
        }
        /// @brief Draws the label
        /// @param destination The control surface to draw to
        /// @param clip The clipping rectangle
        virtual void on_paint(control_surface_type& destination,const srect16& clip) override {
            base_type::on_paint(destination,clip);
            gfx::rgba_pixel<32> background_color,border_color,text_color;
            srect16 b=(srect16)this->dimensions().bounds();
            b=srect16(b.x1,b.y1,b.x2,b.y2);
            background_color = m_background_color;
            border_color = m_border_color;
            text_color = m_text_color;
            b.inflate_inplace(-1,-1);
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
                        if(m_text_scale!=m_text_scale) {
                            m_text_scale = oti.font->scale(m_text_line_height);
                        }
                        oti.scale = m_text_scale;
                        if(m_text_rect.x1==0&&m_text_rect.y1==0&&m_text_rect.x2==0&&m_text_rect.y2==0) {
                            m_text_rect = oti.font->measure_text(ssize16::max(),spoint16::zero(),oti.text,oti.scale,oti.scaled_tab_width,oti.encoding,oti.cache).bounds();
                            switch(m_text_justify) {
                                case uix_justify::top_middle:
                                    m_text_rect.center_horizontal_inplace((srect16)bb);
                                    break;
                                case uix_justify::top_right:
                                    m_text_rect.offset_inplace(bb.width()-m_text_rect.width(),0);
                                    break;
                                case uix_justify::center_left:
                                    m_text_rect.center_vertical_inplace((srect16)bb);
                                    break;
                                case uix_justify::center:
                                    m_text_rect.center_inplace((srect16)bb);
                                    break;
                                case uix_justify::center_right:
                                    m_text_rect.center_vertical_inplace((srect16)bb);
                                    m_text_rect.offset_inplace(bb.width()-m_text_rect.width(),0);
                                    break;
                                case uix_justify::bottom_left:
                                    m_text_rect.offset_inplace(0,bb.height()-m_text_rect.height());
                                    break;
                                case uix_justify::bottom_middle:
                                    m_text_rect.center_horizontal_inplace((srect16)bb);
                                    m_text_rect.offset_inplace(0,bb.height()-m_text_rect.height());
                                    break;
                                case uix_justify::bottom_right:
                                    m_text_rect.offset_inplace(bb.width()-m_text_rect.width(),bb.height()-m_text_rect.height());
                                    break;
                                default: // top left
                                    break;
                            }
                        }
                        if(clip.intersects(m_text_rect)) {
                            gfx::draw::text(destination,m_text_rect,oti,text_color,gfx::rgba_pixel<32>(),&clip);
                        }
                    } else if(m_vfnt!=nullptr) {
                        gfx::vlw_text_info vti;
                        vti.font = m_vfnt;
                        vti.text = m_text;
                        vti.encoding = m_text_encoding;
                        vti.tab_width = 4;
                        vti.no_antialiasing = false;
                        vti.transparent_background = true;
                        if(m_text_rect.x1==0&&m_text_rect.y1==0&&m_text_rect.x2==0&&m_text_rect.y2==0) {
                            m_text_rect = vti.font->measure_text(ssize16::max(),vti.text,vti.tab_width,vti.encoding).bounds();
                            switch(m_text_justify) {
                                case uix_justify::top_middle:
                                    m_text_rect.center_horizontal_inplace((srect16)bb);
                                    break;
                                case uix_justify::top_right:
                                    m_text_rect.offset_inplace(bb.width()-m_text_rect.width(),0);
                                    break;
                                case uix_justify::center_left:
                                    m_text_rect.center_vertical_inplace((srect16)bb);
                                    break;
                                case uix_justify::center:
                                    m_text_rect.center_inplace((srect16)bb);
                                    break;
                                case uix_justify::center_right:
                                    m_text_rect.center_vertical_inplace((srect16)bb);
                                    m_text_rect.offset_inplace(bb.width()-m_text_rect.width(),0);
                                    break;
                                case uix_justify::bottom_left:
                                    m_text_rect.offset_inplace(0,bb.height()-m_text_rect.height());
                                    break;
                                case uix_justify::bottom_middle:
                                    m_text_rect.center_horizontal_inplace((srect16)bb);
                                    m_text_rect.offset_inplace(0,bb.height()-m_text_rect.height());
                                    break;
                                case uix_justify::bottom_right:
                                    m_text_rect.offset_inplace(bb.width()-m_text_rect.width(),bb.height()-m_text_rect.height());
                                    break;
                                default: // top left
                                    break;
                            }
                        }
                        if(clip.intersects(m_text_rect)) {
                            gfx::draw::text(destination,m_text_rect,vti,text_color,gfx::rgba_pixel<32>(),&clip);
                        }
                    } else if(m_fnt!=nullptr) {
                        gfx::text_info oti;
                        oti.font = m_fnt;
                        oti.text = m_text;
                        if(m_text_rect.x1==0&&m_text_rect.y1==0&&m_text_rect.x2==0&&m_text_rect.y2==0) {
                            m_text_rect = oti.font->measure_text(ssize16::max(),oti.text,oti.tab_width).bounds();
                            switch(m_text_justify) {
                                case uix_justify::top_middle:
                                    m_text_rect.center_horizontal_inplace((srect16)b);
                                    break;
                                case uix_justify::top_right:
                                    m_text_rect.offset_inplace(b.width()-m_text_rect.width(),0);
                                    break;
                                case uix_justify::center_left:
                                    m_text_rect.center_vertical_inplace((srect16)b);
                                    break;
                                case uix_justify::center:
                                    m_text_rect.center_inplace((srect16)b);
                                    break;
                                case uix_justify::center_right:
                                    m_text_rect.center_vertical_inplace((srect16)b);
                                    m_text_rect.offset_inplace(b.width()-m_text_rect.width(),0);
                                    break;
                                case uix_justify::bottom_left:
                                    m_text_rect.offset_inplace(0,b.height()-m_text_rect.height());
                                    break;
                                case uix_justify::bottom_middle:
                                    m_text_rect.center_horizontal_inplace((srect16)b);
                                    m_text_rect.offset_inplace(0,b.height()-m_text_rect.height());
                                    break;
                                case uix_justify::bottom_right:
                                    m_text_rect.offset_inplace(b.width()-m_text_rect.width(),b.height()-m_text_rect.height());
                                    break;
                                default: // top left
                                    break;
                            }
                        }
                        if(clip.intersects(m_text_rect)) {
                            gfx::draw::text(destination,m_text_rect,oti,text_color,gfx::rgba_pixel<32>(),&clip);
                        }
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
    };
}
#endif