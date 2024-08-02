#ifndef HTCW_UIX_SVG_BOX
#define HTCW_UIX_SVG_BOX
#include <gfx_svg_doc.hpp>

#include "uix_core.hpp"
namespace uix {
/// @brief Represents a control that displays an SVG document
/// @tparam ControlSurfaceType The type of control surface to draw to, usually taken from the screen
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
   protected:
    void do_move_control(svg_box& rhs) {
        this->base_type::do_move_control(rhs);
        m_svg = rhs.m_svg;
        m_rect = rhs.m_rect;
        m_justify = rhs.m_justify;
    }
    void do_copy_control(const svg_box& rhs) {
        this->base_type::do_copy_control(rhs);
        m_svg = rhs.m_svg;
        m_rect = rhs.m_rect;
        m_justify = rhs.m_justify;
    }

   public:
    /// @brief Moves an svg_box
    /// @param rhs The svg_box to move
    svg_box(svg_box&& rhs) {
        do_move_control(rhs);
    }
    /// @brief Moves an svg_box
    /// @param rhs The svg_box to move
    /// @return this
    svg_box& operator=(svg_box&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    /// @brief Copies an svg_box
    /// @param rhs The svg_box to move
    svg_box(const svg_box& rhs) {
        do_copy_control(rhs);
    }
    /// @brief Copies an svg_box
    /// @param rhs The svg_box to move
    /// @return this
    svg_box& operator=(const svg_box& rhs) {
        do_copy_control(rhs);
        return *this;
    }
    /// @brief Indicates the document to render
    /// @return A pointer to a gfx::svg_doc instance
    const gfx::svg_doc* doc() const {
        return m_svg;
    }
    /// @brief Sets the document to render
    /// @param value A pointer to a gfx::svg_doc instance
    void doc(const gfx::svg_doc* value) {
        if (value != m_svg) {
            m_svg = value;
            m_rect = {0, 0, 0, 0};
            this->invalidate();
        }
    }
    /// @brief Constructs a new instance of a svg_box with the specified parent and optional palette
    /// @param parent The parent. Usually this is the screen
    /// @param palette The palette, if any. This is usually taken from the screen
    svg_box(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
    : base_type(parent, palette), m_svg(nullptr), m_rect(0, 0, 0, 0), m_justify(uix_justify::top_left) {
    }
    /// @brief Constructs a new instance of a svg_box with the specified parent and optional palette
    svg_box()
    : base_type(), m_svg(nullptr), m_rect(0, 0, 0, 0), m_justify(uix_justify::top_left) {
    }
    /// @brief Indicates the justification of the SVG within the control
    /// @return The justification
    uix_justify justify() const {
        return m_justify;
    }
    /// @brief Sets the justification for the SVG within the control
    /// @param value The justification
    void justify(uix_justify value) {
        if (m_justify != value) {
            m_justify = value;
            m_rect = {0, 0, 0, 0};
            this->invalidate();
        }
    }
    /// @brief Paints the rendered SVG document
    /// @param destination The destination to paint to
    /// @param clip The clipping rectangle
    virtual void on_paint(control_surface_type& destination, const uix::srect16& clip) override {
        // get the rect for the drawing area
        uix::srect16 b = (uix::srect16)this->dimensions().bounds();
        // if there's an SVG set, render it
        // scaled to the control
        if (m_svg != nullptr) {
            float scale = m_svg->scale(b.dimensions());
            if (m_rect.x1 == 0 && m_rect.y1 == 0 && m_rect.x2 == 0 && m_rect.y2 == 0) {
                uint16_t w = m_svg->dimensions().width * scale;
                uint16_t h = m_svg->dimensions().height * scale;
                m_rect = srect16(0, 0, w - 1, h - 1);
                switch (m_justify) {
                    case uix_justify::top_middle:
                        m_rect.center_horizontal_inplace((srect16)destination.bounds());
                        break;
                    case uix_justify::top_right:
                        m_rect.offset_inplace(destination.dimensions().width - w, 0);
                        break;
                    case uix_justify::center_left:
                        m_rect.center_vertical_inplace((srect16)destination.bounds());
                        break;
                    case uix_justify::center:
                        m_rect.center_inplace((srect16)destination.bounds());
                        break;
                    case uix_justify::center_right:
                        m_rect.center_vertical_inplace((srect16)destination.bounds());
                        m_rect.offset_inplace(destination.dimensions().width - w, 0);
                        break;
                    case uix_justify::bottom_left:
                        m_rect.offset_inplace(0, destination.dimensions().height - h);
                        break;
                    case uix_justify::bottom_middle:
                        m_rect.center_horizontal_inplace((srect16)destination.bounds());
                        m_rect.offset_inplace(0, destination.dimensions().height - h);
                        break;
                    case uix_justify::bottom_right:
                        m_rect.offset_inplace(destination.dimensions().width - w,
                                              destination.dimensions().height - h);
                        break;
                    default:  // top_left
                        break;
                }
            }
            if (clip.intersects(m_rect)) {
                gfx::draw::svg(destination,
                               m_rect,
                               *m_svg,
                               scale, &clip);
            }
        }
        // call the base on paint method
        base_type::on_paint(destination, clip);
    }
    /// @brief Sets The bounds of the control
    /// @param value An srect16 indicating the location and size of the control 
    virtual void bounds(const srect16& value) override {
        base_type::bounds(value);
        m_rect = srect16(0, 0, 0, 0);
    }
};
}  // namespace uix
#endif  // HTCW_UIX_SVG_BOX