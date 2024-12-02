#ifndef HTCW_UIX_VSLIDER
#define HTCW_UIX_VSLIDER
#include "uix_core.hpp"
#include "uix_canvas_control.hpp"
namespace uix {
/// @brief The shape of the slider knob
enum struct vslider_shape {
    /// @brief The knob is an ellipse
    ellipse = 0,
    /// @brief The knob is a rectangle
    rect = 1
};
/// @brief A vector based slider control
/// @tparam ControlSurfaceType The type of control surface - usually the screen
template <typename ControlSurfaceType>
class vslider : public canvas_control<ControlSurfaceType> {
   public:
    using type = vslider;
    using base_type = canvas_control<ControlSurfaceType>;
    using pixel_type = typename ControlSurfaceType::pixel_type;
    using palette_type = typename ControlSurfaceType::palette_type;
    using control_surface_type = ControlSurfaceType;
    /// @brief The callback type for when value() changes
    typedef void (*on_value_changed_callback_type)(uint16_t value,void* state);
    typedef void (*on_released_callback_type)(void* state);

   private:
    gfx::vector_pixel m_knob_color, m_knob_border_color;
    uint16_t m_knob_border_width;
    vslider_shape m_knob_shape;
    size16 m_knob_radiuses;
    gfx::vector_pixel m_bar_color, m_bar_border_color;
    uint16_t m_bar_border_width;
    uint16_t m_bar_width;
    size16 m_bar_radiuses;
    uix_orientation m_orientation;
    uint16_t m_minimum, m_maximum, m_value;
    on_value_changed_callback_type m_on_value_changed_cb;
    void* m_on_value_changed_state;
    on_released_callback_type m_on_released_cb;
    void* m_on_released_state;
    void do_copy_fields(const vslider& rhs) {
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
    void draw_knob(gfx::canvas& dst,spoint16 location) {
        gfx::matrix m=dst.transform();
        dst.transform(gfx::matrix::create_translate(location.x,location.y));
        float radius;
        if(m_orientation==uix_orientation::horizontal) {
            radius = this->bounds().height() * 0.5f;
        } else {
            radius = this->bounds().width() * 0.5f;
        }
        gfx::rectf b((gfx::pointf)location,sizef(radius * 2, radius * 2));
        gfx::canvas_style si = dst.style();
        si.fill_paint_type = gfx::paint_type::solid;
        si.fill_color = m_knob_color;
        si.stroke_width = m_knob_border_width;
        si.stroke_paint_type = gfx::paint_type::solid;
        si.stroke_color = m_knob_border_color;
        dst.style(si);
        if (m_knob_radiuses.width == 0 && m_knob_radiuses.height == 0) {
            if (m_knob_shape == vslider_shape::ellipse) {
                dst.ellipse({radius, radius}, {radius - 1.0f, radius - 1.0f});
            } else if (m_knob_shape == vslider_shape::rect) {
                rectf r(pointf(radius, radius), (radius * 0.5f) - 1.0f);
                if(m_orientation==uix_orientation::horizontal) {
                    r.x2 = r.x1 + radius - 1.0f;
                } else {
                    r.y2 = r.y1 + radius - 1.0f;
                }
                dst.rectangle(r);
            }
        } else {
            if (m_knob_shape == vslider_shape::ellipse) {
                dst.ellipse({radius, radius}, (gfx::sizef)m_knob_radiuses);
            } else if (m_knob_shape == vslider_shape::rect) {
                rectf r(pointf(radius, radius), radius - 1);
                if (m_orientation == uix_orientation::horizontal) {
                    r.x2 = r.x1 + radius - 1.0f;
                } else {
                    r.y2 = r.y1 + radius - 1.0f;
                }
                dst.rounded_rectangle(r, (gfx::sizef)m_knob_radiuses);
            }
        }
        dst.render();
        dst.transform(m);
    }
    void draw_bar(gfx::canvas& dst) {
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
        
        gfx::canvas_style si = dst.style();
        si.fill_paint_type = gfx::paint_type::solid;
        si.fill_color = m_bar_color;
        si.stroke_width = m_bar_border_width;
        si.stroke_paint_type = gfx::paint_type::solid;
        si.stroke_color = m_bar_border_color;
        dst.style(si);
        if (m_bar_radiuses.width != 0 || m_bar_radiuses.height != 0) {
            dst.rounded_rectangle(bounds, (gfx::sizef)m_bar_radiuses);
        } else {
            dst.rectangle(bounds);
        }
        dst.render();
    }

   protected:
    /// @brief For derivative classes, moves the control
    /// @param rhs The slider to move
    void do_move_control(vslider& rhs) {
        this->base_type::do_move_control(rhs);
        do_copy_fields(rhs);
        rhs.m_on_value_changed_cb = nullptr;
        rhs.m_on_released_cb = nullptr;
    }
    /// @brief For derivative classes, copies the control
    /// @param rhs The slider to copy
    void do_copy_control(const vslider& rhs) {
        this->base_type::do_copy_control(rhs);
        do_copy_fields(rhs);
    }

   public:
    /// @brief Moves a slider control
    /// @param rhs The control to move
    vslider(vslider&& rhs) {
        do_move_control(rhs);
    }
    /// @brief Moves a slider control
    /// @param rhs The control to move
    /// @return this
    vslider& operator=(vslider&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    /// @brief Copies a slider control
    /// @param rhs The control to copy
    vslider(const vslider& rhs) {
        do_copy_control(rhs);
    }
    /// @brief Copies a slider control
    /// @param rhs The control to copy
    /// @return this
    vslider& operator=(const vslider& rhs) {
        do_copy_control(rhs);
        return *this;
    }
    /// @brief Constructs a slider from a given parent with an optional palette
    /// @param parent The parent the control is bound to - usually the screen
    /// @param palette The palette associated with the control. This is usually the screen's palette.
    vslider(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent, palette),m_knob_border_width(1), m_knob_shape(vslider_shape::ellipse), m_knob_radiuses(0, 0), m_bar_border_width(1), m_bar_width(5), m_bar_radiuses(2, 2), m_minimum(0), m_maximum(100), m_value(-1), m_on_value_changed_cb(nullptr), m_on_value_changed_state(nullptr), m_on_released_cb(nullptr), m_on_released_state(nullptr) {
        m_knob_color = gfx::vector_pixel(255, 255, 255, 255);
        m_knob_border_color = gfx::vector_pixel(255,0, 0, 0);
        m_bar_color = gfx::vector_pixel(255, 255, 255, 255);
        m_bar_border_color = gfx::vector_pixel(255, 0, 0, 0);
    }
    /// @brief Constructs a slider from a given parent with an optional palette
    vslider() : base_type(),  m_knob_border_width(1), m_knob_shape(vslider_shape::ellipse), m_knob_radiuses(0, 0), m_bar_border_width(1), m_bar_width(5), m_bar_radiuses(2, 2), m_minimum(0), m_maximum(100), m_value(-1), m_on_value_changed_cb(nullptr), m_on_value_changed_state(nullptr), m_on_released_cb(nullptr), m_on_released_state(nullptr) {
        m_knob_color = gfx::vector_pixel(255, 255, 255, 255);
        m_knob_border_color = gfx::vector_pixel(255,0, 0, 0);
        m_bar_color = gfx::vector_pixel(255, 255, 255, 255);
        m_bar_border_color = gfx::vector_pixel(255, 0, 0, 0);
    }
    /// @brief Indicates the color of the knob
    /// @return The color
    gfx::rgba_pixel<32> knob_color() const {
        gfx::rgba_pixel<32> result;
        gfx::convert(m_knob_color,&result);
        return result;
    }
    /// @brief Sets the color of the knob
    /// @param value The color
    void knob_color(gfx::rgba_pixel<32> value) {
        gfx::convert(value,&m_knob_color);
        this->invalidate();
    }
    /// @brief Indicates the color of the knob border
    /// @return The color
    gfx::rgba_pixel<32> knob_border_color() const {
        gfx::rgba_pixel<32> result;
        gfx::convert(m_knob_border_color,&result);
        return result;
    }
    /// @brief Sets the color of the knob border
    /// @param value The color
    void knob_border_color(gfx::rgba_pixel<32> value) {
        gfx::convert(value,&m_knob_border_color);
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
    /// @brief Indicates the radiuses of the knob
    /// @return The radiuses of the knob
    size16 knob_radiuses() const {
        return m_knob_radiuses;
    }
    /// @brief Sets the radiuses of the knob
    /// @param value The knob radiuses
    void knob_radiuses(size16 value) {
        m_knob_radiuses = value;
        this->invalidate();
    }
    /// @brief Indicates the shape of the knob
    /// @return The shape
    vslider_shape knob_shape() const {
        return m_knob_shape;
    }
    /// @brief Sets the shape of the knob
    /// @param value The shape
    void knob_shape(vslider_shape value) {
        m_knob_shape = value;
        this->invalidate();
    }
    /// @brief Indicates the color of the bar
    /// @return The color
    gfx::rgba_pixel<32> color() const {
        gfx::rgba_pixel<32> result;
        gfx::convert(m_bar_color,&result);
        return result;
    }
    /// @brief Sets the color of the bar
    /// @param value The color
    void color(gfx::rgba_pixel<32> value) {
        gfx::convert(value,&m_bar_color);
        this->invalidate();
    }
    /// @brief Indicates the color of the bar border
    /// @return The color
    gfx::rgba_pixel<32> border_color() const {
        gfx::rgba_pixel<32> result;
        gfx::convert(m_bar_border_color,&result);
        return result;
    }
    /// @brief Sets the color of the bar border
    /// @param value The color
    void border_color(gfx::rgba_pixel<32> value) {
        gfx::convert(value,&m_bar_border_color);
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
    /// @return The height of the bar
    uint16_t bar_width() const {
        return m_bar_width;
    }
    /// @brief Sets the width of the bar
    /// @param value The bar height in pixels
    void bar_width(uint16_t value) {
        m_bar_width = value;
        this->invalidate();
    }
    /// @brief Indicates the radiuses of the bar
    /// @return The radiuses of the bar
    size16 bar_radiuses() const {
        return m_bar_radiuses;
    }
    /// @brief Sets the radiuses of the bar
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
        if(m_orientation!=value) {
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
    virtual void on_paint(gfx::canvas& destination, const srect16& clip) override {
        draw_bar(destination);
        const uint16_t range = (m_maximum - m_minimum) + 1;
        const uint16_t offset_value = m_value - m_minimum;
        //const float mult = 1.f / (float)range;
        float radius;
        if(m_orientation==uix_orientation::horizontal) {
            radius = this->bounds().height() * 0.5f;
        } else {
            radius = this->bounds().width() * 0.5f;
        }
        if(m_orientation==uix_orientation::horizontal) {
            const float x = ((float)offset_value/(float)(range+radius/2.f))*(this->dimensions().width-radius)-(radius*0.5f)+1;
            draw_knob(destination, spoint16(x, 0));
        } else {
            const float y = ((float)((range)-offset_value-1)/(float)range)*(this->dimensions().height-radius)-(radius*0.5f);
            draw_knob(destination, spoint16(0,y));
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
    /// @brief Called when the slider is released
    virtual void on_release() override {
        if(m_on_released_cb!=nullptr) {
            m_on_released_cb(m_on_released_state);
        }
    }
};
}  // namespace uix
#endif  