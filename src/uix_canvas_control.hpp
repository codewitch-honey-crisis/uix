#ifndef HTCW_UIX_CANVAS_CONTROL
#define HTCW_UIX_CANVAS_CONTROL
#include "uix_core.hpp"
#include "gfx_draw_canvas.hpp"
namespace uix {
template<typename ControlSurfaceType>
class canvas_control : public control<ControlSurfaceType> {
    using base_type = control<ControlSurfaceType>;
    gfx::canvas m_canvas;
   public:
    using type = canvas_control;
    using pixel_type = typename ControlSurfaceType::pixel_type;
    using palette_type = typename ControlSurfaceType::palette_type;
    using control_surface_type = ControlSurfaceType;
protected:
    /// @brief Called to paint the vector canvas
    /// @param destination The canvas to paint to
    /// @param clip The clipping rectangle
    virtual void on_paint(gfx::canvas& destination, const srect16& clip) {

    }
    /// @brief Called to paint the control. This forwards to the vector canvas overload
    /// @param destination The destination to draw to
    /// @param clip The clipping rectangle
    virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
        m_canvas.initialize();
        gfx::draw::canvas(destination,m_canvas,point16::zero(),nullptr);
        on_paint(m_canvas,clip);
        // for diag purposes:
        // draw::rectangle(destination,destination.bounds(),rgb_pixel<16>(0,0,0));
    }
    /// @brief Copies the control
    /// @param value The control to copy
    void do_copy_control(const canvas_control& value) {
        base_type::on_copy_control(value);
        m_canvas.deinitialize();
        m_canvas.dimensions(size16(value.width(),value.height()));
    }
    /// @brief Moves the control
    /// @param value The control to move
    void do_move_control(canvas_control& value) {
        base_type::on_copy_control(value);
        m_canvas.deinitialize();
        m_canvas.dimensions(size16(value.width(),value.height()));
        value.m_canvas.deinitialize();
    }
public:
    /// @brief Constructs an empty control instance
    canvas_control() : base_type() {
    }
    /// @brief Constructs a control given a parent and an optional palette
    /// @param parent The parent invalidation tracker - usually a screen
    /// @param palette The palette. Typically the screen's palette()
    canvas_control(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent,palette) {
        
    }
    
    virtual void on_after_resize() override {
        m_canvas.deinitialize();
        m_canvas.dimensions(size16(this->bounds().width(),this->bounds().height()));
    }
};
}
#endif