#ifndef HTCW_UIX_BATTERY_HPP
#define HTCW_UIX_BATTERY_HPP
#include <uix_core.hpp>
namespace uix {
template <typename ControlSurfaceType>
class battery : public control<ControlSurfaceType> {
    using base_type = control<ControlSurfaceType>;
    constexpr static const gfx::rgba_pixel<32> color_white = gfx::rgba_pixel<32>(255, 255, 255, 255);
    gfx::rgba_pixel<32> m_inner_color;
    gfx::rgba_pixel<32> m_color;
    bool m_dirty;
    bool m_skip_inner;
    uint8_t m_level;
    static constexpr gfx::spoint16 s_outer[] = {
        {0, 256}, {0, 176}, {4, 151}, {15, 129}, {33, 111}, {55, 100}, {80, 96}, {464, 96}, {489, 100}, {511, 111}, {529, 129}, 
        {540, 151}, {544, 176}, {544, 192}, {567, 201}, {576, 224}, {576, 288}, {567, 311}, {544, 320}, {544, 336}, {540, 361}, 
        {529, 383}, {511, 401}, {489, 412}, {464, 416}, {80, 416}, {55, 412}, {33, 401}, {15, 383}, {4, 361}, {0, 336}, {0, 256}, 
        {64, 256}, {64, 352}, {480, 352}, {480, 160}, {64, 160}, {64, 256}};
    static constexpr size_t s_outer_size = sizeof(s_outer) / sizeof(s_outer[0]);
    static constexpr gfx::srect16 s_h_inner= gfx::srect16(64, 160, 480, 340);
    static constexpr gfx::srect16 s_v_inner=gfx::srect16(192, 64, 370, 480);
    gfx::spoint16 m_points[s_outer_size];
    gfx::srect16 m_inner;
    bool m_vertical;
    static void scale_and_center_points(spoint16* path, size_t path_size, int divisor,
                                        int src_cx, int src_cy,
                                        int dst_cx, int dst_cy) {
        spoint16* p = path;
        for (size_t i = 0; i < path_size; ++i) {
            // signed offset from center; C truncation is symmetric
            p->x = (int16_t)(dst_cx + (p->x - src_cx) / divisor);
            p->y = (int16_t)(dst_cy + (p->y - src_cy) / divisor);
            ++p;
        }
    }
    static void scale_and_center_points(srect16& bounds, int divisor,
                                        int src_cx, int src_cy,
                                        int dst_cx, int dst_cy) {
        // signed offset from center; C truncation is symmetric
        bounds.x1 = (int16_t)(dst_cx + (bounds.x1 - src_cx) / divisor);
        bounds.y1 = (int16_t)(dst_cy + (bounds.y1 - src_cy) / divisor);
        bounds.x2 = (int16_t)(dst_cx + (bounds.x2 - src_cx) / divisor);
        bounds.y2 = (int16_t)(dst_cy + (bounds.y2 - src_cy) / divisor);
    }
    // Quarter-turn the battery path so the terminal cap moves from the right to the
    // TOP (portrait). Rotates about the art center (288,256); winding and seam are
    // preserved, so the hole still punches under either fill rule. Afterward the art
    // spans 320 wide x 576 tall, still centered on (288,256).
    static void orient_vertical(spoint16* path, size_t path_size) {
        const int cx = 288, cy = 256;
        spoint16* p = path;
        for (size_t i = 0; i < path_size; ++i) {
            const int rx = p->x - cx, ry = p->y - cy;
            p->x = (int16_t)(cx + ry);  // (rx,ry) -> (ry,-rx)
            p->y = (int16_t)(cy - rx);
            ++p;
        }
    }

   public:
    using control_surface_type = ControlSurfaceType;
    using pixel_type = typename base_type::pixel_type;
    using palette_type = typename base_type::palette_type;
    /// @brief Constructs a new instance of the battery
    /// @param parent The parent screen
    /// @param palette The palette, if any
    battery(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
        : base_type(parent, palette), m_level(nullptr), m_dirty(true),m_skip_inner(false),m_level(0) {
        m_color = color_white;
        m_inner_color = color_white;
    }
    /// @brief Constructs a new instance of the battery
    battery()
        : base_type(), m_dirty(true),m_skip_inner(false),m_level(0) {
        m_color = color_white;
        m_inner_color = color_white;
    }
    battery(battery&& rhs) {
        this->do_move_control(rhs);
        m_dirty = true;
    }
    battery& operator=(battery&& rhs) {
        this->do_move_control(rhs);
        m_dirty = true;
        return *this;
    }
    battery(const battery& rhs) {
        this->do_copy_control(rhs);
        m_dirty = true;
    }
    battery& operator=(const battery& rhs) {
        this->do_copy_control(rhs);
        m_dirty = true;
        return *this;
    }
    /// @brief Indicates the raw battery m_level
    /// @return The m_level of the battery
    uint8_t level() const {
        return m_level;
    }
    /// @brief Sets the m_level of the battery
    /// @param value The level in percent from 0 to 100
    void level(uint8_t value) {
        m_level = value;
        this->invalidate();
    }
    /// @brief Indicates the color of the qrcode
    /// @return The RGBA8888 color
    gfx::rgba_pixel<32> color() const {
        return m_color;
    }
    /// @brief Sets the color of the qrcode
    /// @param value The RGBA8888 color
    void color(gfx::rgba_pixel<32> value) {
        m_color = value;
        this->invalidate();
    }
    /// @brief Indicates the background color of the qrcode
    /// @return The RGBA8888 color
    gfx::rgba_pixel<32> inner_color() const {
        return m_inner_color;
    }
    /// @brief Sets the color of the qrcode
    /// @param value The RGBA8888 color
    void inner_color(gfx::rgba_pixel<32> value) {
        m_inner_color = value;
        this->invalidate();
    }

   protected:
    void do_move_control(battery& rhs) {
        this->base_type::do_move_control(rhs);
        m_level = rhs.m_level;
        m_color = rhs.m_color;
        m_inner_color = rhs.m_inner_color;
    }
    void do_copy_control(const battery& rhs) {
        this->base_type::do_copy_control(rhs);
        m_level = rhs.m_level;
        m_dirty = true;
        m_color = rhs.m_color;
        m_inner_color = rhs.m_inner_color;
    }
    virtual void on_before_paint() override {
        if(m_dirty) {
            memcpy(m_points,s_outer,sizeof(m_points));
            const srect16 b = this->dimensions().bounds();
            m_skip_inner = false;
            const int bw = b.x2 - b.x1 + 1, bh = b.y2 - b.y1 + 1;
            if (bw < 2 || bh < 2) return;

            m_vertical = (bh > bw);         // portrait bounds -> stand it up
            int art_w = 576, art_h = 320;            // horizontal spans
            m_inner=s_h_inner;// horizontal interior
            if (m_vertical) {
                orient_vertical(m_points,s_outer_size);
                art_w = 320; art_h = 576;            // dims swap
                m_inner = s_v_inner; // rotated interior
            }

            const int div_x = (art_w + (bw - 2)) / (bw - 1);
            const int div_y = (art_h + (bh - 2)) / (bh - 1);
            const int div = (div_x > div_y ? div_x : div_y);

            const int dcx = b.x1 + (bw - 1) / 2, dcy = b.y1 + (bh - 1) / 2;
            scale_and_center_points(m_points, s_outer_size, div, 288, 256, dcx, dcy);
            scale_and_center_points(m_inner, div, 288, 256, dcx, dcy);
            if (m_level > 0) {
                if (!m_vertical) {
                    int fw = ((m_inner.x2 - m_inner.x1 + 1) * m_level) / 100;
                    if (fw <= 0) {
                        m_skip_inner=true;
                    } else {
                        m_inner = srect16(m_inner.x1, m_inner.y1, m_inner.x1 + fw - 1, m_inner.y2);
                    }
                } else {
                    int fh = ((m_inner.y2 - m_inner.y1 + 1) * m_level) / 100;
                    if (fh <= 0) m_skip_inner = true;
                    else m_inner = srect16(m_inner.x1, m_inner.y2 - fh + 1, m_inner.x2, m_inner.y2);
                }
            }
            m_dirty = false;
        }
    }
    virtual void on_after_resize() override {
        m_dirty = true;
    }
    virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
        // if can't draw for some reason, dirty will be true
        if (m_dirty) {
            return;
        }
        gfx::spath16 path(s_outer_size,m_points);
        gfx::draw::aa_filled_polygon(destination, path, m_color);;
    
        if (!m_skip_inner) {
            gfx::draw::filled_rectangle(destination, m_inner, m_inner_color);
        
        }
    }
};
}  // namespace uix
#endif  // HTCW_UIX_BATTERY_HPP