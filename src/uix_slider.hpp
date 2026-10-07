#ifndef HTCW_UIX_SLIDER_HPP
#define HTCW_UIX_SLIDER_HPP
#include "uix_canvas_control.hpp"
#include "uix_core.hpp"
#include "uix_shape_helpers.hpp"
namespace uix {
/// @brief The shape of the slider knob
enum struct slider_shape {
    /// @brief The knob is an circle
    circle = 0,
    /// @brief The knob is a rectangle
    rect = 1
};
/// @brief An anti-aliased slider control drawn with gfx::draw (no canvas, integer math only)
/// @tparam ControlSurfaceType The type of control surface - usually the screen
template <typename ControlSurfaceType>
class slider : public control<ControlSurfaceType> {
   public:
    using type = slider;
    using base_type = control<ControlSurfaceType>;
    using pixel_type = typename ControlSurfaceType::pixel_type;
    using palette_type = typename ControlSurfaceType::palette_type;
    using control_surface_type = ControlSurfaceType;
    /// @brief The callback type for when value() changes
    typedef void (*on_value_changed_callback_type)(uint16_t value, void* state);
    typedef void (*on_released_callback_type)(void* state);

   private:
    gfx::mask_draw_cache m_local_dc;
    gfx::mask_draw_cache* m_dc;
    uix_pixel m_knob_color, m_knob_border_color;
    uint16_t m_knob_border_width;
    slider_shape m_knob_shape;
    size16 m_knob_radiuses;
    uix_pixel m_bar_color, m_bar_border_color;
    uint16_t m_bar_border_width;
    uint16_t m_bar_width;
    size16 m_bar_radiuses;
    uix_orientation m_orientation;
    uint16_t m_minimum, m_maximum, m_value;
    on_value_changed_callback_type m_on_value_changed_cb;
    void* m_on_value_changed_state;
    on_released_callback_type m_on_released_cb;
    void* m_on_released_state;
    void do_copy_fields(const slider& rhs) {
        m_knob_color = rhs.m_knob_color;
        m_knob_border_color = rhs.m_knob_border_color;
        m_knob_border_width = rhs.m_knob_border_width;
        m_knob_shape = rhs.m_knob_shape;
        m_knob_radiuses = rhs.m_knob_radiuses;
        m_bar_color = rhs.m_bar_color;
        m_bar_border_color = rhs.m_bar_border_color;
        m_bar_border_width = rhs.m_bar_border_width;
        m_bar_width = rhs.m_bar_width;
        m_bar_radiuses = rhs.m_bar_radiuses;
        m_orientation = rhs.m_orientation;
        m_minimum = rhs.m_minimum;
        m_maximum = rhs.m_maximum;
        m_value = rhs.m_value;
        m_on_value_changed_cb = rhs.m_on_value_changed_cb;
        m_on_value_changed_state = rhs.m_on_value_changed_state;
        m_on_released_cb = rhs.m_on_released_cb;
        m_on_released_state = rhs.m_on_released_state;
        // keep a caller-supplied cache; otherwise point at our own
        m_dc = (rhs.m_dc == &rhs.m_local_dc) ? &m_local_dc : rhs.m_dc;
    }
    void validate_values() {
        if (m_maximum < m_minimum) {
            uint16_t tmp = m_maximum;
            m_maximum = m_minimum;
            m_minimum = tmp;
        }
    }
    bool horizontal() const {
        return m_orientation == uix_orientation::horizontal;
    }
    // extent along the direction of travel
    int16_t length_dim() const {
        return (int16_t)(horizontal() ? this->dimensions().width : this->dimensions().height);
    }
    // extent across the direction of travel (== knob diameter)
    int16_t cross_dim() const {
        return (int16_t)(horizontal() ? this->dimensions().height : this->dimensions().width);
    }
    // how far the knob's leading edge can move, in pixels
    int16_t travel() const {
        const int16_t t = length_dim() - cross_dim();
        return t < 0 ? 0 : t;
    }
    // knob leading-edge offset along the travel axis. vertical: max at top.
    int16_t knob_pos() const {
        uint16_t lo = m_minimum, hi = m_maximum;
        if (hi < lo) { uint16_t t = lo; lo = hi; hi = t; }
        uint16_t v = m_value;
        if (v < lo) v = lo;
        if (v > hi) v = hi;
        const uint32_t range = (uint32_t)(hi - lo);
        const int16_t tr = travel();
        int16_t p = 0;
        if (range != 0) {
            p = (int16_t)(((uint32_t)(v - lo) * (uint32_t)tr + (range >> 1)) / range);
        }
        if (!horizontal()) {
            p = tr - p;
        }
        return p;
    }
    using shapes = helpers::shape_helpers;
    void draw_knob(ControlSurfaceType& dst, const srect16& clip) {
        const int16_t d = cross_dim();
        if (d <= 0) return;
        const int16_t pos = knob_pos();
        const int16_t bw = (int16_t)m_knob_border_width;
        if (m_knob_shape == slider_shape::circle) {
            const srect16 kr = horizontal() ? srect16(pos, 0, pos + d - 1, d - 1)
                                            : srect16(0, pos, d - 1, pos + d - 1);
            shapes::draw_bordered(dst, kr, m_knob_color, m_knob_border_color, bw, 0, true, m_dc, clip);
        } else {
            // half as thick along the travel axis as across it, centered where the disc would be
            int16_t t = d / 2;
            if (t < 1) t = 1;
            const int16_t s = pos + (d - t) / 2;
            const srect16 kr = horizontal() ? srect16(s, 0, s + t - 1, d - 1)
                                            : srect16(0, s, d - 1, s + t - 1);
            shapes::draw_bordered(dst, kr, m_knob_color, m_knob_border_color, bw, shapes::uniform_radius(m_knob_radiuses), false, m_dc, clip);
        }
    }
    void draw_bar(ControlSurfaceType& dst, const srect16& clip) {
        const int16_t d = cross_dim();
        const int16_t len = length_dim();
        if (d <= 0 || len <= 0) return;
        const int16_t bw = (int16_t)m_bar_border_width;
        int16_t thick = (int16_t)m_bar_width + 2 * bw;  // border is stroked inward
        if (thick > d) thick = d;
        const int16_t c1 = (d - thick) / 2;  // centered across
        const int16_t m = d / 20;            // small end margin
        const srect16 br = horizontal() ? srect16(m, c1, len - 1 - m, c1 + thick - 1)
                                        : srect16(c1, m, c1 + thick - 1, len - 1 - m);
        shapes::draw_bordered(dst, br, m_bar_color, m_bar_border_color, bw, shapes::uniform_radius(m_bar_radiuses), false, m_dc, clip);
    }

   protected:
    /// @brief For derivative classes, moves the control
    /// @param rhs The slider to move
    void do_move_control(slider& rhs) {
        this->base_type::do_move_control(rhs);
        do_copy_fields(rhs);
        rhs.m_on_value_changed_cb = nullptr;
        rhs.m_on_released_cb = nullptr;
    }
    /// @brief For derivative classes, copies the control
    /// @param rhs The slider to copy
    void do_copy_control(const slider& rhs) {
        this->base_type::do_copy_control(rhs);
        do_copy_fields(rhs);
    }

   public:
    /// @brief Indicates the draw cache used for the slider
    /// @return The mask_draw_cache
    gfx::mask_draw_cache& draw_cache() const {
        return *m_dc;
    }
    /// @brief Sets the draw cache used for the slider
    /// @param value The mask_draw_cache
    void draw_cache(gfx::mask_draw_cache& value) {
        m_dc = &value;
    }
    /// @brief Moves a slider control
    /// @param rhs The control to move
    slider(slider&& rhs) {
        do_move_control(rhs);
    }
    /// @brief Moves a slider control
    /// @param rhs The control to move
    /// @return this
    slider& operator=(slider&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    /// @brief Copies a slider control
    /// @param rhs The control to copy
    slider(const slider& rhs) {
        do_copy_control(rhs);
    }
    /// @brief Copies a slider control
    /// @param rhs The control to copy
    /// @return this
    slider& operator=(const slider& rhs) {
        do_copy_control(rhs);
        return *this;
    }
    /// @brief Constructs a slider from a given parent with an optional palette
    /// @param parent The parent the control is bound to - usually the screen
    /// @param palette The palette associated with the control. This is usually the screen's palette.
    slider(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent, palette), m_dc(&m_local_dc), m_knob_border_width(1), m_knob_shape(slider_shape::circle), m_knob_radiuses(0, 0), m_bar_border_width(1), m_bar_width(5), m_bar_radiuses(2, 2), m_orientation(uix_orientation::horizontal), m_minimum(0), m_maximum(100), m_value(0), m_on_value_changed_cb(nullptr), m_on_value_changed_state(nullptr), m_on_released_cb(nullptr), m_on_released_state(nullptr) {
        m_knob_color = uix_pixel(255, 255, 255, 255);
        m_knob_border_color = uix_pixel(0, 0, 0, 255);
        m_bar_color = uix_pixel(255, 255, 255, 255);
        m_bar_border_color = uix_pixel(0, 0, 0, 255);
    }
    /// @brief Constructs a slider
    slider() : base_type(), m_dc(&m_local_dc), m_knob_border_width(1), m_knob_shape(slider_shape::circle), m_knob_radiuses(0, 0), m_bar_border_width(1), m_bar_width(5), m_bar_radiuses(2, 2), m_orientation(uix_orientation::horizontal), m_minimum(0), m_maximum(100), m_value(0), m_on_value_changed_cb(nullptr), m_on_value_changed_state(nullptr), m_on_released_cb(nullptr), m_on_released_state(nullptr) {
        m_knob_color = uix_pixel(255, 255, 255, 255);
        m_knob_border_color = uix_pixel(0, 0, 0, 255);
        m_bar_color = uix_pixel(255, 255, 255, 255);
        m_bar_border_color = uix_pixel(0, 0, 0, 255);
    }
    /// @brief Indicates the color of the knob
    /// @return The color
    uix_pixel knob_color() const {
        return m_knob_color;
    }
    /// @brief Sets the color of the knob
    /// @param value The color
    void knob_color(uix_pixel value) {
        m_knob_color = value;
        this->invalidate();
    }
    /// @brief Indicates the color of the knob border
    /// @return The color
    uix_pixel knob_border_color() const {
        return m_knob_border_color;
    }
    /// @brief Sets the color of the knob border
    /// @param value The color
    void knob_border_color(uix_pixel value) {
        m_knob_border_color = value;
        this->invalidate();
    }
    /// @brief Indicates the width of the knob border
    /// @return The width in pixels
    uint16_t knob_border_width() const {
        return m_knob_border_width;
    }
    /// @brief Sets the width of the knob border
    /// @param value The width in pixels
    void knob_border_width(uint16_t value) {
        m_knob_border_width = value;
        this->invalidate();
    }
    /// @brief Indicates the radiuses of the knob (rect knob only; the aa routines use a single uniform radius)
    /// @return The radiuses of the knob
    size16 knob_radiuses() const {
        return m_knob_radiuses;
    }
    /// @brief Sets the radiuses of the knob (rect knob only; the aa routines use a single uniform radius)
    /// @param value The knob radiuses
    void knob_radiuses(size16 value) {
        m_knob_radiuses = value;
        this->invalidate();
    }
    /// @brief Indicates the shape of the knob
    /// @return The shape
    slider_shape knob_shape() const {
        return m_knob_shape;
    }
    /// @brief Sets the shape of the knob
    /// @param value The shape
    void knob_shape(slider_shape value) {
        m_knob_shape = value;
        this->invalidate();
    }
    /// @brief Indicates the color of the bar
    /// @return The color
    uix_pixel color() const {
        return m_bar_color;
    }
    /// @brief Sets the color of the bar
    /// @param value The color
    void color(uix_pixel value) {
        m_bar_color = value;
        this->invalidate();
    }
    /// @brief Indicates the color of the bar border
    /// @return The color
    uix_pixel border_color() const {
        return m_bar_border_color;
    }
    /// @brief Sets the color of the bar border
    /// @param value The color
    void border_color(uix_pixel value) {
        m_bar_border_color = value;
        this->invalidate();
    }
    /// @brief Indicates the width of the bar border
    /// @return The width in pixels
    uint16_t bar_border_width() const {
        return m_bar_border_width;
    }
    /// @brief Sets the width of the bar border
    /// @param value The width in pixels
    void bar_border_width(uint16_t value) {
        m_bar_border_width = value;
        this->invalidate();
    }
    /// @brief Indicates the width of the bar
    /// @return The width of the bar (inside the border)
    uint16_t bar_width() const {
        return m_bar_width;
    }
    /// @brief Sets the width of the bar
    /// @param value The width of the bar in pixels (inside the border)
    void bar_width(uint16_t value) {
        m_bar_width = value;
        this->invalidate();
    }
    /// @brief Indicates the radiuses of the bar (the aa routines use a single uniform radius)
    /// @return The radiuses of the bar
    size16 bar_radiuses() const {
        return m_bar_radiuses;
    }
    /// @brief Sets the radiuses of the bar (the aa routines use a single uniform radius)
    /// @param value The bar radiuses
    void bar_radiuses(size16 value) {
        m_bar_radiuses = value;
        this->invalidate();
    }
    /// @brief Indicates the orientation of the slider
    /// @return The slider orientation - vertical or horizontal
    uix_orientation orientation() const {
        return m_orientation;
    }
    /// @brief Sets the orientation of the slider
    /// @param value The slider orientation - vertical or horizontal
    void orientation(uix_orientation value) {
        if (m_orientation != value) {
            m_orientation = value;
            this->invalidate();
        }
    }
    /// @brief Indicates the minimum value
    /// @return The minimum value
    uint16_t minimum() const {
        return m_minimum;
    }
    /// @brief Sets the minimum value
    /// @param value The minimum value
    void minimum(uint16_t value) {
        m_minimum = value;
        this->invalidate();
    }
    /// @brief Indicates the maximum value
    /// @return The maximum value
    uint16_t maximum() const {
        return m_maximum;
    }
    /// @brief Sets the maximum value
    /// @param value The maximum value
    void maximum(uint16_t value) {
        m_maximum = value;
        this->invalidate();
    }
    /// @brief Indicates the value
    /// @return The value
    uint16_t value() const {
        return m_value;
    }
    /// @brief Sets the value
    /// @param value The value
    void value(uint16_t value) {
        if (value < m_minimum || value > m_maximum) {
            return;
        }
        if (m_value != value) {
            const int16_t old_pos = knob_pos();
            m_value = value;
            if (m_on_value_changed_cb != nullptr) {
                m_on_value_changed_cb(value, m_on_value_changed_state);
            }
            // only repaint if the knob actually moves on screen
            if (knob_pos() != old_pos) {
                this->invalidate();
            }
        }
    }
    /// @brief Indicates the callback for when the value changes
    /// @return The pointer to the callback
    on_value_changed_callback_type on_value_changed_callback() const {
        return m_on_value_changed_cb;
    }
    /// @brief Sets the callback for when the value changes
    /// @param callback The callback to invoke when the value changes
    /// @param state Any user defined state to pass along with the callback
    void on_value_changed_callback(on_value_changed_callback_type callback, void* state = nullptr) {
        m_on_value_changed_cb = callback;
        m_on_value_changed_state = state;
    }
    /// @brief Indicates the callback for when the slider is released
    /// @return The pointer to the callback
    on_released_callback_type on_released_callback() const {
        return m_on_released_cb;
    }
    /// @brief Sets the callback for when the slider is released
    /// @param callback The callback to invoke when the slider is released
    /// @param state Any user defined state to pass along with the callback
    void on_released_callback(on_released_callback_type callback, void* state = nullptr) {
        m_on_released_cb = callback;
        m_on_released_state = state;
    }
    /// @brief Called before the control is rendered.
    virtual void on_before_paint() override {
        validate_values();
    }

   protected:
    /// @brief Called when the slider is painted
    /// @param destination The draw destination
    /// @param clip The clipping rectangle
    virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
        draw_bar(destination, clip);
        draw_knob(destination, clip);
    }
    /// @brief Called when the slider is touched
    /// @param locations_size The count of locations (only the first one is respected)
    /// @param locations The locations
    /// @return True, because it was handled
    virtual bool on_touch(size_t locations_size, const spoint16* locations) override {
        const int16_t tr = travel();
        uint16_t lo = m_minimum, hi = m_maximum;
        if (hi < lo) { uint16_t t = lo; lo = hi; hi = t; }
        uint16_t v = lo;
        if (tr > 0) {
            // center the knob on the touch point
            int32_t t = (int32_t)(horizontal() ? locations->x : locations->y) - (cross_dim() / 2);
            if (t < 0) t = 0;
            if (t > tr) t = tr;
            if (!horizontal()) t = tr - t;
            const uint32_t range = (uint32_t)(hi - lo);
            v = (uint16_t)(lo + ((uint32_t)t * range + ((uint32_t)tr >> 1)) / (uint32_t)tr);
        }
        value(v);
        return true;
    }
    /// @brief Called when the slider is released
    virtual void on_release() override {
        if (m_on_released_cb != nullptr) {
            m_on_released_cb(m_on_released_state);
        }
    }
};
}  // namespace uix
#endif