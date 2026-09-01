#ifndef HTCW_UIX_COLOR_PICKER_HPP
#define HTCW_UIX_COLOR_PICKER_HPP
#include <memory.h>
#include "uix_core.hpp"
namespace uix {
template<typename ControlSurfaceType>
class color_picker : public control<ControlSurfaceType> {
public:
    using type = color_picker;
    using control_surface_type = ControlSurfaceType;
    using pixel_type = typename ControlSurfaceType::pixel_type;
    using palette_type = typename ControlSurfaceType::palette_type;
    typedef void(*on_color_changed_callback_type)(uix_pixel color, void* state);
private:
    using base_type = control<ControlSurfaceType>;
    uint8_t m_hue;
    uint8_t m_saturation;
    uint8_t m_value;
    uix_pixel m_color;
    gfx::spoint16 m_pick;
    bool m_pick_visible;
    on_color_changed_callback_type m_on_color_changed_callback;
    void* m_on_color_changed_callback_state;
    static const constexpr int16_t pick_size = 16;
public:
    color_picker() : m_hue(0),m_saturation(0),m_value(0),m_color(0,true),m_pick(0,0),m_pick_visible(false),m_on_color_changed_callback(nullptr) {
        
    }
    color_picker(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent,palette),m_hue(0),m_saturation(0),m_value(0),m_color(0,true),m_pick(0,0),m_pick_visible(false),m_on_color_changed_callback(nullptr) {
    
    }
    /// @brief Indicates the selected color
    /// @return The color
    uix_pixel color() const {
        return m_color;
    }
    /// @brief Sets the color
    /// @param value The color
    void color(uix_pixel value) {
        if(m_color!=value) {
            if(m_pick_visible) {
                m_pick_visible=false;
                this->invalidate(gfx::srect16(m_pick.x,m_pick.y,m_pick.x+pick_size-1,m_pick.y+pick_size-1));
            }
            m_color = value;
            gfx::hsv_pixel<24> hsv(0,0,0);
            convert(m_color,&hsv,&hsv);
            m_hue = hsv.channel<gfx::channel_name::H>();
            m_saturation = hsv.channel<gfx::channel_name::S>();
            m_value = hsv.channel<gfx::channel_name::V>();
            if(m_on_color_changed_callback!=nullptr) {
                m_on_color_changed_callback(m_color,m_on_color_changed_callback_state);
            }
        }
    }
    /// @brief Sets the callback when the color changes
    /// @param callback The function to call
    /// @param state The user defined state to pass along with the color
    void on_color_changed_callback(on_color_changed_callback_type callback, void* state = nullptr) {
        m_on_color_changed_callback = callback;
        m_on_color_changed_callback_state = state;
    }
    /// @brief Gets the callback used when the color changes, or nullptr if none
    /// @return The callback
    on_color_changed_callback_type on_color_changed_callback() const {
        return m_on_color_changed_callback;
    }
    /// @brief Gets the user defined state passed along with the callback
    /// @return The state value
    void* on_color_changed_callback_state() const {
        return m_on_color_changed_callback_state;
    }
    /// @brief Sets the user defined state passed along with the callback
    /// @param value The state
    void on_color_changed_callback_state(void* value) {
        m_on_color_changed_callback_state=value;
    }
protected:
    /// @brief Copies the control
    /// @param value The control to copy
    void do_copy_control(const color_picker& value) {
        base_type::on_copy_control(value);
        m_hue=value.m_hue;
        m_saturation=value.m_saturation;
        m_value=value.m_value;
        m_color = value.m_color;
        m_pick = value.m_pick;
        m_pick_visible = value.m_pick_visible;
        m_on_color_changed_callback = value.m_on_color_changed_callback;
        m_on_color_changed_callback_state = value.m_on_color_changed_callback_state;
     }
    /// @brief Moves the control
    /// @param value The control to move
    void do_move_control(color_picker& value) {
        base_type::on_move_control(value);
        m_hue=value.m_hue;
        m_saturation=value.m_saturation;
        m_value=value.m_value;
        m_color = value.m_color;
        m_pick = value.m_pick;
        m_pick_visible = value.m_pick_visible;
        m_on_color_changed_callback = value.m_on_color_changed_callback;
        m_on_color_changed_callback_state = value.m_on_color_changed_callback_state;
    }
    /// @brief Paints the control
    /// @param destination The destination
    /// @param clip The clipping rectangle
    virtual void on_paint(control_surface_type& destination, const gfx::srect16& clip) override {
        const int16_t bar_size = destination.dimensions().height / 10;
        const int16_t width    = destination.dimensions().width;
        const int16_t height   = destination.dimensions().height;
        const int16_t region_h = height - bar_size;   // SV rows: 0 .. region_h-1

        // normalize clip bounds (in case x1>x2 / y1>y2) and clamp to the control
        int16_t cx1 = clip.x1 < clip.x2 ? clip.x1 : clip.x2;
        int16_t cx2 = clip.x1 < clip.x2 ? clip.x2 : clip.x1;
        int16_t cy1 = clip.y1 < clip.y2 ? clip.y1 : clip.y2;
        int16_t cy2 = clip.y1 < clip.y2 ? clip.y2 : clip.y1;
        if (cx1 < 0) { cx1 = 0; }
        if (cx2 > width - 1)  { cx2 = width - 1; }
        if (cy1 < 0) { cy1 = 0;  }
        if (cy2 > height - 1) {cy2 = height - 1;}

        // --- 2D S/V gradient: clipped, drawn as runs of equal color per row ---
        const int16_t sv_y1 = cy1;
        const int16_t sv_y2 = (cy2 < region_h - 1) ? cy2 : (int16_t)(region_h - 1);
        using px_t = typename control_surface_type::pixel_type;   // surface's native format
        for (int16_t y = sv_y1; y <= sv_y2; ++y) {
            uint8_t v = (region_h > 1)
                ? (uint8_t)(((long)(region_h - 1 - y) * 255) / (region_h - 1))
                : 255;

            int16_t run_start = cx1;
            px_t run_px;
            bool have_run = false;
            for (int16_t x = cx1; x <= cx2; ++x) {
                uint8_t s = (width > 1)
                    ? (uint8_t)(((long)x * 255) / (width - 1))
                    : 0;
                gfx::hsv_pixel<24> hsv(m_hue, s, v);
                px_t native;
                gfx::convert(hsv, &native);        // one conversion; compare in native space

                if (!have_run) {
                    run_start = x; run_px = native; have_run = true;
                } else if (native != run_px) {
                    gfx::draw::filled_rectangle(
                        destination,
                        gfx::srect16(run_start, y, (int16_t)(x - 1), y),
                        run_px);
                    run_start = x; run_px = native;
                }
            }
            if (have_run) {
                gfx::draw::filled_rectangle(
                    destination,
                    gfx::srect16(run_start, y, cx2, y),
                    run_px);
            }
        }

        // --- horizontal hue bar: skip entirely if clip misses its rows ---
        const int16_t hbar_y1 = height - bar_size;
        const int16_t hbar_y2 = height - 1;
        if (cy2 >= hbar_y1 && cy1 <= hbar_y2) {
            int16_t prev_x = 0;
            for (int h = 0; h <= 255; ++h) {
                int16_t next_x = (int16_t)(((long)(h + 1) * width) / 256);
                if (next_x > prev_x) {
                    // this block spans [prev_x, next_x-1]; draw only if it overlaps clip x
                    if (next_x - 1 >= cx1 && prev_x <= cx2) {
                        gfx::hsv_pixel<24> color(h, 255, 127);
                        gfx::draw::filled_rectangle(
                            destination,
                            gfx::srect16(prev_x, hbar_y1, (int16_t)(next_x - 1), hbar_y2),
                            color);
                    }
                    prev_x = next_x;
                }
            }
        }
        if(m_pick_visible) {
            const gfx::srect16 pick(m_pick.x,m_pick.y,m_pick.x+pick_size-1,m_pick.y+pick_size-1);
            gfx::draw::rectangle(destination,pick,uix_pixel(0,0,0,0xFF));
            gfx::draw::filled_rectangle(destination,pick.inflate(-1,-1),m_color);
        }
    }
    /// @brief Indicates the control was touched
    /// @param locations_size The count of points
    /// @param locations The touch points
    /// @return 
    virtual bool on_touch(size_t locations_size, const gfx::spoint16* locations) override {
        const int16_t width    = this->dimensions().width;
        const int16_t height   = this->dimensions().height;
        const int16_t bar_size = height / 10;
        const int16_t region_h = height - bar_size;   // SV plane rows: 0 .. region_h-1

        gfx::spoint16 pt = locations[0];
        
        if (pt.y >= height - bar_size) {
            // --- hue bar: inverse of  x = hue * width / 256 ---
            int h = ((int)pt.x * 256) / width;
            if (h > 255) h = 255;
            m_hue = (uint8_t)h;
            m_pick_visible = true;
            m_pick = locations[0].offset(4,-pick_size-4);
            gfx::hsv_pixel<24> hsv(m_hue,m_saturation,m_value);
            gfx::convert(hsv,&m_color);
            if(m_on_color_changed_callback!=nullptr) {
                m_on_color_changed_callback(m_color,m_on_color_changed_callback_state);
            }
            this->invalidate(); // force a repaint
        } else {
            // --- SV plane: inverse of the on_paint ramps ---
            // S min at left, max at right:  s = x * 255 / (width-1)
            m_saturation = (width > 1)
                ? (uint8_t)(((long)pt.x * 255) / (width - 1))
                : 0;
            // V max at top (y=0), min at bottom:  v = (region_h-1-y) * 255 / (region_h-1)
            int16_t yy = pt.y;
            if (yy > region_h - 1) yy = region_h - 1;   // in case bar rounding overlaps
            m_value = (region_h > 1)
                ? (uint8_t)(((long)(region_h - 1 - yy) * 255) / (region_h - 1))
                : 255;
            gfx::hsv_pixel<24> hsv(m_hue,m_saturation,m_value);
            gfx::convert(hsv,&m_color);
            if(m_on_color_changed_callback!=nullptr) {
                m_on_color_changed_callback(m_color,m_on_color_changed_callback_state);
            }
            if(m_pick_visible) {
                this->invalidate(gfx::srect16(m_pick.x,m_pick.y,m_pick.x+pick_size-1,m_pick.y+pick_size-1));
            }
            m_pick_visible = true;
            m_pick = locations[0].offset(4,-pick_size-4);
            if(m_pick.x<0) {
                m_pick.x += (pick_size+4);
            }
            if(m_pick.y<0) {
                m_pick.y += (pick_size+4);
            }
            if(m_pick.x+pick_size>this->dimensions().width) {
                m_pick.x = this->dimensions().width - pick_size;
            }
            if(m_pick.y+pick_size>this->dimensions().height) {
                m_pick.y = this->dimensions().height - pick_size;
            }
            this->invalidate(gfx::srect16(m_pick.x,m_pick.y,m_pick.x+pick_size-1,m_pick.y+pick_size-1));
        }

        return true;
    }
    /// @brief Indicates when the control has been released
    virtual void on_release() override {
        if(m_pick_visible) { // should always be true could assert it i suppose
            this->invalidate(gfx::srect16(m_pick.x,m_pick.y,m_pick.x+pick_size-1,m_pick.y+pick_size-1));
        }
        m_pick_visible = false;
    }  
};
}
#endif // HTCW_UIX_COLOR_PICKER_HPP