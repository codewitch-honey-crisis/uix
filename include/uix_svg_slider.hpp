#ifndef HTCW_UIX_SVG_SLIDER
#define HTCW_UIX_SVG_SLIDER
#include "uix_core.hpp"
namespace uix {
/// @brief The shape of the slider knob
enum struct svg_slider_shape {
    /// @brief The knob is an ellipse
    ellipse = 0,
    /// @brief The knob is a rectangle
    rect = 1
};
/// @brief An SVG slider control
/// @tparam ControlSurfaceType The type of control surface - usually the screen
template <typename ControlSurfaceType>
class svg_slider : public control<ControlSurfaceType> {
   public:
    using type = svg_slider;
    using base_type = control<ControlSurfaceType>;
    using pixel_type = typename ControlSurfaceType::pixel_type;
    using palette_type = typename ControlSurfaceType::palette_type;
    using control_surface_type = ControlSurfaceType;
    /// @brief The callback type for when value() changes
    typedef void (*on_value_changed_callback_type)(void* state);

   private:
    gfx::svg_doc m_bar, m_knob;
    bool m_knob_dirty, m_bar_dirty;
    gfx::rgba_pixel<32> m_knob_color, m_knob_border_color;
    uint16_t m_knob_border_width;
    svg_slider_shape m_knob_shape;
    sizef m_knob_radiuses;
    gfx::rgba_pixel<32> m_bar_color, m_bar_border_color;
    uint16_t m_bar_border_width;
    uint16_t m_bar_width;
    sizef m_bar_radiuses;
    uix_orientation m_orientation;
    uint16_t m_minimum, m_maximum, m_value;
    on_value_changed_callback_type m_on_value_changed_cb;
    void* m_on_value_changed_state;
    void do_copy_fields(const svg_slider& rhs) {
        m_bar_dirty = true;
        m_knob_dirty = true;
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
    }
    void validate_values() {
        if (m_maximum < m_minimum) {
            uint16_t tmp = m_maximum;
            m_maximum = m_minimum;
            m_minimum = tmp;
        }
        if (m_value < m_minimum) {
            m_value = m_minimum;
        }
        if (m_value > m_maximum) {
            m_value = m_maximum;
        }
    }
    void rebuild_knob() {
        if (m_knob_dirty) {
            float radius;
            if(m_orientation==uix_orientation::horizontal) {
                radius = this->bounds().height() * 0.5f;
            } else {
                radius = this->bounds().width() * 0.5f;
            }
            gfx::svg_doc_builder b(sizef(radius * 2, radius * 2));
            gfx::svg_shape_info si;
            si.fill.type = gfx::svg_paint_type::color;
            si.fill.color = m_knob_color;
            si.stroke_width = m_knob_border_width;
            si.stroke.type = gfx::svg_paint_type::color;
            si.stroke.color = m_knob_border_color;
            if (m_knob_radiuses.width == 0 && m_knob_radiuses.height == 0) {
                if (m_knob_shape == svg_slider_shape::ellipse) {
                    b.add_ellipse({radius, radius}, {radius - 1.0f, radius - 1.0f}, si);
                } else if (m_knob_shape == svg_slider_shape::rect) {
                    rectf r(pointf(radius, radius), (radius * 0.5f) - 1.0f);
                    if(m_orientation==uix_orientation::horizontal) {
                        r.x2 = r.x1 + radius - 1.0f;
                    } else {
                        r.y2 = r.y1 + radius - 1.0f;
                    }
                    b.add_rectangle(r, si);
                }
            } else {
                if (m_knob_shape == svg_slider_shape::ellipse) {
                    b.add_ellipse({radius, m_knob_radiuses.height + 1.0f}, m_knob_radiuses, si);
                } else if (m_knob_shape == svg_slider_shape::rect) {
                    rectf r(pointf(radius, radius), radius - 1);
                    if (m_orientation == uix_orientation::horizontal) {
                        r.x2 = r.x1 + radius - 1.0f;
                    } else {
                        r.y2 = r.y1 + radius - 1.0f;
                    }
                    b.add_rounded_rectangle(r, m_knob_radiuses, si);
                }
            }
            b.to_doc(&m_knob);
            m_knob_dirty = false;
        }
    }
    void rebuild_bar() {
        if (m_bar_dirty) {
            float radius;
            rectf bounds;
            if(m_orientation==uix_orientation::horizontal) {
                radius = this->dimensions().height * .05f;
                bounds = rectf(radius, 0, this->dimensions().width - 1 - radius, m_bar_width - 1);
                bounds.y1 = (this->dimensions().height - m_bar_width) * 0.5f;
                bounds.y2 = bounds.y1 + m_bar_width - 1;
            } else {
                radius = this->dimensions().width * .05f;
                bounds = rectf(0, radius, m_bar_width - 1, this->dimensions().height - 1 - radius);
                bounds.x1 = (this->dimensions().width - m_bar_width) * 0.5f;
                bounds.x2 = bounds.x1 + m_bar_width - 1;
            }
            
            gfx::svg_doc_builder b(bounds.dimensions());
            gfx::svg_shape_info si;
            si.fill.type = gfx::svg_paint_type::color;
            si.fill.color = m_bar_color;
            si.stroke_width = m_bar_border_width;
            si.stroke.type = gfx::svg_paint_type::color;
            si.stroke.color = m_bar_border_color;
            if (m_bar_radiuses.width != 0 || m_bar_radiuses.height != 0) {
                b.add_rounded_rectangle(bounds, m_bar_radiuses, si);
            } else {
                b.add_rectangle(bounds, si);
            }
            b.to_doc(&m_bar);
            m_bar_dirty = false;
        }
    }

   protected:
    /// @brief For derivative classes, moves the control
    /// @param rhs The slider to move
    void do_move_control(svg_slider& rhs) {
        this->base_type::do_move_control(rhs);
        do_copy_fields(rhs);
        m_bar_dirty = true;
        m_knob_dirty = true;
        rhs.m_on_value_changed_cb = nullptr;
    }
    /// @brief For derivative classes, copies the control
    /// @param rhs The slider to copy
    void do_copy_control(const svg_slider& rhs) {
        this->base_type::do_copy_control(rhs);
        do_copy_fields(rhs);
        m_bar_dirty = true;
        m_knob_dirty = true;
    }

   public:
    /// @brief Moves a slider control
    /// @param rhs The control to move
    svg_slider(svg_slider&& rhs) {
        do_move_control(rhs);
    }
    /// @brief Moves a slider control
    /// @param rhs The control to move
    /// @return this
    svg_slider& operator=(svg_slider&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    /// @brief Copies a slider control
    /// @param rhs The control to copy
    svg_slider(const svg_slider& rhs) {
        do_copy_control(rhs);
    }
    /// @brief Copies a slider control
    /// @param rhs The control to copy
    /// @return this
    svg_slider& operator=(const svg_slider& rhs) {
        do_copy_control(rhs);
        return *this;
    }
    /// @brief Constructs a slider from a given parent with an optional palette
    /// @param parent The parent the control is bound to - usually the screen
    /// @param palette The palette associated with the control. This is usually the screen's palette.
    svg_slider(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent, palette), m_knob_dirty(true), m_bar_dirty(true), m_knob_border_width(1), m_knob_shape(svg_slider_shape::ellipse), m_knob_radiuses(0, 0), m_bar_border_width(1), m_bar_width(5), m_bar_radiuses(2, 2), m_minimum(0), m_maximum(100), m_value(0), m_on_value_changed_cb(nullptr), m_on_value_changed_state(nullptr) {
        m_knob_color = gfx::rgba_pixel<32>(255, 255, 255, 255);
        m_knob_border_color = gfx::rgba_pixel<32>(0, 0, 0, 255);
        m_bar_color = gfx::rgba_pixel<32>(255, 255, 255, 255);
        m_bar_border_color = gfx::rgba_pixel<32>(0, 0, 0, 255);
    }
    /// @brief Indicates the color of the knob
    /// @return The color
    gfx::rgba_pixel<32> knob_color() const {
        return m_knob_color;
    }
    /// @brief Sets the color of the knob
    /// @param value The color
    void knob_color(gfx::rgba_pixel<32> value) {
        m_knob_color = value;
        m_knob_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the knob border
    /// @return The color
    gfx::rgba_pixel<32> knob_border_color() const {
        return m_knob_border_color;
    }
    /// @brief Sets the color of the knob border
    /// @param value The color
    void knob_border_color(gfx::rgba_pixel<32> value) {
        m_knob_border_color = value;
        m_knob_dirty = true;
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
        m_knob_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the radiuses of the knob
    /// @return The radiuses of the knob
    sizef knob_radiuses() const {
        return m_knob_radiuses;
    }
    /// @brief Sets the radiuses of the knob
    /// @param value The knob radiuses
    void knob_radiuses(sizef value) {
        m_knob_radiuses = value;
        m_knob_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the shape of the knob
    /// @return The shape
    svg_slider_shape knob_shape() const {
        return m_knob_shape;
    }
    /// @brief Sets the shape of the knob
    /// @param value The shape
    void knob_shape(svg_slider_shape value) {
        m_knob_shape = value;
        m_knob_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the bar
    /// @return The color
    gfx::rgba_pixel<32> bar_color() const {
        return m_bar_color;
    }
    /// @brief Sets the color of the bar
    /// @param value The color
    void bar_color(gfx::rgba_pixel<32> value) {
        m_bar_color = value;
        m_bar_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the bar border
    /// @return The color
    gfx::rgba_pixel<32> bar_border_color() const {
        return m_bar_border_color;
    }
    /// @brief Sets the color of the bar border
    /// @param value The color
    void bar_border_color(gfx::rgba_pixel<32> value) {
        m_bar_border_color = value;
        m_bar_dirty = true;
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
        m_bar_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the width of the bar
    /// @return The height of the bar
    uint16_t bar_width() const {
        return m_bar_width;
    }
    /// @brief Sets the width of the bar
    /// @param value The bar height in pixels
    void bar_width(uint16_t value) {
        m_bar_width = value;
        m_bar_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the radiuses of the bar
    /// @return The radiuses of the bar
    sizef bar_radiuses() const {
        return m_bar_radiuses;
    }
    /// @brief Sets the radiuses of the bar
    /// @param value The bar radiuses
    void bar_radiuses(sizef value) {
        m_bar_radiuses = value;
        m_bar_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the orientation of the slider
    /// @return The slider orientation - vertical or horizontal
    uix_orientation orientation() const {
        return m_orientation;
    }
    /// @brief Sets the orientation of the slider
    /// @param value The slider orientation - vertical or horizontal
    void orienation(uix_orientation value) {
        if(m_orientation!=value) {
            m_orientation = value;
            m_knob_dirty = true;
            m_bar_dirty = true;
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
        if (m_value != value) {
            m_value = value;
            if (m_on_value_changed_cb != nullptr) {
                m_on_value_changed_cb(m_on_value_changed_state);
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
    /// @brief Called before the control is rendered.
    virtual void on_before_render() override {
        validate_values();
        rebuild_bar();
        rebuild_knob();
    }
    /// @brief Called when the slider is painted
    /// @param destination The draw destination
    /// @param clip The clipping rectangle
    virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
        gfx::draw::svg(destination, destination.bounds(), m_bar, 1.0f, &clip);
        const uint16_t range = (m_maximum - m_minimum) + 1;
        const uint16_t offset_value = m_value - m_minimum;
        const float mult = (float)offset_value / (float)range;
        int16_t adj;
        if(m_orientation==uix_orientation::horizontal) {
            if (m_knob_shape == svg_slider_shape::ellipse) {
                adj = m_knob_radiuses.width == 0 ? -(m_knob.dimensions().width * 0.5f) : -m_knob_radiuses.width;
            } else {
                adj = -(m_knob.dimensions().height / 4 + 1);
            }
            const uint16_t scr_range = destination.dimensions().width - 1 + (adj * 2);
            gfx::draw::svg(destination, destination.bounds().offset(mult * (scr_range), 0), m_knob, 1.0f, &clip);
        } else {
            if (m_knob_shape == svg_slider_shape::ellipse) {
                adj = m_knob_radiuses.height == 0 ? -(m_knob.dimensions().height * 0.5f) : -m_knob_radiuses.height;
            } else {
                adj = -(m_knob.dimensions().width / 4 + 1);
            }
            const uint16_t scr_range = destination.dimensions().height - 1 + (adj * 2);
            gfx::draw::svg(destination, destination.bounds().offset(0,scr_range-(mult * (scr_range))-1), m_knob, 1.0f, &clip);
        }
    }
    /// @brief Called when the slider is touched
    /// @param locations_size The count of locations (only the first one is respected)
    /// @param locations The locations
    /// @return True, because it was handled
    virtual bool on_touch(size_t locations_size, const spoint16* locations) override {
        float ext;
        float i;
        if(m_orientation==uix_orientation::horizontal) {
            i = locations->x;
            ext = this->dimensions().width;
            if (i < 0.0f) i = 0.0f;
            if (i >= ext) {
                i = ext - 1;
            }
        } else {
            i = locations->y;
            ext = this->dimensions().height;
            if (i < 0.0f) i = 0.0f;
            if (i >= ext) {
                i = ext - 1;
            }
            i=ext-i-1;
        }
        const float mult = i / (float)(ext - 1);
        const int range = m_maximum - m_minimum;
        int v = mult * range;
        if (v > maximum()) {
            v = maximum();
        }
        if (v < minimum()) {
            v = minimum();
        }
        value(mult * range);
        return true;
    }
};
}  // namespace uix
#endif  // HTCW_UIX_SVG_SLIDER