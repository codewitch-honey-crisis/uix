#ifndef HTCW_UIX_SWITCH_HPP
#define HTCW_UIX_SWITCH_HPP
#include "uix_core.hpp"
#include "uix_canvas_control.hpp"
#include "uix_shape_helpers.hpp"
namespace uix {
/// @brief The shape of the switch knob
enum struct switch_shape {
    /// @brief The knob is a circle
    circle = 0,
    /// @brief The knob is a square
    square = 1
};
/// @brief An anti-aliased switch control drawn with gfx::draw (no canvas, integer math only)
/// @tparam ControlSurfaceType The type of control surface - usually the screen
template <typename ControlSurfaceType>
class switch_control : public control<ControlSurfaceType> {
   public:
    using type = switch_control;
    using base_type = control<ControlSurfaceType>;
    using pixel_type = typename ControlSurfaceType::pixel_type;
    using palette_type = typename ControlSurfaceType::palette_type;
    using control_surface_type = ControlSurfaceType;
    /// @brief The callback type for when value() changes
    /// @param value The new value
    typedef void (*on_value_changed_callback_type)(bool value, void* state);

   private:
    gfx::mask_draw_cache m_local_dc;
    gfx::mask_draw_cache* m_dc;
    uix_pixel m_knob_color, m_knob_border_color;
    uint16_t m_knob_border_width;
    switch_shape m_knob_shape;
    size16 m_knob_radiuses;
    uix_pixel m_background_color, m_border_color;
    uint16_t m_border_width;
    size16 m_radiuses;
    uix_orientation m_orientation;
    bool m_value;
    on_value_changed_callback_type m_on_value_changed_cb;
    void* m_on_value_changed_state;
    void do_copy_fields(const switch_control& rhs) {
        m_knob_color = rhs.m_knob_color;
        m_knob_border_color = rhs.m_knob_border_color;
        m_knob_border_width = rhs.m_knob_border_width;
        m_knob_shape = rhs.m_knob_shape;
        m_knob_radiuses = rhs.m_knob_radiuses;
        m_background_color = rhs.m_background_color;
        m_border_color = rhs.m_border_color;
        m_border_width = rhs.m_border_width;
        m_radiuses = rhs.m_radiuses;
        m_orientation = rhs.m_orientation;
        m_value = rhs.m_value;
        m_on_value_changed_cb = rhs.m_on_value_changed_cb;
        m_on_value_changed_state = rhs.m_on_value_changed_state;
        // keep a caller-supplied cache; otherwise point at our own
        m_dc = (rhs.m_dc == &rhs.m_local_dc) ? &m_local_dc : rhs.m_dc;
    }
    bool horizontal() const {
        return m_orientation == uix_orientation::horizontal;
    }
    // extent along the direction of travel
    int16_t length_dim() const {
        return (int16_t)(horizontal() ? this->dimensions().width : this->dimensions().height);
    }
    // extent across the direction of travel (== the knob's square cell)
    int16_t cross_dim() const {
        return (int16_t)(horizontal() ? this->dimensions().height : this->dimensions().width);
    }
    using shapes = helpers::shape_helpers;
    void draw_backing(ControlSurfaceType& dst, const srect16& clip) {
        const srect16 r(0, 0, (int16_t)this->dimensions().width - 1, (int16_t)this->dimensions().height - 1);
        shapes::draw_bordered(dst, r, m_background_color, m_border_color, (int16_t)m_border_width, shapes::uniform_radius(m_radiuses), false, m_dc, clip);
    }
    void draw_knob(ControlSurfaceType& dst, const srect16& clip) {
        const int16_t d = cross_dim();
        const int16_t len = length_dim();
        if (d <= 0 || len <= 0) return;
        // the knob sits in a d x d cell at one end: on = right (horizontal) / top (vertical)
        int16_t cell = 0;
        if (horizontal()) {
            if (m_value) cell = len - d;
        } else {
            if (!m_value) cell = len - d;
        }
        if (cell < 0) cell = 0;
        // knob is 70% of the cell, centered in it
        int16_t kd = (int16_t)(((int32_t)d * 7 + 5) / 10);
        if (kd < 1) kd = 1;
        const int16_t off = (d - kd) / 2;
        const int16_t a = cell + off;  // along the travel axis
        const srect16 kr = horizontal() ? srect16(a, off, a + kd - 1, off + kd - 1)
                                        : srect16(off, a, off + kd - 1, a + kd - 1);
        const bool circle = m_knob_shape == switch_shape::circle;
        shapes::draw_bordered(dst, kr, m_knob_color, m_knob_border_color, (int16_t)m_knob_border_width,
                              circle ? 0 : shapes::uniform_radius(m_knob_radiuses), circle, m_dc, clip);
    }

   protected:
    /// @brief For derivative classes, moves the control
    /// @param rhs The switch to move
    void do_move_control(switch_control& rhs) {
        this->base_type::do_move_control(rhs);
        do_copy_fields(rhs);
        rhs.m_on_value_changed_cb = nullptr;
    }
    /// @brief For derivative classes, copies the control
    /// @param rhs The switch to copy
    void do_copy_control(const switch_control& rhs) {
        this->base_type::do_copy_control(rhs);
        do_copy_fields(rhs);
    }

   public:
    /// @brief Indicates the draw cache used for the switch
    /// @return The mask_draw_cache
    gfx::mask_draw_cache& draw_cache() const {
        return *m_dc;
    }
    /// @brief Sets the draw cache used for the switch
    /// @param value The mask_draw_cache
    void draw_cache(gfx::mask_draw_cache& value) {
        m_dc = &value;
    }
    /// @brief Moves a switch control
    /// @param rhs The control to move
    switch_control(switch_control&& rhs) {
        do_move_control(rhs);
    }
    /// @brief Moves a switch control
    /// @param rhs The control to move
    /// @return this
    switch_control& operator=(switch_control&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    /// @brief Copies a switch control
    /// @param rhs The control to copy
    switch_control(const switch_control& rhs) {
        do_copy_control(rhs);
    }
    /// @brief Copies a switch control
    /// @param rhs The control to copy
    /// @return this
    switch_control& operator=(const switch_control& rhs) {
        do_copy_control(rhs);
        return *this;
    }
    /// @brief Constructs a switch from a given parent with an optional palette
    /// @param parent The parent the control is bound to - usually the screen
    /// @param palette The palette associated with the control. This is usually the screen's palette.
    switch_control(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent, palette), m_dc(&m_local_dc), m_knob_border_width(1), m_knob_shape(switch_shape::circle), m_knob_radiuses(2, 2), m_border_width(1), m_radiuses(2, 2), m_orientation(uix_orientation::horizontal), m_value(false), m_on_value_changed_cb(nullptr), m_on_value_changed_state(nullptr) {
        m_knob_color = uix_pixel(255, 255, 255, 255);
        m_knob_border_color = uix_pixel(0, 0, 0, 255);
        m_background_color = uix_pixel(255, 255, 255, 255);
        m_border_color = uix_pixel(0, 0, 0, 255);
    }
    /// @brief Constructs a switch
    switch_control() : base_type(), m_dc(&m_local_dc), m_knob_border_width(1), m_knob_shape(switch_shape::circle), m_knob_radiuses(2, 2), m_border_width(1), m_radiuses(2, 2), m_orientation(uix_orientation::horizontal), m_value(false), m_on_value_changed_cb(nullptr), m_on_value_changed_state(nullptr) {
        m_knob_color = uix_pixel(255, 255, 255, 255);
        m_knob_border_color = uix_pixel(0, 0, 0, 255);
        m_background_color = uix_pixel(255, 255, 255, 255);
        m_border_color = uix_pixel(0, 0, 0, 255);
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
    /// @brief Indicates the shape of the knob
    /// @return The shape
    switch_shape knob_shape() const {
        return m_knob_shape;
    }
    /// @brief Sets the shape of the knob
    /// @param value The shape
    void knob_shape(switch_shape value) {
        m_knob_shape = value;
        this->invalidate();
    }
    /// @brief Indicates the radiuses of the knob (square knob only; the aa routines use a single uniform radius)
    /// @return The radiuses of the knob
    size16 knob_radiuses() const {
        return m_knob_radiuses;
    }
    /// @brief Sets the radiuses of the knob (square knob only; the aa routines use a single uniform radius)
    /// @param value The knob radiuses
    void knob_radiuses(size16 value) {
        m_knob_radiuses = value;
        this->invalidate();
    }
    /// @brief Indicates the color of the switch background
    /// @return The color
    uix_pixel background_color() const {
        return m_background_color;
    }
    /// @brief Sets the color of the switch background
    /// @param value The color
    void background_color(uix_pixel value) {
        m_background_color = value;
        this->invalidate();
    }
    /// @brief Indicates the color of the border
    /// @return The color
    uix_pixel border_color() const {
        return m_border_color;
    }
    /// @brief Sets the color of the border
    /// @param value The color
    void border_color(uix_pixel value) {
        m_border_color = value;
        this->invalidate();
    }
    /// @brief Indicates the width of the border
    /// @return The width in pixels
    uint16_t border_width() const {
        return m_border_width;
    }
    /// @brief Sets the width of the border
    /// @param value The width in pixels
    void border_width(uint16_t value) {
        m_border_width = value;
        this->invalidate();
    }
    /// @brief Indicates the radiuses of the backing (the aa routines use a single uniform radius)
    /// @return The radiuses
    size16 radiuses() const {
        return m_radiuses;
    }
    /// @brief Sets the radiuses of the backing (the aa routines use a single uniform radius)
    /// @param value The radiuses
    void radiuses(size16 value) {
        m_radiuses = value;
        this->invalidate();
    }
    /// @brief Indicates the orientation of the switch
    /// @return The switch orientation - vertical or horizontal
    uix_orientation orientation() const {
        return m_orientation;
    }
    /// @brief Sets the orientation of the switch
    /// @param value The switch orientation - vertical or horizontal
    void orientation(uix_orientation value) {
        if (m_orientation != value) {
            m_orientation = value;
            this->invalidate();
        }
    }
    /// @brief Indicates the value
    /// @return The value
    bool value() const {
        return m_value;
    }
    /// @brief Sets the value
    /// @param value The value
    void value(bool value) {
        if (m_value != value) {
            m_value = value;
            if (m_on_value_changed_cb != nullptr) {
                m_on_value_changed_cb(m_value, m_on_value_changed_state);
            }
            this->invalidate();
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

   protected:
    /// @brief Called when the switch is painted
    /// @param destination The draw destination
    /// @param clip The clipping rectangle
    virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
        draw_backing(destination, clip);
        draw_knob(destination, clip);
    }
    /// @brief Called when the switch is touched
    /// @param locations_size The count of locations (only the first one is respected)
    /// @param locations The locations
    /// @return True, because it was handled
    virtual bool on_touch(size_t locations_size, const spoint16* locations) override {
        if (horizontal()) {
            value(locations->x > (int16_t)(this->dimensions().width / 2));
        } else {
            value(locations->y <= (int16_t)(this->dimensions().height / 2));
        }
        return true;
    }
};
}  // namespace uix
#endif  // HTCW_UIX_SWITCH_HPP