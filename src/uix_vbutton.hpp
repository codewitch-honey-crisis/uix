#ifndef HTCW_UIX_VBUTTON_HPP
#define HTCW_UIX_VBUTTON_HPP
#include "uix_canvas_control.hpp"
namespace uix {
template <typename ControlSurfaceType>
class vbutton : public canvas_control<ControlSurfaceType> {
   public:
    using type = vbutton;
    using pixel_type = typename ControlSurfaceType::pixel_type;
    using palette_type = typename ControlSurfaceType::palette_type;
    using control_surface_type = ControlSurfaceType;
    typedef void (*on_pressed_changed_callback_type)(bool new_value, void* state);

   private:
    using base_type = canvas_control<ControlSurfaceType>;
    ssize16 m_padding;
    uix_justify m_text_justify;
    gfx::canvas_text_info m_text_info;
    gfx::vector_pixel m_color;
    gfx::vector_pixel m_background_color;
    gfx::vector_pixel m_border_color;
    size16 m_radiuses;
    uint16_t m_border_width;
    gfx::pointf m_text_loc;
    on_pressed_changed_callback_type m_on_pressed_changed_callback;
    void* m_on_pressed_changed_callback_state;
    bool m_pressed;
    bool is_valid() {
        return m_text_info.text != nullptr && m_text_info.font_size != 0 && m_text_info.ttf_font != nullptr && m_text_info.encoding != nullptr && m_text_info.text_byte_count != 0;
    }
    gfx::pointf compute_location() {
        if (!is_valid() || m_text_justify == uix_justify::top_left) {
            return {0.f, 0.f};
        }
        ssize16 csz = this->dimensions();
        gfx::canvas_path p;
        if (gfx::gfx_result::success != p.initialize()) {
            return {0.f, 0.f};
        }
        p.text({0.f, 0.f}, m_text_info);
        gfx::rectf b = p.bounds(true);
        p.deinitialize();
        gfx::pointf offset(-b.x1, -b.y1);
        b.offset_inplace(offset);
        gfx::rectf s = (gfx::rectf)csz.bounds();
        s.inflate_inplace(-m_border_width - 1, -m_border_width - 1);
        switch (m_text_justify) {
            case uix_justify::center:
                return b.center(s).point1().offset(offset);
            case uix_justify::bottom_left:
                return gfx::pointf(0, s.height() - b.height()).offset(offset);
            case uix_justify::bottom_middle: {
                const float y = s.height() - b.height();
                const float x = (s.width() - b.width()) * 0.5f;
                return gfx::pointf(x, y).offset(offset);
            }
            case uix_justify::bottom_right: {
                const float y = s.height() - b.height();
                const float x = s.width() - b.width();
                return gfx::pointf(x, y).offset(offset);
            }
            case uix_justify::center_left:
                return b.center_vertical(s).point1().offset(offset);
            case uix_justify::center_right: {
                const float y = (s.height() - b.height()) * 0.5f;
                const float x = s.width() - b.width();
                return gfx::pointf(x, y).offset(offset);
            }
            case uix_justify::top_middle:
                return b.center_horizontal(s).point1().offset(offset);
            case uix_justify::top_right:
                return gfx::pointf(s.width() - b.width(), 0).offset(offset);
            default:
                return {0.f, 0.f};
        }
    }

   protected:
    void do_move_control(vbutton& rhs) {
        this->base_type::do_move_control(rhs);
        m_padding = rhs.m_padding;
        m_text_justify = rhs.m_text_justify;
        m_text_info = rhs.m_text_info;
        m_color = rhs.m_color;
        m_background_color = rhs.m_background_color;
        m_border_color = rhs.m_border_color;
        m_border_width = rhs.m_border_width;
        m_radiuses = rhs.m_radiuses;
        m_text_loc = rhs.m_text_loc;
        m_on_pressed_changed_callback = rhs.m_on_pressed_changed_callback;
        rhs.m_on_pressed_changed_callback = nullptr;
        m_on_pressed_changed_callback_state = rhs.m_on_pressed_changed_callback_state;
        m_pressed = rhs.m_pressed;
        rhs.m_pressed = false;
    }
    void do_copy_control(const vbutton& rhs) {
        this->base_type::do_copy_control(rhs);
        m_padding = rhs.m_padding;
        m_text_justify = rhs.m_text_justify;
        m_text_info = rhs.m_text_info;
        m_color = rhs.m_color;
        m_background_color = rhs.m_background_color;
        m_border_color = rhs.m_border_color;
        m_border_width = rhs.m_border_width;
        m_radiuses = rhs.m_radiuses;
        m_text_loc = rhs.m_text_loc;
        m_on_pressed_changed_callback = rhs.m_on_pressed_changed_callback;
        m_on_pressed_changed_callback_state = rhs.m_on_pressed_changed_callback_state;
        m_pressed = rhs.m_pressed;
    }
    virtual void on_paint(gfx::canvas& destination, const srect16& clip) override {
        const bool has_border = m_border_color != m_background_color && m_border_color.opacity() != 0;
        gfx::canvas_style st = destination.style();
        st.fill_rule = gfx::fill_rule::non_zero;
        if (has_border) {
            st.stroke_color = m_border_color;
            st.stroke_width = m_border_width * 0.5f;
            st.stroke_paint_type = gfx::paint_type::solid;
        } else {
            st.stroke_paint_type = gfx::paint_type::none;
        }
        st.fill_color = m_background_color;
        st.fill_paint_type = gfx::paint_type::solid;
        destination.style(st);
        gfx::rectf rf = (rectf)destination.bounds();
        rf.inflate_inplace(-2, -2);
        if (m_pressed) {
            rf.offset_inplace(1, 1);
        }
        gfx::sizef szf = (sizef)m_radiuses;
        destination.rounded_rectangle(rf, szf);
        destination.render();
        gfx::canvas_text_info cti;
        if (is_valid()) {
            destination.stroke_paint_type(gfx::paint_type::none);
            destination.fill_color(m_color);
            if (m_text_loc.x == -1.f && m_text_loc.y == -1.f) {
                m_text_loc = compute_location();
            }
            destination.text(m_text_loc.offset(m_pressed, m_pressed), m_text_info);
            destination.render();
        }
    }
    virtual void on_pressed_changed(bool new_value) {
        if (m_on_pressed_changed_callback != nullptr) {
            m_on_pressed_changed_callback(new_value, m_on_pressed_changed_callback_state);
        }
    }
    /// @brief Called when the button is touched
    /// @param locations_size The count of locations
    /// @param locations The locations
    /// @return True if handled, otherwise false
    virtual bool on_touch(size_t locations_size, const spoint16* locations) override {
        if (m_pressed == false) {
            m_pressed = true;
            on_pressed_changed(m_pressed);
            if (base_type::visible()) {
                this->invalidate();
            }
        }
        return true;
    }
    /// @brief Called when the button is released.
    virtual void on_release() override {
        m_pressed = false;
        on_pressed_changed(m_pressed);
        if (base_type::visible()) {
            this->invalidate();
        }
    }

   public:
    /// @brief Constructs an empty control instance
    vbutton() : base_type(), m_padding({4, 4}), m_text_justify(uix_justify::center), m_text_loc(-1, -1), m_on_pressed_changed_callback(nullptr), m_pressed(false) {
        constexpr static const auto black = gfx::rgba_pixel<32>(0xFF, 0xFF, 0xFF, 0xFF);
        constexpr static const auto gray = gfx::rgba_pixel<32>(0x7F, 0x7F, 0x7F, 0xFF);
        color(black);
        background_color(gray);
        border_color(black);
        border_width(2);
        radiuses({0, 0});
        m_text_info.ttf_font = nullptr;
        m_text_info.encoding = &gfx::text_encoding::utf8;
    }
    /// @brief Constructs a control given a parent and an optional palette
    /// @param parent The parent invalidation tracker - usually a screen
    /// @param palette The palette. Typically the screen's palette()
    vbutton(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent, palette), m_padding({4, 4}), m_text_justify(uix_justify::center), m_text_loc(-1, -1), m_on_pressed_changed_callback(nullptr), m_pressed(false) {
        constexpr static const auto black = gfx::rgba_pixel<32>(0xFF, 0xFF, 0xFF, 0xFF);
        constexpr static const auto gray = gfx::rgba_pixel<32>(0x7F, 0x7F, 0x7F, 0xFF);
        color(black);
        background_color(gray);
        border_color(black);
        border_width(2);
        radiuses({0, 0});
        m_text_info.ttf_font = nullptr;
        m_text_info.encoding = &gfx::text_encoding::utf8;
    }
    /// @brief Indicates whether or not the button is pressed
    /// @return True if pressed, otherwise false
    bool pressed() const {
        return m_pressed;
    }
    /// @brief Indicates the raw button text
    /// @return The text of the button
    gfx::text_handle text() const {
        return m_text_info.text;
    }
    /// @brief Indicates the byte count for the raw text
    /// @return The byte count of the raw text
    size_t text_byte_count() const {
        return m_text_info.text_byte_count;
    }
    /// @brief Sets the text of the button
    /// @param value
    void text(const char* value) {
        m_text_info.text_sz(value);
        m_text_loc = gfx::pointf(-1, -1);
        this->invalidate();
    }
    /// @brief Sets the text of the button
    /// @param value the raw text data
    /// @param byte_count the number of bytes
    void text(const gfx::text_handle value, size_t byte_count) {
        m_text_info.text = value;
        m_text_info.text_byte_count = byte_count;
        m_text_loc = gfx::pointf(-1, -1);
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
        m_text_loc = gfx::pointf(-1, -1);
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
        m_text_loc = gfx::pointf(-1, -1);
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
        if (m_text_info.encoding != value) {
            m_text_info.encoding = value;
            m_text_loc = gfx::pointf(-1, -1);
            this->invalidate();
        }
    }
    /// @brief Indicates font being used, if any
    /// @return A pointer to the font
    const io::stream& font() const {
        return *m_text_info.ttf_font;
    }
    /// @brief Sets the font being used, if any
    /// @param value The font, or null to clear the setting
    void font(io::stream& value) {
        m_text_info.ttf_font = &value;
        m_text_loc = gfx::pointf(-1, -1);
        this->invalidate();
    }
    /// @brief Indicates font size
    /// @return The height of the font, in pixels
    uint16_t font_size() const {
        return m_text_info.font_size + 0.5f;
    }
    /// @brief Sets size of the font
    /// @param value The font height in pixels
    void font_size(uint16_t value) {
        m_text_info.font_size = value;
        m_text_loc = gfx::pointf(-1, -1);
        this->invalidate();
    }
    /// @brief Indicates the text color of the button
    /// @return The RGBA8888 color
    gfx::rgba_pixel<32> color() const {
        gfx::rgba_pixel<32> result;
        convert(m_color, &result);
        return result;
    }
    /// @brief Sets the text color of the button
    /// @param value The RGBA8888 color
    void color(gfx::rgba_pixel<32> value) {
        convert(value, &m_color);
        this->invalidate();
    }
    /// @brief Indicates the background color of the button
    /// @return The RGBA8888 color
    gfx::rgba_pixel<32> background_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_background_color, &result);
        return result;
    }
    /// @brief Sets the background color of the button
    /// @param value The RGBA8888 color
    void background_color(gfx::rgba_pixel<32> value) {
        convert(value, &m_background_color);
        this->invalidate();
    }
    /// @brief Indicates the border color of the button
    /// @return The RGBA8888 color
    gfx::rgba_pixel<32> border_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_border_color, &result);
        return result;
    }
    /// @brief Sets the border color of the button
    /// @param value The RGBA8888 color
    void border_color(gfx::rgba_pixel<32> value) {
        convert(value, &m_border_color);
        this->invalidate();
    }
    /// @brief Indicates the border width of the button
    /// @return The width in pixels
    uint16_t border_width() const {
        return m_border_width;
    }
    /// @brief Sets the border width of the button
    /// @param value The width in pixels
    void border_width(uint16_t value) {
        m_border_width = value;
        this->invalidate();
    }
    /// @brief Indicates the radiuses of the button edge
    /// @return The radiuses in pixels
    size16 radiuses() const {
        return m_radiuses;
    }
    /// @brief Sets the radiuses of the button edge
    /// @param value The width in pixels
    void radiuses(size16 value) {
        m_radiuses = value;
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
};
}  // namespace uix
#endif
