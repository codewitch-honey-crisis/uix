#ifndef HTCW_UIX_VSWITCH
#define HTCW_UIX_VSWITCH
#include "uix_core.hpp"
#include "uix_canvas_control.hpp"
namespace uix {
/// @brief The shape of the switch knob
enum struct vswitch_shape {
    /// @brief The knob is a circle
    circle = 0,
    /// @brief The knob is a square
    square = 1
};
/// @brief An SVG switch control
/// @tparam ControlSurfaceType The type of control surface - usually the screen
template <typename ControlSurfaceType>
class vswitch : public canvas_control<ControlSurfaceType> {
   public:
    using type = vswitch;
    using base_type = canvas_control<ControlSurfaceType>;
    using pixel_type = typename ControlSurfaceType::pixel_type;
    using palette_type = typename ControlSurfaceType::palette_type;
    using control_surface_type = ControlSurfaceType;
    /// @brief The callback type for when value() changes
    /// @param value The new value
    typedef void (*on_value_changed_callback_type)(bool value,void* state);

   private:
    gfx::vector_pixel m_knob_color, m_knob_border_color;
    uint16_t m_knob_border_width;
    vswitch_shape m_knob_shape;
    size16 m_knob_radiuses;
    gfx::vector_pixel m_background_color, m_border_color;
    uint16_t m_border_width;
    size16 m_radiuses;
    uix_orientation m_orientation;
    bool m_value;
    on_value_changed_callback_type m_on_value_changed_cb;
    void* m_on_value_changed_state;
    void do_copy_fields(const vswitch& rhs) {
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
    }
    void draw_knob(gfx::canvas& dst, spoint16 loc) {
        float radius;
        if (m_orientation == uix_orientation::horizontal) {
            radius = this->bounds().height() * 0.5f;
        } else {
            radius = this->bounds().width() * 0.5f;
        }
        gfx::canvas_style si=dst.style();
        si.fill_paint_type = gfx::paint_type::solid;
        si.fill_color = m_knob_color;
        si.stroke_width = m_knob_border_width;
        si.stroke_paint_type = gfx::paint_type::solid;
        si.stroke_color = m_knob_border_color;
        dst.style(si);
        if(m_knob_shape==vswitch_shape::circle) {
            pointf center(radius, radius);
            dst.ellipse(center.offset(loc.x,loc.y), {radius*0.7f, radius*0.7f});
        } else {
            rectf r(pointf(radius,radius).offset(loc.x,loc.y),radius*0.7f);
            if(m_knob_radiuses.width==0.0f && m_knob_radiuses.height==0.0f) {
                dst.rectangle(r);
            } else {
                dst.rounded_rectangle(r,(sizef)m_knob_radiuses);
            }
        }
        dst.render();
    
    }
    void draw_backing(gfx::canvas& dst) {
        rectf bounds;
        if (m_orientation == uix_orientation::horizontal) {
            bounds = rectf(0, 0, this->dimensions().width - 1 ,this->dimensions().height - 1);
        } else {
            bounds = rectf(0, 0, this->dimensions().width - 1, this->dimensions().height - 1);
        }
        gfx::canvas_style si=dst.style();
        si.fill_paint_type = gfx::paint_type::solid;
        si.fill_color = m_background_color;
        si.stroke_width = m_border_width;
        si.stroke_paint_type = gfx::paint_type::solid;
        si.stroke_color = m_border_color;
        dst.style(si);
        bounds.x1+=1.0f;
        bounds.y1+=1.0f;
        bounds.x2-=1.0f;
        bounds.y2-=1.0f;
        if (m_radiuses.width != 0 || m_radiuses.height != 0) {
            dst.rounded_rectangle(bounds, (sizef)m_radiuses);
        } else {
            dst.rectangle(bounds);
        }
        dst.render();
    
    }

   protected:
    /// @brief For derivative classes, moves the control
    /// @param rhs The slider to move
    void do_move_control(vswitch& rhs) {
        this->base_type::do_move_control(rhs);
        do_copy_fields(rhs);
        rhs.m_on_value_changed_cb = nullptr;
    }
    /// @brief For derivative classes, copies the control
    /// @param rhs The slider to copy
    void do_copy_control(const vswitch& rhs) {
        this->base_type::do_copy_control(rhs);
        do_copy_fields(rhs);
    }

   public:
    /// @brief Moves a slider control
    /// @param rhs The control to move
    vswitch(vswitch&& rhs) {
        do_move_control(rhs);
    }
    /// @brief Moves a slider control
    /// @param rhs The control to move
    /// @return this
    vswitch& operator=(vswitch&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    /// @brief Copies a slider control
    /// @param rhs The control to copy
    vswitch(const vswitch& rhs) {
        do_copy_control(rhs);
    }
    /// @brief Copies a slider control
    /// @param rhs The control to copy
    /// @return this
    vswitch& operator=(const vswitch& rhs) {
        do_copy_control(rhs);
        return *this;
    }
    /// @brief Constructs a slider from a given parent with an optional palette
    /// @param parent The parent the control is bound to - usually the screen
    /// @param palette The palette associated with the control. This is usually the screen's palette.
    vswitch(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent, palette), m_knob_border_width(1),m_knob_shape(vswitch_shape::circle), m_knob_radiuses(2,2), m_border_width(1), m_radiuses(2, 2), m_value(false), m_on_value_changed_cb(nullptr), m_on_value_changed_state(nullptr) {
        m_knob_color = gfx::vector_pixel(255, 255, 255, 255);
        m_knob_border_color = gfx::vector_pixel(255, 0, 0, 0);
        m_background_color = gfx::vector_pixel(255, 255, 255, 255);
        m_border_color = gfx::vector_pixel(255,0, 0, 0);
    }
    /// @brief Constructs a slider from a given parent with an optional palette
    vswitch() : base_type(), m_knob_border_width(1),m_knob_shape(vswitch_shape::circle), m_knob_radiuses(2,2), m_border_width(1), m_radiuses(2, 2), m_value(false), m_on_value_changed_cb(nullptr), m_on_value_changed_state(nullptr) {
        m_knob_color = gfx::vector_pixel(255, 255, 255, 255);
        m_knob_border_color = gfx::vector_pixel(255, 0, 0, 0);
        m_background_color = gfx::vector_pixel(255, 255, 255, 255);
        m_border_color = gfx::vector_pixel(255,0, 0, 0);
    }
    /// @brief Indicates the color of the knob
    /// @return The color
    gfx::rgba_pixel<32> knob_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_knob_color,&result);
        return result;
    }
    /// @brief Sets the color of the knob
    /// @param value The color
    void knob_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_knob_color);
        this->invalidate();
    }
    /// @brief Indicates the color of the knob border
    /// @return The color
    gfx::rgba_pixel<32> knob_border_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_knob_border_color,&result);
        return result;
    }
    /// @brief Sets the color of the knob border
    /// @param value The color
    void knob_border_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_knob_border_color);
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
    vswitch_shape knob_shape() const {
        return m_knob_shape;
    }
    /// @brief Sets the shape of the knob
    /// @param value The shape
    void knob_shape(vswitch_shape value) {
        m_knob_shape = value;
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
    /// @brief Indicates the color of the switch background
    /// @return The color
    gfx::rgba_pixel<32> background_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_background_color,&result);
        return result;
    }
    /// @brief Sets the color of the switch background
    /// @param value The color
    void background_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_background_color);
        this->invalidate();
    }
    /// @brief Indicates the color of the border
    /// @return The color
    gfx::rgba_pixel<32> border_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_border_color,&result);
        return result;
    }
    /// @brief Sets the color of the bar border
    /// @param value The color
    void border_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_border_color);
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
    
    /// @brief Indicates the radiuses of the bar
    /// @return The radiuses of the bar
    size16 radiuses() const {
        return m_radiuses;
    }
    /// @brief Sets the radiuses of the bar
    /// @param value The bar radiuses
    void radiuses(size16 value) {
        m_radiuses = value;
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
                m_on_value_changed_cb(m_value,m_on_value_changed_state);
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
    
    /// @brief Called when the slider is painted
    /// @param destination The draw destination
    /// @param clip The clipping rectangle
    virtual void on_paint(gfx::canvas& destination, const srect16& clip) override {
        draw_backing(destination);
        
        if (m_orientation == uix_orientation::horizontal) {
            float radiusx2 = this->bounds().height();
            spoint16 pt(value() *(this->dimensions().width-radiusx2), 0);
            draw_knob(destination,pt);
        } else {
            float radiusx2 = this->bounds().width();
            spoint16 pt(0,(!value()) * (this->dimensions().height - radiusx2));
            draw_knob(destination,pt);
        }
    }
    /// @brief Called when the slider is touched
    /// @param locations_size The count of locations (only the first one is respected)
    /// @param locations The locations
    /// @return True, because it was handled
    virtual bool on_touch(size_t locations_size, const spoint16* locations) override {
        float ext;
        float i;
        if (m_orientation == uix_orientation::horizontal) {
            i = locations->x;
            ext = this->dimensions().width/2;
            bool v = i>ext;
            value(v);
        } else {
            i = locations->y;
            ext = this->dimensions().height/2;
            bool v = i <= ext;
            value(v);
        }
      
        return true;
    }
};
}  // namespace uix
#endif  // HTCW_UIX_VSWITCH