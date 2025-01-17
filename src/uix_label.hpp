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
        ssize16 m_padding;
        uix_justify m_text_justify;
        gfx::text_info m_text_info;
        gfx::rgba_pixel<32> m_color;
        srect16 m_text_rect;
        bool is_valid() {
            return m_text_info.text!=nullptr && m_text_info.text_font!=nullptr && m_text_info.encoding!=nullptr && m_text_info.text_byte_count!=0;
        }
    protected:
        void do_move_control(label& rhs) {
            this->base_type::do_move_control(rhs);
            m_padding = rhs.m_padding;
            m_text_justify = rhs.m_text_justify;
            m_text_info = rhs.m_text_info;
            m_color = rhs.m_color;
            m_text_rect = rhs.m_text_rect;
        }
        void do_copy_control(const label& rhs) {
            this->base_type::do_copy_control(rhs);
            m_padding = rhs.m_padding;
            m_text_justify = rhs.m_text_justify;
            m_text_info = rhs.m_text_info;
            m_color = rhs.m_color;
            m_text_rect = rhs.m_text_rect;
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
        label(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent,palette), m_padding(4,4),m_text_justify(uix_justify::center), m_text_rect(0,0,0,0) {
            constexpr static const auto white = gfx::rgba_pixel<32>(0xFF,0xFF,0xFF,0xFF);
            color(white);
            m_text_info.text_font = nullptr;
            m_text_info.encoding = &gfx::text_encoding::utf8;
            
        }
        /// @brief Constructs a new instance of a label with the specified parent and optional palette
        label() : base_type(), m_padding(4,4),m_text_justify(uix_justify::center), m_text_rect(0,0,0,0) {
            constexpr static const auto white = gfx::rgba_pixel<32>(0xFF,0xFF,0xFF,0xFF);
            color(white);
        }
        /// @brief Indicates the raw label text
        /// @return The text of the label
        gfx::text_handle text() const {
            return m_text_info.text;
        }
        /// @brief Indicates the byte count for the raw text
        /// @return The byte count of the raw text
        size_t text_byte_count() const {
            return m_text_info.text_byte_count;
        }
        /// @brief Sets the text of the label
        /// @param value 
        void text(const char* value) {
            m_text_info.text_sz(value);
            m_text_rect = {0,0,0,0};
            this->invalidate();
        }
        /// @brief Sets the text of the label
        /// @param value the raw text data
        /// @param byte_count the number of bytes
        void text(const gfx::text_handle value, size_t byte_count) {
            m_text_info.text=value;
            m_text_info.text_byte_count = byte_count;
            m_text_rect = {0,0,0,0};
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
        const text_encoder* text_encoding() const {
            return m_text_info.encoding;
        }
        /// @brief Sets the character encoding (Unicode capable fonts only)
        /// @param value The character encoding
        void text_encoding(const text_encoder* value) {
            if(m_text_info.encoding!=value) {
                m_text_info.encoding = value;
                m_text_rect = {0,0,0,0};
                this->invalidate();
            }
        }
        /// @brief Indicates font being used, if any
        /// @return A pointer to the font
        const gfx::font& font() const { 
            return *m_text_info.text_font;
        }
        /// @brief Sets the font being used, if any
        /// @param value The font, or null to clear the setting
        void font(const gfx::font& value) {
            m_text_info.text_font = &value;
            m_text_rect = {0,0,0,0};
            this->invalidate();
        }
        /// @brief Indicates the text color of the label
        /// @return The RGBA8888 color
        gfx::rgba_pixel<32> color() const {
            return m_color;
        }
        /// @brief Sets the text color of the label
        /// @param value The RGBA8888 color
        void color(gfx::rgba_pixel<32> value) {
            m_color = value;
            this->invalidate();
        }
        /// @brief Indicates the draw cache, if any
        /// @return The draw cache used for this font
        gfx::font_draw_cache* draw_cache() const {
            return m_text_info.draw_cache;
        }
        /// @brief Sets the draw cache
        /// @param value The draw cache for this font
        void draw_cache(gfx::font_draw_cache* value) {
            m_text_info.draw_cache = value;
        }
        /// @brief Sets the draw cache
        /// @param value The draw cache for this font
        void draw_cache(gfx::font_draw_cache& value) {
            draw_cache(&value);
        }
        /// @brief Indicates the measure cache, if any
        /// @return The measure cache used for this font
        gfx::font_measure_cache* measure_cache() const {
            return m_text_info.measure_cache;
        }
        /// @brief Sets the measure cache
        /// @param value The measure cache for this font
        void measure_cache(gfx::font_measure_cache* value) {
            m_text_info.measure_cache = value;
        }
        /// @brief Sets the measure cache
        /// @param value The measure cache for this font
        void measure_cache(gfx::font_measure_cache& value) {
            measure_cache(&value);
        }
        /// @brief Indicates the tab width
        /// @return The tab width used for this font
        uint16_t tab_width() const {
            return m_text_info.tab_width;
        }
        /// @brief Sets the tab width
        /// @param value The tab width for this font
        void tab_width(uint16_t value) {
            m_text_info.tab_width = value;
        }
        
        /// @brief Draws the label
        /// @param destination The control surface to draw to
        /// @param clip The clipping rectangle
        virtual void on_paint(control_surface_type& destination,const srect16& clip) override {
            base_type::on_paint(destination,clip);
            gfx::rgba_pixel<32> color;
            srect16 b=(srect16)this->dimensions().bounds();
            b=srect16(b.x1,b.y1,b.x2,b.y2);
            color = m_color;
            b.inflate_inplace(-1,-1);
            if(b.height()>m_padding.height*2 && b.width()>m_padding.width*2) {
                srect16 bb = b.inflate(-m_padding.width,-m_padding.height);
                if(bb.height()>=1 && is_valid()) {
                    if(m_text_rect.x1==0&&m_text_rect.y1==0&&m_text_rect.x2==0&&m_text_rect.y2==0) {
                        size16 sz;
                        m_text_info.text_font->measure(uint16_t(-1),m_text_info,&sz);
                        m_text_rect = (srect16)sz.bounds();
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
                        gfx::draw::text(destination,m_text_rect,m_text_info,color,&clip);
                    }
                }
            }
        }
    };
}
#endif