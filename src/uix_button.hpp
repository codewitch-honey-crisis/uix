#ifndef HTCW_UIX_BUTTON_HPP
#define HTCW_UIX_BUTTON_HPP
#include "uix_core.hpp"
#include "uix_shape_helpers.hpp"
namespace uix {
/// @brief An anti-aliased push button drawn with gfx::draw (no canvas, integer math only)
/// @tparam ControlSurfaceType The type of control surface - usually the screen
template <typename ControlSurfaceType>
class button : public control<ControlSurfaceType> {
   public:
    using type = button;
    using pixel_type = typename ControlSurfaceType::pixel_type;
    using palette_type = typename ControlSurfaceType::palette_type;
    using control_surface_type = ControlSurfaceType;
    typedef void (*on_pressed_changed_callback_type)(bool new_value, void* state);

   private:
    using base_type = control<ControlSurfaceType>;
    using shapes = ::uix::helpers::shape_helpers;
    gfx::mask_draw_cache m_local_dc;
    gfx::mask_draw_cache* m_dc;
    ssize16 m_padding;
    uix_justify m_text_justify;
    gfx::text_info m_text_info;
    uix_pixel m_color;
    uix_pixel m_background_color;
    uix_pixel m_border_color;
    size16 m_radiuses;
    uint16_t m_border_width;
    gfx::srect16 m_text_rect;
    bool m_text_dirty;
    on_pressed_changed_callback_type m_on_pressed_changed_callback;
    void* m_on_pressed_changed_callback_state;
    bool m_pressed;
    bool is_valid() const {
        return m_text_info.text != nullptr && m_text_info.text_font != nullptr && m_text_info.encoding != nullptr && m_text_info.text_byte_count != 0;
    }
    // The face occupies (0,0)-(w-2,h-2) when released and is shifted 1px down/right
    // when pressed, so it never leaves the control bounds.
    gfx::srect16 face_rect() const {
        const int16_t w = (int16_t)this->dimensions().width;
        const int16_t h = (int16_t)this->dimensions().height;
        gfx::srect16 r(0, 0, w - 2, h - 2);
        if (m_pressed) {
            r = r.offset(1, 1);
        }
        return r;
    }
    // Text rect relative to the released face, inside the border and padding.
    gfx::srect16 compute_rect() {
        gfx::size16 ta;
        m_text_info.text_font->measure((uint16_t)-1, m_text_info, &ta);
        const int16_t tw = (int16_t)ta.width, th = (int16_t)ta.height;
        const int16_t w = (int16_t)this->dimensions().width - 1;   // face width
        const int16_t h = (int16_t)this->dimensions().height - 1;  // face height
        const int16_t inx = (int16_t)m_border_width + m_padding.width;
        const int16_t iny = (int16_t)m_border_width + m_padding.height;
        int16_t aw = w - 2 * inx, ah = h - 2 * iny;  // content area
        if (aw < 0) aw = 0;
        if (ah < 0) ah = 0;
        int hj = 1, vj = 1;  // 0 = start, 1 = middle, 2 = end
        switch (m_text_justify) {
            case uix_justify::top_left:      hj = 0; vj = 0; break;
            case uix_justify::top_middle:    hj = 1; vj = 0; break;
            case uix_justify::top_right:     hj = 2; vj = 0; break;
            case uix_justify::center_left:   hj = 0; vj = 1; break;
            case uix_justify::center_right:  hj = 2; vj = 1; break;
            case uix_justify::bottom_left:   hj = 0; vj = 2; break;
            case uix_justify::bottom_middle: hj = 1; vj = 2; break;
            case uix_justify::bottom_right:  hj = 2; vj = 2; break;
            default: break;  // center
        }
        // integer centering; an oversized text goes negative and is clipped by the surface
        const int16_t x = inx + (hj == 0 ? 0 : hj == 1 ? (aw - tw) / 2 : aw - tw);
        const int16_t y = iny + (vj == 0 ? 0 : vj == 1 ? (ah - th) / 2 : ah - th);
        return gfx::srect16(gfx::spoint16(x, y), gfx::ssize16(tw, th));
    }
    void do_copy_fields(const button& rhs) {
        m_padding = rhs.m_padding;
        m_text_justify = rhs.m_text_justify;
        m_text_info = rhs.m_text_info;
        m_color = rhs.m_color;
        m_background_color = rhs.m_background_color;
        m_border_color = rhs.m_border_color;
        m_border_width = rhs.m_border_width;
        m_radiuses = rhs.m_radiuses;
        m_text_rect = rhs.m_text_rect;
        m_text_dirty = rhs.m_text_dirty;
        m_on_pressed_changed_callback = rhs.m_on_pressed_changed_callback;
        m_on_pressed_changed_callback_state = rhs.m_on_pressed_changed_callback_state;
        m_pressed = rhs.m_pressed;
        // keep a caller-supplied cache; otherwise point at our own
        m_dc = (rhs.m_dc == &rhs.m_local_dc) ? &m_local_dc : rhs.m_dc;
    }

   protected:
    void do_move_control(button& rhs) {
        this->base_type::do_move_control(rhs);
        do_copy_fields(rhs);
        rhs.m_on_pressed_changed_callback = nullptr;
        rhs.m_pressed = false;
    }
    void do_copy_control(const button& rhs) {
        this->base_type::do_copy_control(rhs);
        do_copy_fields(rhs);
    }
    virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
        const bool has_border = m_border_width > 0 && m_border_color != m_background_color && m_border_color.opacity8() != 0;
        shapes::draw_bordered(destination, face_rect(), m_background_color, m_border_color,
                              has_border ? (int16_t)m_border_width : 0,
                              shapes::uniform_radius(m_radiuses), false, m_dc, clip);
        if (is_valid()) {
            if (m_text_dirty) {
                m_text_rect = compute_rect();
                m_text_dirty = false;
            }
            const gfx::srect16 tr = m_text_rect.offset(m_pressed, m_pressed);
            if (shapes::overlaps(tr, clip)) {
                gfx::draw::text(destination, tr, m_text_info, m_color);
            }
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
    button() : base_type(), m_dc(&m_local_dc), m_padding({4, 4}), m_text_justify(uix_justify::center), m_text_rect(0, 0, 0, 0), m_text_dirty(true), m_on_pressed_changed_callback(nullptr), m_on_pressed_changed_callback_state(nullptr), m_pressed(false) {
        constexpr static const auto black = gfx::rgba_pixel<32>(0x0, 0x0, 0x0, 0xFF);
        constexpr static const auto gray = gfx::rgba_pixel<32>(0x7F, 0x7F, 0x7F, 0xFF);
        color(black);
        background_color(gray);
        border_color(black);
        border_width(2);
        radiuses({0, 0});
        m_text_info.text_font = nullptr;
        m_text_info.encoding = &gfx::text_encoding::utf8;
    }
    /// @brief Constructs a control given a parent and an optional palette
    /// @param parent The parent invalidation tracker - usually a screen
    /// @param palette The palette. Typically the screen's palette()
    button(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent, palette), m_dc(&m_local_dc), m_padding({4, 4}), m_text_justify(uix_justify::center), m_text_rect(0, 0, 0, 0), m_text_dirty(true), m_on_pressed_changed_callback(nullptr), m_on_pressed_changed_callback_state(nullptr), m_pressed(false) {
        constexpr static const auto black = gfx::rgba_pixel<32>(0xFF, 0xFF, 0xFF, 0xFF);
        constexpr static const auto gray = gfx::rgba_pixel<32>(0x7F, 0x7F, 0x7F, 0xFF);
        color(black);
        background_color(gray);
        border_color(black);
        border_width(2);
        radiuses({0, 0});
        m_text_info.text_font = nullptr;
        m_text_info.encoding = &gfx::text_encoding::utf8;
    }
    /// @brief Moves a button control
    /// @param rhs The control to move
    button(button&& rhs) {
        do_move_control(rhs);
    }
    /// @brief Moves a button control
    /// @param rhs The control to move
    /// @return this
    button& operator=(button&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    /// @brief Copies a button control
    /// @param rhs The control to copy
    button(const button& rhs) {
        do_copy_control(rhs);
    }
    /// @brief Copies a button control
    /// @param rhs The control to copy
    /// @return this
    button& operator=(const button& rhs) {
        do_copy_control(rhs);
        return *this;
    }
    /// @brief Indicates the draw cache used for the button
    /// @return The mask_draw_cache
    gfx::mask_draw_cache& draw_cache() const {
        return *m_dc;
    }
    /// @brief Sets the draw cache used for the button
    /// @param value The mask_draw_cache
    void draw_cache(gfx::mask_draw_cache& value) {
        m_dc = &value;
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
        m_text_dirty = true;
        this->invalidate();
    }
    /// @brief Sets the text of the button
    /// @param value the raw text data
    /// @param byte_count the number of bytes
    void text(const gfx::text_handle value, size_t byte_count) {
        m_text_info.text = value;
        m_text_info.text_byte_count = byte_count;
        m_text_dirty = true;
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
        m_text_dirty = true;
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
        m_text_dirty = true;
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
            m_text_dirty = true;
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
    void font(gfx::font& value) {
        m_text_info.text_font = &value;
        m_text_dirty = true;
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
    /// @return The width in pixels (stroked inward)
    uint16_t border_width() const {
        return m_border_width;
    }
    /// @brief Sets the border width of the button
    /// @param value The width in pixels (stroked inward)
    void border_width(uint16_t value) {
        m_border_width = value;
        m_text_dirty = true;  // the text area sits inside the border
        this->invalidate();
    }
    /// @brief Indicates the radiuses of the button edge (the aa routines use a single uniform radius)
    /// @return The radiuses in pixels
    size16 radiuses() const {
        return m_radiuses;
    }
    /// @brief Sets the radiuses of the button edge (the aa routines use a single uniform radius)
    /// @param value The radiuses in pixels
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