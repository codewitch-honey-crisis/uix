#ifndef HTCW_UIX_SVG_BOX
#define HTCW_UIX_SVG_BOX
#include "uix_core.hpp"
namespace uix {

template <typename PixelType, typename PaletteType = gfx::palette<PixelType, PixelType>>
class svg_box : public uix::control<PixelType, PaletteType> {
    // public and private type aliases
    // pixel_type and palette_type are
    // required on any control
   public:
    using type = svg_box;
    using pixel_type = PixelType;
    using palette_type = PaletteType;

   private:
    using base_type = uix::control<PixelType, PaletteType>;
    using control_surface_type = typename base_type::control_surface_type;
    // member data
    gfx::svg_doc* m_svg;
    // no reason for copy semantics
    svg_box(const svg_box& rhs) = delete;
    svg_box& operator=(const svg_box& rhs) = delete;
    // implements move semantics
    void do_move(svg_box& rhs) {
        do_move_control(rhs);
        m_svg = rhs.m_svg;
    }

   public:
    svg_box(svg_box&& rhs) {
        do_move(rhs);
    }
    svg_box& operator=(svg_box&& rhs) {
        do_move(rhs);
        return *this;
    }
    gfx::svg_doc* doc() const {
        return m_svg;
    }
    void doc(gfx::svg_doc* value) {
        if (value != m_svg) {
            m_svg = value;
            this->invalidate();
        }
    }
    svg_box(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
        : base_type(parent, palette), m_svg(nullptr) {
    }

    virtual void on_paint(control_surface_type& destination, const uix::srect16& clip) override {
        // get the rect for the drawing area
        uix::srect16 b = (uix::srect16)this->dimensions().bounds();
        // if there's an SVG set, render it
        // scaled to the control
        if (m_svg != nullptr) {
            gfx::draw::svg(destination,
                           b,
                           *m_svg,
                           m_svg->scale(b.dimensions()));
        }
        // call the base on paint method
        base_type::on_paint(destination, clip);
    }
};
}  // namespace uix
#endif  // HTCW_UIX_SVG_BOX