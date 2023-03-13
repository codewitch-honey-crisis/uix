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
    srect16 m_rect;
    // no reason for copy semantics
    svg_box(const svg_box& rhs) = delete;
    svg_box& operator=(const svg_box& rhs) = delete;
    // implements move semantics
    void do_move(svg_box& rhs) {
        do_move_control(rhs);
        m_svg = rhs.m_svg;
        m_rect = rhs.m_rect;
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
            m_rect = {0,0,0,0};
            this->invalidate();
        }
    }
    svg_box(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
        : base_type(parent, palette), m_svg(nullptr), m_rect(0,0,0,0) {
    }

    virtual void on_paint(control_surface_type& destination, const uix::srect16& clip) override {
        // get the rect for the drawing area
        uix::srect16 b = (uix::srect16)this->dimensions().bounds();
        // if there's an SVG set, render it
        // scaled to the control
        if (m_svg != nullptr) {
            float scale = m_svg->scale(b.dimensions());
            if(m_rect.x1==0&&m_rect.y1==0&&m_rect.x2==0&&m_rect.y2==0) {
                m_rect = srect16(0,0,m_svg->dimensions().width*scale-1,m_svg->dimensions().height*scale-1);
            }
            if(clip.intersects(m_rect)) {
                gfx::draw::svg(destination,
                            b,
                            *m_svg,
                            scale,&clip);
            }
        }
        // call the base on paint method
        base_type::on_paint(destination, clip);
    }
};
}  // namespace uix
#endif  // HTCW_UIX_SVG_BOX