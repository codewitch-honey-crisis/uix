#ifndef HTCW_UIX_SVG_CLOCK
#define HTCW_UIX_SVG_CLOCK
#include <time.h>

#include "uix_core.hpp"
namespace uix {
/// @brief Represents an analog clock
/// @tparam ControlSurfaceType The control surface type, usually taken from the screen
template <typename ControlSurfaceType>
class svg_clock : public uix::control<ControlSurfaceType> {
   public:
    /// @brief The type of the instance
    using type = svg_clock;
    /// @brief The pixel type of the draw surface
    using pixel_type = typename ControlSurfaceType::pixel_type;
    /// @brief The palette type of the draw surface
    using palette_type = typename ControlSurfaceType::palette_type;

   private:
    using base_type = uix::control<ControlSurfaceType>;
    using control_surface_type = ControlSurfaceType;
    gfx::svg_doc m_svg;
    time_t m_time;
    bool m_dirty;
    bool m_dragging;
    time_t m_time_drag;
    spoint16 m_drag_point;
    bool m_allow_drag;
    gfx::rgba_pixel<32> m_face_color, m_face_border_color;
    uint16_t m_face_border_width;
    gfx::rgba_pixel<32> m_tick_color, m_tick_border_color;
    uint16_t m_tick_border_width;
    gfx::rgba_pixel<32> m_hour_color, m_hour_border_color;
    uint16_t m_hour_border_width;
    gfx::rgba_pixel<32> m_minute_color, m_minute_border_color;
    uint16_t m_minute_border_width;
    gfx::rgba_pixel<32> m_second_color, m_second_border_color;
    uint16_t m_second_border_width;
    void do_copy_fields(const svg_clock& rhs) {
        m_time = rhs.m_time;
        m_dragging = rhs.m_dragging;
        m_time_drag = rhs.m_time_drag;
        m_drag_point = rhs.m_drag_point;
        m_allow_drag = rhs.m_allow_drag;
        m_face_color = rhs.m_face_color;
        m_face_border_color = rhs.m_face_border_color;
        m_face_border_width = rhs.m_face_border_width;
        m_tick_color = rhs.m_tick_color;
        m_tick_border_color = rhs.m_tick_border_color;
        m_tick_border_width = rhs.m_tick_border_width;
        m_hour_color = rhs.m_hour_color;
        m_hour_border_color = rhs.m_hour_border_color;
        m_hour_border_width = rhs.m_hour_border_width;
        m_minute_color = rhs.m_minute_color;
        m_minute_border_color = rhs.m_minute_border_color;
        m_minute_border_width = rhs.m_minute_border_width;
        m_second_color = rhs.m_second_color;
        m_second_border_color = rhs.m_second_border_color;
        m_second_border_width = rhs.m_second_border_width;
    }
    void update_transform(float rotation, float& ctheta, float& stheta) {
        float rads = rotation * (3.1415926536f / 180.0f);
        ctheta = cosf(rads);
        stheta = sinf(rads);
    }
    constexpr gfx::pointf translate(float ctheta, float stheta, gfx::pointf center, gfx::pointf offset, float x, float y) const {
        float rx = (ctheta * (x - (float)center.x) - stheta * (y - (float)center.y) + (float)center.x) + offset.x;
        float ry = (stheta * (x - (float)center.x) + ctheta * (y - (float)center.y) + (float)center.y) + offset.y;
        return {(float)rx, (float)ry};
    }

   protected:
    /// @brief Moves the control
    /// @param rhs The control to move from
    void do_move_control(svg_clock& rhs) {
        this->base_type::do_move_control(rhs);
        do_copy_fields(rhs);
        m_dirty = true;
    }
    /// @brief Copies the control
    /// @param rhs The control to copy from
    void do_copy_control(const svg_clock& rhs) {
        this->base_type::do_copy_control(rhs);
        do_copy_fields(rhs);
        m_dirty = true;
    }

   public:
    /// @brief Moves a new clock control from an existing one
    /// @param rhs The existing clock
    svg_clock(svg_clock&& rhs) {
        do_move_control(rhs);
    }
    /// @brief Moves the clock control from an existing one
    /// @param rhs The existing clock
    /// @return This clock
    svg_clock& operator=(svg_clock&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    /// @brief Copies a new clock control from an existing one
    /// @param rhs The existing clock
    svg_clock(const svg_clock& rhs) {
        do_copy_control(rhs);
    }
    /// @brief Copies the clock control from an existing one
    /// @param rhs The exisitng clock
    /// @return This clock
    svg_clock& operator=(const svg_clock& rhs) {
        do_copy_control(rhs);
        return *this;
    }
    /// @brief Constructs a new instance of a clock
    /// @param parent The parent (a screen)
    /// @param palette The palette, if applicable
    svg_clock(uix::invalidation_tracker& parent, const palette_type* palette = nullptr)
        : base_type(parent, palette), m_time(0), m_dirty(true), m_dragging(false),m_allow_drag(false) {
        static const constexpr gfx::rgba_pixel<32> white(0xFF, 0xFF, 0xFF, 0xFF);
        static const constexpr gfx::rgba_pixel<32> black(0x0, 0x0, 0x0, 0xFF);
        static const constexpr gfx::rgba_pixel<32> gray(0x7F, 0x7F, 0x7F, 0xFF);
        static const constexpr gfx::rgba_pixel<32> red(0xFF, 0x0, 0x0, 0xFF);
        m_face_color = white;
        m_face_border_color = white;
        m_face_border_width = 1;
        m_tick_color = gray;
        m_tick_border_color = gray;
        m_tick_border_width = 1;
        m_hour_color = black;
        m_hour_border_color = black;
        m_hour_border_width = 1;
        m_minute_color = black;
        m_minute_border_color = black;
        m_minute_border_width = 1;
        m_second_color = red;
        m_second_border_color = red;
        m_second_border_width = 1;
    }
    /// @brief Indicates the time
    /// @return The time
    time_t time() const {
        return m_time;
    }
    /// @brief Sets the time
    /// @param value The time
    void time(time_t value) {
        if (value != m_time) {
            m_time = value;
            m_dirty = true;
            this->invalidate();
        }
    }
    /// @brief Indicates the color of the face
    /// @return The color
    gfx::rgba_pixel<32> face_color() const {
        return m_face_color;
    }
    /// @brief Sets the color of the face
    /// @param value The color
    void face_color(gfx::rgba_pixel<32> value) {
        m_face_color = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the face border
    /// @return The color
    gfx::rgba_pixel<32> face_border_color() const {
        return m_face_border_color;
    }
    /// @brief Sets the color of the face border
    /// @param value The color
    void face_border_color(gfx::rgba_pixel<32> value) {
        m_face_border_color = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the border width of the face
    /// @return The width in pixels
    uint16_t face_border_width() const {
        return m_face_border_width;
    }
    /// @brief Sets the border width of the face
    /// @param value The width in pixels
    void face_border_width(uint16_t value) {
        m_face_border_width = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the ticks
    /// @return The color
    gfx::rgba_pixel<32> tick_color() const {
        return m_tick_color;
    }
    /// @brief Sets the color of the ticks
    /// @param value The color
    void tick_color(gfx::rgba_pixel<32> value) {
        m_tick_color = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the tick borders
    /// @return The color
    gfx::rgba_pixel<32> tick_border_color() const {
        return m_tick_border_color;
    }
    /// @brief Sets the color of the tick borders
    /// @param value The color
    void tick_border_color(gfx::rgba_pixel<32> value) {
        m_tick_border_color = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the border width of the ticks
    /// @return The width in pixels
    uint16_t tick_border_width() const {
        return m_tick_border_width;
    }
    /// @brief Sets the border width of the ticks
    /// @param value The width in pixels
    void tick_border_width(uint16_t value) {
        m_tick_border_width = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the hour hand
    /// @return The color
    gfx::rgba_pixel<32> hour_color() const {
        return m_hour_color;
    }
    /// @brief Sets the color of the hour hand
    /// @param value The color
    void hour_color(gfx::rgba_pixel<32> value) {
        m_hour_color = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the hour hand border
    /// @return The color
    gfx::rgba_pixel<32> hour_border_color() const {
        return m_hour_border_color;
    }
    /// @brief Sets the color of the hour hand border
    /// @param value The color
    void hour_border_color(gfx::rgba_pixel<32> value) {
        m_hour_border_color = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the border width of the hour hand
    /// @return The width in pixels
    uint16_t hour_border_width() const {
        return m_hour_border_width;
    }
    /// @brief Sets the border width of the hour hand
    /// @param value The width in pixels
    void hour_border_width(uint16_t value) {
        m_hour_border_width = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the minute hand
    /// @return The color
    gfx::rgba_pixel<32> minute_color() const {
        return m_minute_color;
    }
    /// @brief Sets the color of the minute hand
    /// @param value The color
    void minute_color(gfx::rgba_pixel<32> value) {
        m_minute_color = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the minute hand border
    /// @return The color
    gfx::rgba_pixel<32> minute_border_color() const {
        return m_minute_border_color;
    }
    /// @brief Sets the color of the minute hand border
    /// @param value The color
    void minute_border_color(gfx::rgba_pixel<32> value) {
        m_minute_border_color = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the border width of the minute hand
    /// @return The width in pixels
    uint16_t minute_border_width() const {
        return m_minute_border_width;
    }
    /// @brief Sets the border width of the minute hand
    /// @param value The width in pixels
    void minute_border_width(uint16_t value) {
        m_minute_border_width = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the second hand
    /// @return The color
    gfx::rgba_pixel<32> second_color() const {
        return m_second_color;
    }
    /// @brief Sets the color of the second hand
    /// @param value The color
    void second_color(gfx::rgba_pixel<32> value) {
        m_second_color = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the color of the second hand border
    /// @return The color
    gfx::rgba_pixel<32> second_border_color() const {
        return m_second_border_color;
    }
    /// @brief Sets the color of the second hand border
    /// @param value The color
    void second_border_color(gfx::rgba_pixel<32> value) {
        m_second_border_color = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates the border width of the second hand
    /// @return The width in pixels
    uint16_t second_border_width() const {
        return m_second_border_width;
    }
    /// @brief Sets the border width of the second hand
    /// @param value The width in pixels
    void second_border_width(uint16_t value) {
        m_second_border_width = value;
        m_dirty = true;
        this->invalidate();
    }
    /// @brief Indicates whether the clock can be dragged
    /// @return True if it can be dragged, otherwise false
    bool allow_drag() const {
        return m_allow_drag;
    }
    /// @brief Sets the clock can be dragged
    /// @param value True if it can be dragged, otherwise false
    void allow_drag(bool value) {
        m_allow_drag = value;
    }
    /// @brief Indicates whether the clock is being dragged
    /// @return True if it's being dragged, otherwise false
    bool dragging() const {
        return m_dragging;
    }
   protected:
    /// @brief Override to handle when the control is touched
    /// @param locations_size The count of touch locations
    /// @param locations The locations, translated to local control coordinates
    /// @return True if touch was handled, or false if it wasn't.
    virtual bool on_touch(size_t locations_size, const spoint16* locations) {
        if(m_allow_drag) {
            if(!m_dragging) {
                m_dragging = true;
                m_time_drag = m_time;
                m_drag_point = *locations;
            } else {
                spoint16 pt = *locations;
                //time(m_time_drag + (((pt.x - m_drag_point.x)*m_drag_multiplier) + ((pt.y - m_drag_point.y) * m_drag_multiplier)));
                time(m_time_drag + (((pt.x - m_drag_point.x) ) + ((pt.y - m_drag_point.y))));
            }
            return true;
        }
        return false;

    }
    /// @brief Override to handle when the touched control has been released
    virtual void on_release() {
        m_dragging = false;
    }
    /// @brief Paints the control
    /// @param destination The surface to draw to
    /// @param clip The clipping rectangle
    virtual void on_paint(control_surface_type& destination, const uix::srect16& clip) override {
        // call the base on paint method
        base_type::on_paint(destination, clip);
        // get the rect for the drawing area
        uix::srect16 b = (uix::srect16)this->dimensions().bounds();
        gfx::draw::svg(destination, b, m_svg, 1, &clip);
    }
    /// @brief Prepars the control for drawing
    virtual void on_before_render() override {
        if (m_dirty) {
            gfx::svg_shape_info si;
            si.fill.type = gfx::svg_paint_type::color;
            si.stroke.type = gfx::svg_paint_type::color;
            gfx::pointf offset(0, 0);
            gfx::pointf center(0, 0);
            time_t time = m_time;
            float rotation(0);
            float ctheta, stheta;
            gfx::ssize16 size = this->bounds().dimensions();
            gfx::rectf b = gfx::sizef(size.width, size.height).bounds();
            gfx::svg_doc_builder db(b.dimensions());
            gfx::svg_path_builder pb;
            gfx::svg_path* path;
            b.inflate_inplace(-m_face_border_width - 1, -m_face_border_width - 1);
            
            float w = b.width();
            float h = b.height();
            if(w>h) w= h;
            // min(b.width(), b.height());
            gfx::rectf sr(0, w / 30, w / 30, w / 5);
            sr.center_horizontal_inplace(b);
            center = gfx::pointf(w * 0.5f + m_face_border_width + 1, w * 0.5f + m_face_border_width + 1);
            si.fill.color = m_face_color;
            si.stroke.color = m_face_border_color;
            si.stroke_width = m_face_border_width;
            db.add_ellipse(center, {center.x - 1, center.x - 1}, si);
            constexpr static const float rot_step = 360.0f / 12.0f;
            bool toggle = false;
            si.stroke.color = m_tick_border_color;
            si.fill.color = m_tick_color;
            si.stroke_width = m_tick_border_width;
            for (float rot = 0; rot < 360.0f; rot += rot_step) {
                rotation = rot;
                update_transform(rotation, ctheta, stheta);
                toggle = !toggle;
                if (toggle) {
                    pb.move_to(translate(ctheta, stheta, center, offset, sr.x1, sr.y1));
                    pb.line_to(translate(ctheta, stheta, center, offset, sr.x2, sr.y1));
                    pb.line_to(translate(ctheta, stheta, center, offset, sr.x2, sr.y2));
                    pb.line_to(translate(ctheta, stheta, center, offset, sr.x1, sr.y2));
                    pb.to_path(&path, true);
                    db.add_path(path, si);
                } else {
                    pb.move_to(translate(ctheta, stheta, center, offset, sr.x1, sr.y1));
                    pb.line_to(translate(ctheta, stheta, center, offset, sr.x2, sr.y1));
                    pb.line_to(translate(ctheta, stheta, center, offset, sr.x2, sr.y2 - sr.height() * 0.5f));
                    pb.line_to(translate(ctheta, stheta, center, offset, sr.x1, sr.y2 - sr.height() * 0.5f));
                    pb.to_path(&path, true);
                    db.add_path(path, si);
                }
            }
            sr = gfx::rectf(0, w / 40, w / 16, w / 2);
            sr.center_horizontal_inplace(b);
            rotation = (fmodf(time / 60.0f, 60) / 60.0f) * 360.0f;
            update_transform(rotation, ctheta, stheta);
            pb.move_to(translate(ctheta, stheta, center, offset, sr.x1 + sr.width() * 0.5f, sr.y1));
            pb.line_to(translate(ctheta, stheta, center, offset, sr.x2, sr.y2));
            pb.line_to(translate(ctheta, stheta, center, offset, sr.x1 + sr.width() * 0.5f, sr.y2 + (w / 20)));
            pb.line_to(translate(ctheta, stheta, center, offset, sr.x1, sr.y2));
            pb.to_path(&path, true);
            si.fill.color = m_minute_color;
            si.stroke.color = m_minute_border_color;
            si.stroke_width = m_minute_border_width;
            db.add_path(path, si);
            sr.y1 += w / 8;
            rotation = (fmodf(time / (3600.0f), 12.0f) / (12.0f)) * 360.0f;
            update_transform(rotation, ctheta, stheta);
            pb.move_to(translate(ctheta, stheta, center, offset, sr.x1 + sr.width() * 0.5f, sr.y1));
            pb.line_to(translate(ctheta, stheta, center, offset, sr.x2, sr.y2));
            pb.line_to(translate(ctheta, stheta, center, offset, sr.x1 + sr.width() * 0.5f, sr.y2 + (w / 20)));
            pb.line_to(translate(ctheta, stheta, center, offset, sr.x1, sr.y2));
            pb.to_path(&path, true);
            si.fill.color = m_hour_color;
            si.stroke.color = m_hour_border_color;
            si.stroke_width = m_hour_border_width;
            db.add_path(path, si);
            sr.y1 -= w / 8;
            rotation = ((time % 60) / 60.0f) * 360.0f;
            update_transform(rotation, ctheta, stheta);
            pb.move_to(translate(ctheta, stheta, center, offset, sr.x1 + sr.width() * 0.5f, sr.y1));
            pb.line_to(translate(ctheta, stheta, center, offset, sr.x2, sr.y2));
            pb.line_to(translate(ctheta, stheta, center, offset, sr.x1 + sr.width() * 0.5f, sr.y2 + (w / 20)));
            pb.line_to(translate(ctheta, stheta, center, offset, sr.x1, sr.y2));
            pb.to_path(&path, true);
            si.fill.color = m_second_color;
            si.stroke.color = m_second_border_color;
            si.stroke_width = m_second_border_width;
            db.add_path(path, si);

            db.to_doc(&m_svg);
            m_dirty = false;
        }
    }
};
}  // namespace uix
#endif  // HTCW_UIX_SVG_CLOCK