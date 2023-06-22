#ifndef HTCW_UIX_SVG_BOX
#define HTCW_UIX_SVG_BOX
#include "uix_core.hpp"
#include <gfx_svg_doc.hpp>
namespace uix {

template <typename ControlSurfaceType>
class svg_box : public uix::control<ControlSurfaceType> {
    // public and private type aliases
    // pixel_type and palette_type are
    // required on any control
   public:
    using type = svg_box;
    using pixel_type = typename ControlSurfaceType::pixel_type;
    using palette_type = typename ControlSurfaceType::palette_type;

   private:
    using base_type = uix::control<ControlSurfaceType>;
    using control_surface_type = ControlSurfaceType;
    // member data
    const gfx::svg_doc* m_svg;
    srect16 m_rect;
    uix_justify m_justify;
    void do_move(svg_box& rhs) {
        this->do_move_control(rhs);
        m_svg = rhs.m_svg;
        m_rect = rhs.m_rect;
        m_justify = rhs.m_justify;
    }
    void do_copy(const svg_box& rhs) {
        this->do_copy_control(rhs);
        m_svg = rhs.m_svg;
        m_rect = rhs.m_rect;
        m_justify = rhs.m_justify;
    }
   public:
    svg_box(svg_box&& rhs) {
        do_move(rhs);
    }
    svg_box& operator=(svg_box&& rhs) {
        do_move(rhs);
        return *this;
    }
    svg_box(const svg_box& rhs) {
        do_copy(rhs);
    }
    svg_box& operator=(const svg_box& rhs) {
        do_copy(rhs);
        return *this;
    }
    const gfx::svg_doc* doc() const {
        return m_svg;
    }
    void doc(const gfx::svg_doc* value) {
        if (value != m_svg) {
            m_svg = value;
            m_rect = {0,0,0,0};
            this->invalidate();
        }
    }
    svg_box(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
        : base_type(parent, palette), m_svg(nullptr), m_rect(0,0,0,0),m_justify(uix_justify::top_left) {
    }
    uix_justify justify() const {
        return m_justify;
    }
    void justify(uix_justify value) {
        if(m_justify!=value) {
            m_justify = value;
            m_rect = {0,0,0,0};
            this->invalidate();
        }
    }
    virtual void on_paint(control_surface_type& destination, const uix::srect16& clip) override {
        // get the rect for the drawing area
        uix::srect16 b = (uix::srect16)this->dimensions().bounds();
        // if there's an SVG set, render it
        // scaled to the control
        if (m_svg != nullptr) {
            float scale = m_svg->scale(b.dimensions());
            if(m_rect.x1==0&&m_rect.y1==0&&m_rect.x2==0&&m_rect.y2==0) {
                uint16_t w = m_svg->dimensions().width*scale;
                uint16_t h = m_svg->dimensions().height*scale;
                m_rect = srect16(0,0,w-1,h-1);
                switch(m_justify) {
                    case uix_justify::top_middle:
                        m_rect.center_horizontal_inplace(((base_type*)this)->bounds());
                        break;
                    case uix_justify::top_right:
                        m_rect.offset_inplace(this->dimensions().width-w,0);
                        break;
                    case uix_justify::center_left:
                        m_rect.center_vertical_inplace(((base_type*)this)->bounds());
                        break;
                    case uix_justify::center:
                        m_rect.center_inplace(((base_type*)this)->bounds());
                        break;
                    case uix_justify::center_right:
                        m_rect.center_vertical_inplace(((base_type*)this)->bounds());
                        m_rect.offset_inplace(this->dimensions().width-w,0);
                        break;
                    case uix_justify::bottom_left:
                        m_rect.offset_inplace(0,this->dimensions().height-h);
                        break;
                    case uix_justify::bottom_middle:
                        m_rect.center_horizontal_inplace(((base_type*)this)->bounds());
                        m_rect.offset_inplace(0,this->dimensions().height-h);
                        break;
                    case uix_justify::bottom_right:
                        m_rect.offset_inplace(this->dimensions().width-w,
                                            this->dimensions().height-h);
                        break;
                    default: // top_left
                    break;
                }
                
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
    virtual void bounds(const srect16& value) override {
        base_type::bounds(value);
        m_rect = srect16(0,0,0,0);
    }
};
}  // namespace uix
#endif  // HTCW_UIX_SVG_BOX