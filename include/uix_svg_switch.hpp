#ifndef HTCW_UIX_SVG_SWITCH
#define HTCW_UIX_SVG_SWITCH
#include "uix_core.hpp"
namespace uix {
/// @brief An SVG switch control
/// @tparam ControlSurfaceType The type of control surface - usually the screen
template <typename ControlSurfaceType>
class svg_switch : public control<ControlSurfaceType> {
   public:
    using type = svg_switch;
    using base_type = control<ControlSurfaceType>;
    using pixel_type = typename ControlSurfaceType::pixel_type;
    using palette_type = typename ControlSurfaceType::palette_type;
    using control_surface_type = ControlSurfaceType;
    /// @brief The callback type for when value() changes
    typedef void (*on_value_changed_callback_type)(void* state);

   private:
    gfx::svg_doc m_bar, m_knob;
    bool m_knob_dirty, m_backing_dirty;
    gfx::rgba_pixel<32> m_knob_color, m_knob_border_color;
    uint16_t m_knob_border_width;
    gfx::rgba_pixel<32> m_background_color, m_border_color;
    uint16_t m_border_width;
    sizef m_radiuses;
    uix_orientation m_orientation;
    bool m_value;
    on_value_changed_callback_type m_on_value_changed_cb;
    void* m_on_value_changed_state;
    void do_copy_fields(const svg_switch& rhs) {
        m_backing_dirty = true;
        m_knob_dirty = true;
        m_knob_color = rhs.m_knob_color;
        m_knob_border_color = rhs.m_knob_border_color;
        m_knob_border_width = rhs.m_knob_border_width;
        m_background_color = rhs.m_background_color;
        m_border_color = rhs.m_border_color;
        m_border_width = rhs.m_border_width;
        m_radiuses = rhs.m_radiuses;
        m_orientation = rhs.m_orientation;
        m_value = rhs.m_value;
        m_on_value_changed_cb = rhs.m_on_value_changed_cb;
        m_on_value_changed_state = rhs.m_on_value_changed_state;
    }
    void rebuild_knob() {
        if (m_knob_dirty) {
            float radius;
            if (m_orientation == uix_orientation::horizontal) {
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

            b.add_ellipse({radius,radius}, {radius*0.7f, radius*0.7f}, si);

            b.to_doc(&m_knob);
            m_knob_dirty = false;
        }
    }
    void rebuild_backing() {
        if (m_backing_dirty) {
            float radius;
            rectf bounds;
            if (m_orientation == uix_orientation::horizontal) {
                radius = this->dimensions().height * .05f;
                bounds = rectf(0, 0, this->dimensions().width - 1 ,this->dimensions().height - 1);
            } else {
                radius = this->dimensions().width * .05f;
                bounds = rectf(0, 0, this->dimensions().width - 1, this->dimensions().height - 1);
            }
            gfx::svg_doc_builder b(bounds.dimensions());
            gfx::svg_shape_info si;
            si.fill.type = gfx::svg_paint_type::color;
            si.fill.color = m_background_color;
            si.stroke_width = m_border_width;
            si.stroke.type = gfx::svg_paint_type::color;
            si.stroke.color = m_border_color;
            bounds.x1+=1.0f;
            bounds.y1+=1.0f;
            bounds.x2-=1.0f;
            bounds.y2-=1.0f;
            if (m_radiuses.width != 0 || m_radiuses.height != 0) {
                b.add_rounded_rectangle(bounds, m_radiuses, si);
            } else {
                b.add_rectangle(bounds, si);
            }
            b.to_doc(&m_bar);
            m_backing_dirty = false;
        }
    }

   protected:
    /// @brief For derivative classes, moves the control
    /// @param rhs The slider to move
    void do_move_control(svg_switch& rhs) {
        this->base_type::do_move_control(rhs);
        do_copy_fields(rhs);
        m_backing_dirty = true;
        m_knob_dirty = true;
        rhs.m_on_value_changed_cb = nullptr;
    }
    /// @brief For derivative classes, copies the control
    /// @param rhs The slider to copy
    void do_copy_control(const svg_switch& rhs) {
        this->base_type::do_copy_control(rhs);
        do_copy_fields(rhs);
        m_backing_dirty = true;
        m_knob_dirty = true;
    }

   public:
    /// @brief Moves a slider control
    /// @param rhs The control to move
    svg_switch(svg_switch&& rhs) {
        do_move_control(rhs);
    }
    /// @brief Moves a slider control
    /// @param rhs The control to move
    /// @return this
    svg_switch& operator=(svg_switch&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    /// @brief Copies a slider control
    /// @param rhs The control to copy
    svg_switch(const svg_switch& rhs) {
        do_copy_control(rhs);
    }
    /// @brief Copies a slider control
    /// @param rhs The control to copy
    /// @return this
    svg_switch& operator=(const svg_switch& rhs) {
        do_copy_control(rhs);
        return *this;
    }
    /// @brief Constructs a slider from a given parent with an optional palette
    /// @param parent The parent the control is bound to - usually the screen
    /// @param palette The palette associated with the control. This is usually the screen's palette.
    svg_switch(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent, palette), m_knob_dirty(true), m_backing_dirty(true), m_knob_border_width(1), m_border_width(1), m_radiuses(2, 2), m_value(false), m_on_value_changed_cb(nullptr), m_on_value_changed_state(nullptr) {
        m_knob_color = gfx::rgba_pixel<32>(255, 255, 255, 255);
        m_knob_border_color = gfx::rgba_pixel<32>(0, 0, 0, 255);
        m_background_color = gfx::rgba_pixel<32>(255, 255, 255, 255);
        m_border_color = gfx::rgba_pixel<32>(0, 0, 0, 255);
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
    
    /// @brief Indicates the color of the switch background
    /// @return The color
    gfx::rgba_pixel<32> background_color() const {
        return m_background_color;
    }
    /// @brief Sets the color of the switch background
    /// @param value The color
    void background_color(gfx::rgba_pixel<32> value) {
        m_background_color = value;
        m_backing_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the border
    /// @return The color
    gfx::rgba_pixel<32> border_color() const {
        return m_border_color;
    }
    /// @brief Sets the color of the bar border
    /// @param value The color
    void border_color(gfx::rgba_pixel<32> value) {
        m_border_color = value;
        m_backing_dirty = true;
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
        m_backing_dirty = true;
        this->invalidate();
    }
    
    /// @brief Indicates the radiuses of the bar
    /// @return The radiuses of the bar
    sizef radiuses() const {
        return m_radiuses;
    }
    /// @brief Sets the radiuses of the bar
    /// @param value The bar radiuses
    void radiuses(sizef value) {
        m_radiuses = value;
        m_knob_dirty = true;
        m_backing_dirty = true;
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
        if (m_orientation != value) {
            m_orientation = value;
            m_knob_dirty = true;
            m_backing_dirty = true;
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
        rebuild_backing();
        rebuild_knob();
    }
    /// @brief Called when the slider is painted
    /// @param destination The draw destination
    /// @param clip The clipping rectangle
    virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
        gfx::draw::svg(destination, destination.bounds(), m_bar, 1.0f, &clip);
        const uint16_t range = 1;
        const uint16_t offset_value = m_value;
        const float mult = (float)offset_value / (float)range;
        int16_t adj;
        float radius;

        if (m_orientation == uix_orientation::horizontal) {
            gfx::draw::svg(destination, destination.bounds().offset(value() *(this->dimensions().width-m_knob.dimensions().width), 0), m_knob, 1.0f, &clip);
        } else {
            gfx::draw::svg(destination, destination.bounds().offset(0,(!value()) * (this->dimensions().height - m_knob.dimensions().height)), m_knob, 1.0f, &clip);
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
            ext = this->dimensions().width;
            if (i < 0.0f) i = 0.0f;
            if (i >= ext) {
                i = ext - 1;
            }
            const float mult = i / (float)(ext - 1);
            bool v = i >= 0.5f;
            value(v);
        } else {
            i = locations->y;
            ext = this->dimensions().height;
            if (i < 0.0f) i = 0.0f;
            if (i >= ext) {
                i = ext - 1;
            }
            const float mult = i / (float)(ext - 1);
            bool v = i < 0.5f;
            value(v);
        }
      
        return true;
    }
};
}  // namespace uix
#endif  // HTCW_UIX_SVG_SWITCH