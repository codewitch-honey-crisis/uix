#ifndef HTCW_UIX_SCREEN_HPP
#define HTCW_UIX_SCREEN_HPP
#include <htcw_data.hpp>

#include "uix_core.hpp"
namespace uix {
enum struct screen_update_mode {
    // update parts pf the display using backbuffering
    partial = 0,
    // update the backbuffer containing the entire display directly
    direct = 1
};
enum struct screen_update_strategy {
    // full-dirty-width strips, top-to-bottom (your current behavior)
    throughput = 0,
    // full-width strips, but cut lines snap to control edges when possible
    balanced = 1,
    // guillotine partition: vertical cuts too, to avoid splitting controls
    minimize_paints = 2
};
class screen_base : public invalidation_tracker {
   public:
    /// @brief The callback for wait style DMA transfers
    typedef void (*on_wait_flush_callback_type)(void* state);
    /// @brief The flush callback for transfering data to the display.
    typedef void (*on_flush_callback_type)(const rect16& bounds,
                                           const void* bmp, void* state);
    /// @brief The touch callback for getting touch screen touch location
    /// information
    typedef void (*on_touch_callback_type)(point16* out_locations,
                                           size_t* in_out_locations_size,
                                           void* state);

    /// @brief Invalidate a rectangular region
    /// @param rect The region to invalidate
    /// @return The result of the operation
    virtual uix_result invalidate(const srect16& rect) = 0;
    /// @brief Marks all dirty rectangles as clean
    /// @return The result of the operation
    virtual uix_result validate_all() = 0;
    /// @brief Indicates the dimensions of the screen
    /// @return A ssize16 indicating the width and height.
    virtual ssize16 dimensions() const = 0;
    /// @brief Sets the dimensions of the screen
    /// @param value the new dimensions
    virtual void dimensions(ssize16 value) = 0;
    /// @brief Indicates the bounds of the screen. This is
    /// (0,0)-(Width-1,Height-1)
    /// @return an srect16 containing the bounds
    virtual srect16 bounds() const = 0;
    /// @brief Indicates whether the screen is currently in the middle of
    /// flushing. Unless update(false) is called or checked unsafely from
    /// another thread, this will always be false.
    /// @return True if the screen is currently flushing, otherwise false.
    virtual bool flushing() const = 0;
    /// @brief Indicates the update mode for the screen
    /// @return The update mode
    virtual screen_update_mode update_mode() const = 0;
    /// @brief Sets the update mode for the screen
    /// @param mode The update mode
    virtual void update_mode(screen_update_mode mode) = 0;
    /// @brief The strategy used to update the screen, either favoring minimum redraws, minimum transfers, or balanced
    /// @return The screen update strategy
    virtual screen_update_strategy update_strategy() const = 0;
    /// @brief The strategy used to update the screen, either favoring minimum redraws, minimum transfers, or balanced
    /// @param value The screen update strategy
    virtual void update_strategy(screen_update_strategy value) = 0;
    /// @brief Indicates the size of the transfer buffer(s)
    /// @return a size_t containing the size of the buffer
    virtual size_t buffer_size() const = 0;
    /// @brief Sets the size of the transfer buffer(s)
    /// @param value the new buffer size
    virtual void buffer_size(size_t value) = 0;
    /// @brief Gets the first or only buffer
    /// @return A pointer to the buffer
    virtual uint8_t* buffer1() = 0;
    /// @brief Sets the first or only buffer
    /// @param buffer A pointer to the new buffer
    virtual void buffer1(uint8_t* buffer) = 0;
    /// @brief Gets the second buffer
    /// @return A pointer to the buffer
    virtual uint8_t* buffer2() = 0;
    /// @brief Sets the second buffer
    /// @param buffer A pointer to the new buffer
    virtual void buffer2(uint8_t* buffer) = 0;
    /// @brief Invalidates the entire screen
    /// @return The result of the operation
    virtual uix_result invalidate() = 0;
    /// @brief Call when a flush has finished so the screen can recycle the
    /// buffers. Should either be called in the flush callback implementation
    /// (no DMA) or via a DMA completion callback that signals when the previous
    /// transfer was completed.
    virtual void flush_complete() = 0;
    /// @brief Retrieves the on_flush_callback pointer
    /// @return A pointer to the callback method
    virtual on_flush_callback_type on_flush_callback() const = 0;
    /// @brief Retrieves the flush callback state
    /// @return The user defined flush callback state
    virtual void* on_flush_callback_state() const = 0;
    /// @brief Sets the flush callback
    /// @param callback The callback that transfers data to the display
    /// @param state A user defined state value to pass to the callback
    virtual void on_flush_callback(on_flush_callback_type callback,
                                   void* state = nullptr) = 0;
    /// @brief Indicates the wait callback for wait style DMA completion
    /// @return A pointer to the callback method
    virtual on_wait_flush_callback_type on_wait_flush_callback() const = 0;
    /// @brief Retrieves the wait callback state
    /// @return The user defined wait callback state
    virtual void* on_wait_flush_callback_state() const = 0;
    /// @brief Sets the wait callback
    /// @param callback The callback that tells the MCU to wait for a previous
    /// DMA transfer to complete
    /// @param state A user defined state value to pass to the callback
    virtual void on_wait_flush_callback(on_wait_flush_callback_type callback,
                                        void* state = nullptr) = 0;
    /// @brief Retrieves the touch callback
    /// @return A pointer to the callback method
    virtual on_touch_callback_type on_touch_callback() const = 0;
    /// @brief Retrieves the touch callback state
    /// @return The user defined touch callback state
    virtual void* on_touch_callback_state() const = 0;
    /// @brief Sets the touch callback
    /// @param callback The callback that reports locations from a touch screen
    /// or pointer
    /// @param state A user defined state value to pass to the callback
    virtual void on_touch_callback(on_touch_callback_type callback,
                                   void* state = nullptr) = 0;
    /// @brief Updates the screen, processing touch input and updating and
    /// flushing invalid portions of the screen to the display
    /// @param full True to fully update the display, false to only update one
    /// subrect iteration rather than all dirty rectangles
    /// @return The result of the operation
    virtual uix_result update(bool full = true) = 0;
    /// @brief Indicates if the screen has any dirty regions to update and flush
    /// @return True if the screen needs updating, otherwise false
    virtual bool dirty() const = 0;
    virtual bool flush_pending() const = 0;
};
/// @brief Represents a screen
/// @tparam BitmapType The type of backing bitmap used over the transfer buffer.
/// This is what is drawn to by the controls.
/// @tparam HorizontalAlignment The update rectangle alignment on the x-axis
/// @tparam VerticalAlignment The update rectangle alignment on the y-axis
template <typename BitmapType, uint8_t HorizontalAlignment = 1,
          uint8_t VerticalAlignment = 1>
class screen_ex final : public screen_base {
   public:
    using type = screen_ex;
    using native_bitmap_type = gfx::bitmap<typename BitmapType::pixel_type,
                                           typename BitmapType::palette_type>;
    using bitmap_type = BitmapType;
    using pixel_type = typename bitmap_type::pixel_type;
    using palette_type = typename bitmap_type::palette_type;
    using control_surface_type = control_surface<BitmapType>;
    using control_type = control<control_surface_type>;
    // /// @brief The callback for wait style DMA transfers like those used with
    // GFX typedef void(*on_wait_flush_callback_type)(void* state);
    // /// @brief The flush callback for transfering data to the display.
    // typedef void(*on_flush_callback_type)(const rect16& bounds,const void*
    // bmp,void* state);
    // /// @brief The touch callback for getting touch screen touch location
    // information typedef void(*on_touch_callback_type)(point16*
    // out_locations,size_t* in_out_locations_size,void* state);
    /// @brief The update rectangle alignment on the x-axis
    constexpr static const uint8_t horizontal_alignment = HorizontalAlignment;
    /// @brief The update rectangle alignment on the y-axis
    constexpr static const uint8_t vertical_alignment = VerticalAlignment;

   private:
    struct tracker_entry {
        control_type* ctrl;
        // 0 = nothing called yet
        // 1 = on_before_paint called
        // 2 = on_after_render_called
        int state;
    };
    using dirty_rects_type = data::simple_vector<rect16>;
    using controls_type = data::simple_vector<tracker_entry>;

    screen_ex(const screen_ex& rhs) = delete;
    screen_ex& operator=(const screen_ex& rhs) = delete;
    void do_move(screen_ex& rhs) {
        m_dimensions = rhs.m_dimensions;
        m_buffer_size = rhs.m_buffer_size;
        rhs.m_buffer_size = 0;
        m_write_buffer = rhs.m_write_buffer;
        m_buffer1 = rhs.m_buffer1;
        m_buffer2 = rhs.m_buffer2;
        m_palette = rhs.m_palette;
        m_flushing = rhs.m_flushing;
        m_on_wait_flush_callback = rhs.m_on_wait_flush_callback;
        rhs.m_on_wait_flush_callback = nullptr;
        m_on_wait_flush_callback_state = rhs.m_on_wait_flush_callback_state;
        m_on_flush_callback = rhs.m_on_flush_callback;
        rhs.m_on_flush_callback = nullptr;
        m_on_flush_callback_state = rhs.m_on_flush_callback_state;
        m_dirty_rects = helpers::uix_move(m_dirty_rects);
        m_controls = helpers::uix_move(m_controls);
        m_background_color = rhs.m_background_color;
        m_it_dirties = rhs.m_it_dirties;
        rhs.m_it_dirties = nullptr;
        m_update_strategy = rhs.m_update_strategy;
        m_active_strategy = rhs.m_active_strategy;
        m_rendering = rhs.m_rendering;
        m_strip_y = rhs.m_strip_y;
        for (uint8_t i = 0; i < region_stack_size; ++i)
            m_region_stack[i] = rhs.m_region_stack[i];
        m_sp = rhs.m_sp;
        m_banding = rhs.m_banding;
        m_band_region = rhs.m_band_region;
        m_last_touched = rhs.m_last_touched;
        m_on_touch_callback = rhs.m_on_touch_callback;
        rhs.m_on_touch_callback = nullptr;
        m_on_touch_callback_state = rhs.m_on_touch_callback_state;
        m_flush_pending = rhs.m_flush_pending;
        m_flush_pending_bounds = rhs.m_flush_pending_bounds;
        m_update_mode = rhs.m_update_mode;
    }

    template <typename T>
    constexpr static T h_align_up(T value) {
        if (value % horizontal_alignment != 0)
            value += (T)(horizontal_alignment - value % horizontal_alignment);
        return value;
    }
    template <typename T>
    constexpr static T h_align_down(T value) {
        value -= value % horizontal_alignment;
        return value;
    }
    template <typename T>
    constexpr static T v_align_up(T value) {
        if (value % vertical_alignment != 0)
            value += (T)(vertical_alignment - value % vertical_alignment);
        return value;
    }
    template <typename T>
    constexpr static T v_align_down(T value) {
        value -= value % vertical_alignment;
        return value;
    }
    constexpr static rect16 align_up(const rect16& value) {
        int x2 = h_align_up(value.x2);
        if (horizontal_alignment != 1) {
            --x2;
        }
        int y2 = v_align_up(value.y2);
        if (vertical_alignment != 1) {
            --y2;
        }
        return rect16(h_align_down(value.x1), v_align_down(value.y1), x2, y2);
    }
    bool switch_buffers() {
        if (m_buffer2 != nullptr) {
            if (m_buffer1 == m_write_buffer) {
                m_write_buffer = m_buffer2;
            } else {
                if (m_on_wait_flush_callback != nullptr) {
                    m_on_wait_flush_callback(m_on_wait_flush_callback_state);
                    m_flushing = 0;
                }
                m_write_buffer = m_buffer1;
            }
            return true;
        } else {
            if (m_on_wait_flush_callback != nullptr) {
                m_on_wait_flush_callback(m_on_wait_flush_callback_state);
                m_flushing = 0;
            }
        }
        return false;
    }
    enum struct plan_status { has_tile, done, out_of_memory };

    static int s_iabs(int v) { return v < 0 ? -v : v; }

    bool fits_buffer(const rect16& r) const {
        return native_bitmap_type::sizeof_buffer(
                   size16(r.width(), r.height())) <= m_buffer_size;
    }
    // max vertically-aligned line count of width w that fits one buffer
    uint16_t max_lines_for(uint16_t w) const {
        size_t stride = native_bitmap_type::sizeof_buffer(size16(w, 1));
        if (stride == 0) return 0;
        int lines = v_align_down((int)(m_buffer_size / stride));
        if (lines > dimensions().height) lines = dimensions().height;
        while (lines > 0 &&
               native_bitmap_type::sizeof_buffer(size16(w, (uint16_t)lines)) >
                   m_buffer_size) {
            lines -= 1;
        }
        return (uint16_t)lines;
    }

    // A horizontal cut at row `cy` (next strip starts at cy) is "clean" if it
    // does not slice through any visible control within R.
    bool is_clean_hcut(const rect16& R, uint16_t cy) const {
        for (typename controls_type::const_iterator it = m_controls.cbegin();
             it != m_controls.cend(); ++it) {
            control_type* p = it->ctrl;
            if (!p->visible()) continue;
            srect16 cb = p->bounds();
            if (!cb.intersects((srect16)R)) continue;
            srect16 ci = cb.crop((srect16)R);
            if ((int)ci.y1 < (int)cy && (int)cy <= (int)ci.y2) return false;
        }
        return true;
    }
    bool is_clean_vcut(const rect16& R, uint16_t cx) const {
        for (typename controls_type::const_iterator it = m_controls.cbegin();
             it != m_controls.cend(); ++it) {
            control_type* p = it->ctrl;
            if (!p->visible()) continue;
            srect16 cb = p->bounds();
            if (!cb.intersects((srect16)R)) continue;
            srect16 ci = cb.crop((srect16)R);
            if ((int)ci.x1 < (int)cx && (int)cx <= (int)ci.x2) return false;
        }
        return true;
    }
    // best clean vertical cut (aligned, interior), nearest R's horizontal mid
    bool find_clean_vcut(const rect16& R, uint16_t& out_cx) const {
        int mid = (int)R.x1 + (int)R.width() / 2;
        bool found = false; int best = 0, bestd = 0x7fffffff;
        for (typename controls_type::const_iterator it = m_controls.cbegin();
             it != m_controls.cend(); ++it) {
            control_type* p = it->ctrl;
            if (!p->visible()) continue;
            srect16 cb = p->bounds();
            if (!cb.intersects((srect16)R)) continue;
            srect16 ci = cb.crop((srect16)R);
            int cand[2] = { h_align_down((int)ci.x1),
                            h_align_up((int)ci.x2 + 1) };
            for (int k = 0; k < 2; ++k) {
                int c = cand[k];
                if (c <= (int)R.x1 || c > (int)R.x2) continue;
                if (!is_clean_vcut(R, (uint16_t)c)) continue;
                int d = s_iabs(c - mid);
                if (d < bestd) { bestd = d; best = c; found = true; }
            }
        }
        if (found) out_cx = (uint16_t)best;
        return found;
    }
    bool find_clean_hcut(const rect16& R, uint16_t& out_cy) const {
        int mid = (int)R.y1 + (int)R.height() / 2;
        bool found = false; int best = 0, bestd = 0x7fffffff;
        for (typename controls_type::const_iterator it = m_controls.cbegin();
             it != m_controls.cend(); ++it) {
            control_type* p = it->ctrl;
            if (!p->visible()) continue;
            srect16 cb = p->bounds();
            if (!cb.intersects((srect16)R)) continue;
            srect16 ci = cb.crop((srect16)R);
            int cand[2] = { v_align_down((int)ci.y1),
                            v_align_up((int)ci.y2 + 1) };
            for (int k = 0; k < 2; ++k) {
                int c = cand[k];
                if (c <= (int)R.y1 || c > (int)R.y2) continue;
                if (!is_clean_hcut(R, (uint16_t)c)) continue;
                int d = s_iabs(c - mid);
                if (d < bestd) { bestd = d; best = c; found = true; }
            }
        }
        if (found) out_cy = (uint16_t)best;
        return found;
    }

    plan_status throughput_next(rect16& out) {
        for (;;) {
            if (m_it_dirties == m_dirty_rects.cend()) return plan_status::done;
            rect16 D = align_up(*m_it_dirties);
            if (m_strip_y > D.y2) {
                ++m_it_dirties;
                if (m_it_dirties == m_dirty_rects.cend())
                    return plan_status::done;
                m_strip_y = align_up(*m_it_dirties).y1;
                continue;
            }
            uint16_t ml = max_lines_for(D.width());
            if (ml == 0) return plan_status::out_of_memory;
            int cut = (int)m_strip_y + ml;
            if (cut > (int)D.y2 + 1) cut = (int)D.y2 + 1;
            out = rect16(D.x1, m_strip_y, D.x2, (uint16_t)(cut - 1));
            m_strip_y = (uint16_t)cut;
            return plan_status::has_tile;
        }
    }

    plan_status balanced_next(rect16& out) {
        for (;;) {
            if (m_it_dirties == m_dirty_rects.cend()) return plan_status::done;
            rect16 D = align_up(*m_it_dirties);
            if (m_strip_y > D.y2) {
                ++m_it_dirties;
                if (m_it_dirties == m_dirty_rects.cend())
                    return plan_status::done;
                m_strip_y = align_up(*m_it_dirties).y1;
                continue;
            }
            uint16_t ml = max_lines_for(D.width());
            if (ml == 0) return plan_status::out_of_memory;
            int forced = (int)m_strip_y + ml;
            if (forced > (int)D.y2 + 1) forced = (int)D.y2 + 1;
            int chosen;
            if (is_clean_hcut(D, (uint16_t)forced)) {
                chosen = forced;  // the full strip already lands in a gap
            } else {
                // pull the cut up to the largest clean control-edge boundary
                int best = 0; bool found = false;
                for (typename controls_type::const_iterator it =
                         m_controls.cbegin();
                     it != m_controls.cend(); ++it) {
                    control_type* p = it->ctrl;
                    if (!p->visible()) continue;
                    srect16 cb = p->bounds();
                    if (!cb.intersects((srect16)D)) continue;
                    srect16 ci = cb.crop((srect16)D);
                    int cand[2] = { v_align_down((int)ci.y1),
                                    v_align_up((int)ci.y2 + 1) };
                    for (int k = 0; k < 2; ++k) {
                        int c = cand[k];
                        if (c <= (int)m_strip_y || c > forced) continue;
                        if (!is_clean_hcut(D, (uint16_t)c)) continue;
                        if (c > best) { best = c; found = true; }
                    }
                }
                chosen = found ? best : forced;  // forced => unavoidable split
            }
            out = rect16(D.x1, m_strip_y, D.x2, (uint16_t)(chosen - 1));
            m_strip_y = (uint16_t)chosen;
            return plan_status::has_tile;
        }
    }

    plan_status guillotine_next(rect16& out) {
        for (;;) {
            if (m_banding) {
                uint16_t ml = max_lines_for(m_band_region.width());
                if (ml == 0) return plan_status::out_of_memory;
                if (ml >= m_band_region.height()) {
                    out = m_band_region;
                    m_banding = false;
                    return plan_status::has_tile;
                }
                out = rect16(m_band_region.x1, m_band_region.y1,
                             m_band_region.x2,
                             (uint16_t)(m_band_region.y1 + ml - 1));
                m_band_region = rect16(m_band_region.x1,
                                       (uint16_t)(m_band_region.y1 + ml),
                                       m_band_region.x2, m_band_region.y2);
                return plan_status::has_tile;
            }
            if (m_sp == 0) {
                if (m_it_dirties == m_dirty_rects.cend())
                    return plan_status::done;
                m_region_stack[m_sp++] = align_up(*m_it_dirties);
                ++m_it_dirties;
            }
            rect16 R = m_region_stack[--m_sp];
            if (fits_buffer(R)) { out = R; return plan_status::has_tile; }
            uint16_t cx = 0, cy = 0;
            bool hv = find_clean_vcut(R, cx);
            bool hh = find_clean_hcut(R, cy);
            if (hv || hh) {
                bool use_v;
                if (hv && hh) {
                    int dv = s_iabs((int)cx - ((int)R.x1 + (int)R.width() / 2));
                    int dh = s_iabs((int)cy - ((int)R.y1 + (int)R.height() / 2));
                    use_v = (dv <= dh);  // prefer the more balanced split
                } else {
                    use_v = hv;
                }
                rect16 a, b;
                if (use_v) {
                    a = rect16(R.x1, R.y1, (uint16_t)(cx - 1), R.y2);
                    b = rect16(cx, R.y1, R.x2, R.y2);
                } else {
                    a = rect16(R.x1, R.y1, R.x2, (uint16_t)(cy - 1));
                    b = rect16(R.x1, cy, R.x2, R.y2);
                }
                if ((int)m_sp + 2 > (int)region_stack_size) {
                    // stack would overflow -> band R in place (never drops area)
                    m_banding = true; m_band_region = R; continue;
                }
                m_region_stack[m_sp++] = b;  // process 'a' (top/left) first
                m_region_stack[m_sp++] = a;
                continue;
            }
            // no clean cut on either axis -> forced band
            m_banding = true; m_band_region = R; continue;
        }
    }

    plan_status next_tile(rect16& out) {
        switch (m_active_strategy) {
            case screen_update_strategy::throughput: return throughput_next(out);
            case screen_update_strategy::balanced:   return balanced_next(out);
            default:                                 return guillotine_next(out);
        }
    }

    void planner_init() {
        m_rendering = true;
        m_active_strategy = m_update_strategy;
        m_it_dirties = m_dirty_rects.cbegin();
        // too many disjoint dirty rects for the guillotine stack? degrade.
        if (m_active_strategy == screen_update_strategy::minimize_paints &&
            m_dirty_rects.size() > region_stack_size) {
            m_active_strategy = screen_update_strategy::balanced;
        }
        if (m_active_strategy == screen_update_strategy::throughput ||
            m_active_strategy == screen_update_strategy::balanced) {
            m_strip_y = align_up(*m_it_dirties).y1;
        } else {
            m_sp = 0;
            m_banding = false;
        }
    }

    void finalize_paint() {
        for (typename controls_type::iterator it = m_controls.begin();
             it != m_controls.end(); ++it) {
            if (it->state == 1) {
                it->ctrl->on_after_paint();
                it->state = 0;
            }
        }
        m_it_dirties = nullptr;
        m_rendering = false;
        m_banding = false;
        m_sp = 0;
    }

    // identical to your existing per-band paint loop; shared by all strategies
    void render_subrect(const srect16& subrect, uint8_t* buf) {
        bitmap_type bmp((size16)subrect.dimensions(), buf, m_palette);
        bmp.fill(bmp.bounds(), m_background_color);
        for (typename controls_type::iterator ctl_it = m_controls.begin();
             ctl_it != m_controls.end(); ++ctl_it) {
            control_type* pctl = ctl_it->ctrl;
            if (pctl->visible() && pctl->bounds().intersects(subrect)) {
                srect16 surface_rect = pctl->bounds();
                spoint16 bmp_offset(surface_rect.x1 - subrect.x1,
                                    surface_rect.y1 - subrect.y1);
                surface_rect.offset_inplace(-subrect.x1, -subrect.y1);
                srect16 surface_clip = pctl->bounds().crop(subrect);
                surface_clip.offset_inplace(-pctl->bounds().x1,
                                            -pctl->bounds().y1);
                control_surface_type surface(bmp, surface_rect, bmp_offset);
                if (ctl_it->state == 0) {
                    pctl->on_before_paint();
                    ctl_it->state = 1;
                }
                pctl->on_paint(surface, surface_clip);
            }
        }
    }
    typename controls_type::iterator find_touch_target(
        spoint16 pt, typename controls_type::iterator pend = nullptr) {
        // loop through the controls in z-order back to front
        // find the last/front-most control whose bounds()
        // intersect the first touch point
        if (pend == nullptr) {
            pend = m_controls.end();
        }
        typename controls_type::iterator target = nullptr;
        for (typename controls_type::iterator ctl_it = m_controls.begin();
             ctl_it != pend; ++ctl_it) {
            control_type* pctl = ctl_it->ctrl;
            if (pctl->visible() && pctl->bounds().intersects(pt)) {
                target = ctl_it;
            }
        }
        return target;
    }
    uix_result update_impl() {
        // we had to early exit the last time
        if (m_flush_pending) {
            if (m_flushing) {
                return uix_result::success;
            }
            // Serial.println("Initiating pending flush");
            m_flushing = 1;
            m_flush_pending = false;
            uint8_t* buf = (uint8_t*)m_write_buffer;
            switch_buffers();
            // initiate the DMA transfer on whatever was *previously*
            // m_write_buffer before switch_buffers was called.
            // delay(50);
            m_on_flush_callback(
                m_flush_pending_bounds, buf,
                m_on_flush_callback_state);  // initiate DMA transfer
            // Serial.println("Pending flush started. Early out");
            return uix_result::success;
        }
        // if not rendering, process touch
        if (!m_rendering && m_on_touch_callback != nullptr) {
            point16 locs[5];
            spoint16 slocs[5];
            size_t locs_size = sizeof(locs);
            m_on_touch_callback(locs, &locs_size, m_on_touch_callback_state);
            if (locs_size > 0) {
                // if we currently have a touched control
                // forward all successive messages to that control
                // even if they're outside the control bounds.
                // that way we can do dragging if necessary.
                // this works like MS Windows.
                if (m_last_touched != nullptr) {
                    // offset the touch points to the control and then
                    // call on_touch for the control
                    for (size_t i = 0; i < locs_size; ++i) {
                        slocs[i].x = locs[i].x -
                                     (int16_t)m_last_touched->ctrl->bounds().x1;
                        slocs[i].y = locs[i].y -
                                     (int16_t)m_last_touched->ctrl->bounds().y1;
                    }
                    m_last_touched->ctrl->on_touch(locs_size, slocs);

                } else {
                    // loop through the controls in z-order back to front
                    // find the last/front-most control whose bounds()
                    // intersect the first touch point
                    spoint16 tpt = (spoint16)locs[0];
                    typename controls_type::iterator ptarget =
                        find_touch_target(tpt);
                    if (ptarget != nullptr) {
                        for (size_t i = 0; i < locs_size; ++i) {
                            slocs[i].x = locs[i].x -
                                         (int16_t)(ptarget->ctrl)->bounds().x1;
                            slocs[i].y = locs[i].y -
                                         (int16_t)(ptarget->ctrl)->bounds().y1;
                        }
                        while (ptarget != nullptr &&
                               !(ptarget->ctrl)->on_touch(locs_size, slocs)) {
                            ptarget = find_touch_target(tpt, ptarget);
                        }
                        if (ptarget != nullptr) {
                            m_last_touched = ptarget;
                        }
                    }
                }
            } else {
                // released. if we have an active control let it know.
                if (m_last_touched != nullptr) {
                    m_last_touched->ctrl->on_release();
                    m_last_touched = nullptr;
                }
            }
        }
        switch (m_update_mode) {
                // rendering process
                // note we skip this until we have a free buffer
            case screen_update_mode::partial: {
                if (m_on_flush_callback != nullptr && m_buffer_size != 0 &&
                    m_buffer1 != nullptr && m_dirty_rects.size() != 0) {
                    // single-buffer: wait for the in-flight flush to finish
                    if (m_buffer2 == nullptr && m_flushing) {
                        return uix_result::success;
                    }
                    if (!m_rendering) {
                        planner_init();
                    }
                    rect16 tile;
                    plan_status st = next_tile(tile);
                    if (st == plan_status::out_of_memory) {
                        finalize_paint();
                        return uix_result::out_of_memory;
                    }
                    if (st == plan_status::done) {
                        finalize_paint();
                        return validate_all();
                    }
                    srect16 subrect = (srect16)tile;
                    uint8_t* buf = (uint8_t*)m_write_buffer;
                    render_subrect(subrect, buf);
                    // DMA double-buffer early-exit (unchanged)
                    if (m_buffer2 != nullptr && m_flushing) {
                        m_flush_pending_bounds = (rect16)subrect;
                        m_flush_pending = true;
                        return uix_result::success;
                    }
                    switch_buffers();
                    m_flushing = 1;
                    m_on_flush_callback((rect16)subrect, buf,
                                        m_on_flush_callback_state);
                }
            } break;
            case screen_update_mode::direct: {
                if (m_buffer_size != 0 && m_buffer1 != nullptr && m_dirty_rects.size() != 0) {
                    bitmap_type bmp((size16)this->dimensions(), m_buffer1, m_palette);
                    for (auto it_d = m_dirty_rects.cbegin(); it_d != m_dirty_rects.cend(); ++it_d) {
                        rect16 r = *it_d;
                        srect16 subrect = (srect16)r;
                        bmp.fill(r, m_background_color);
                        for (typename controls_type::iterator ctl_it = m_controls.begin();
                             ctl_it != m_controls.end(); ++ctl_it) {
                            control_type* pctl = ctl_it->ctrl;
                            // if it's visible and intersects this subrect
                            if (pctl->visible() && pctl->bounds().intersects(subrect)) {
                                // create the offset surface rectangle for drawing
                                srect16 surface_rect = pctl->bounds();
                                spoint16 bmp_offset(0, 0);
                                // create the clip rectangle for the control
                                srect16 surface_clip = pctl->bounds().crop(subrect);
                                surface_clip.offset_inplace(-pctl->bounds().x1,
                                                            -pctl->bounds().y1);
                                // create the control surface
                                control_surface_type surface(bmp, surface_rect, bmp_offset);
                                // if we haven't called on_before_paint, do so now
                                if (ctl_it->state == 0) {
                                    pctl->on_before_paint();
                                    ctl_it->state = 1;
                                }
                                // and paint
                                pctl->on_paint(surface, surface_clip);
                                if (m_on_flush_callback != nullptr) {
                                    // tell it we're flushing and run the callback
                                    m_flushing = 1;
                                    m_on_flush_callback(
                                        (rect16)subrect, m_buffer1,
                                        m_on_flush_callback_state); 
                                }
                            }
                        }
                    }
                    // if we're at the end, shut it down
                    // first tell any necessary controls we're done
                    // rendering
                    for (typename controls_type::iterator it =
                             m_controls.begin();
                         it != m_controls.end(); ++it) {
                        if (it->state == 1) {
                            it->ctrl->on_after_paint();
                            it->state = 0;
                        }
                    }

                    return validate_all();
                }
            } break;
            default:
                break;
        }

        return uix_result::success;
    }
    ssize16 m_dimensions;
    size_t m_buffer_size;
    volatile uint8_t* m_write_buffer;
    uint8_t *m_buffer1, *m_buffer2;
    const palette_type* m_palette;
    volatile int m_flushing;
    on_wait_flush_callback_type m_on_wait_flush_callback;
    void* m_on_wait_flush_callback_state;
    on_flush_callback_type m_on_flush_callback;
    void* m_on_flush_callback_state;
    dirty_rects_type m_dirty_rects;
    controls_type m_controls;
    pixel_type m_background_color;
    typename dirty_rects_type::const_iterator m_it_dirties;
    on_touch_callback_type m_on_touch_callback;
    void* m_on_touch_callback_state;
    typename controls_type::iterator m_last_touched;
    bool m_flush_pending;
    rect16 m_flush_pending_bounds;
    screen_update_mode m_update_mode;
    screen_update_strategy m_update_strategy;   // requested strategy
    screen_update_strategy m_active_strategy;   // strategy for the current frame (may degrade)
    bool m_rendering;                           // true while a frame is being tiled
    uint16_t m_strip_y;                         // cursor for throughput/balanced
    static constexpr uint8_t region_stack_size = 24;  // ~200 bytes, fixed, no heap
    rect16 m_region_stack[region_stack_size];   // guillotine work stack
    uint8_t m_sp;                               // stack pointer
    bool m_banding;                             // guillotine forced-band fallback active
    rect16 m_band_region;                       // remaining region being force-banded
    
   public:
    /// @brief Constructs a screen given a buffer size, and one or two buffers,
    /// plus an optional palette
    /// @param dimensions The width and height of the screen as a ssize16
    /// @param buffer_size The size of each buffer. Larger is better for
    /// performance, but takes more RAM
    /// @param buffer The first buffer. If DMA is not available, this will be
    /// the only buffer.
    /// @param buffer2 The second buffer. If DMA is available this is used to
    /// increase performance. Both buffers must be the same size.
    /// @param palette The associated palette. This is used for things like
    /// color e-ink displays
    /// @param allocator The memory allocator to use for the controls (malloc)
    /// @param reallocator The memory reallocator to use for the controls
    /// (realloc)
    /// @param deallocator The memory deallocator to use for the controls (free)
    screen_ex(ssize16 dimensions, size_t buffer_size, uint8_t* buffer,
              uint8_t* buffer2 = nullptr, const palette_type* palette = nullptr,
              void*(allocator)(size_t) = ::malloc,
              void*(reallocator)(void*, size_t) = ::realloc,
              void(deallocator)(void*) = ::free)
        : m_dimensions(dimensions),
          m_buffer_size(buffer_size),
          m_write_buffer(buffer),
          m_buffer1(buffer),
          m_buffer2(buffer2),
          m_palette(palette),
          m_flushing(0),
          m_on_wait_flush_callback(nullptr),
          m_on_wait_flush_callback_state(nullptr),
          m_on_flush_callback(nullptr),
          m_on_flush_callback_state(nullptr),
          m_dirty_rects(allocator, reallocator, deallocator),
          m_controls(allocator, reallocator, deallocator),
          m_background_color(pixel_type()),
          m_it_dirties(nullptr),
          m_on_touch_callback(nullptr),
          m_on_touch_callback_state(nullptr),
          m_last_touched(nullptr),
          m_flush_pending(false),
          m_update_mode(screen_update_mode::partial),
          m_update_strategy(screen_update_strategy::balanced),
          m_active_strategy(screen_update_strategy::balanced),
          m_rendering(false),
          m_strip_y(false),
          m_sp(0),
          m_banding(false)
          {}
    /// @brief Constructs an uninitialized screen instance
    /// @param allocator The memory allocator to use for the controls (malloc)
    /// @param reallocator The memory reallocator to use for the controls
    /// (realloc)
    /// @param deallocator The memory deallocator to use for the controls (free)
    screen_ex(void*(allocator)(size_t) = ::malloc,
              void*(reallocator)(void*, size_t) = ::realloc,
              void(deallocator)(void*) = ::free)
        : m_dimensions(0, 0),
          m_buffer_size(0),
          m_write_buffer(nullptr),
          m_buffer1(nullptr),
          m_buffer2(nullptr),
          m_palette(nullptr),
          m_flushing(0),
          m_on_wait_flush_callback(nullptr),
          m_on_wait_flush_callback_state(nullptr),
          m_on_flush_callback(nullptr),
          m_on_flush_callback_state(nullptr),
          m_dirty_rects(allocator, reallocator, deallocator),
          m_controls(allocator, reallocator, deallocator),
          m_background_color(pixel_type()),
          m_it_dirties(nullptr),
          m_on_touch_callback(nullptr),
          m_on_touch_callback_state(nullptr),
          m_last_touched(nullptr),
          m_flush_pending(false),
          m_update_mode(screen_update_mode::partial),
          m_update_strategy(screen_update_strategy::balanced),
          m_active_strategy(screen_update_strategy::balanced),
          m_rendering(false),
          m_strip_y(false),
          m_sp(0),
          m_banding(false) {}
    /// @brief Moves a screen
    /// @param rhs The screen to move
    screen_ex(screen_ex&& rhs) { do_move_control(rhs); }
    /// @brief Moves a screen
    /// @param rhs The screen to move
    /// @return this
    screen_ex& operator=(screen_ex&& rhs) {
        do_move_control(rhs);
        return *this;
    }
    /// @brief Indicates the dimensions of the screen
    /// @return A ssize16 indicating the width and height.
    virtual ssize16 dimensions() const override { return m_dimensions; }
    /// @brief Sets the dimensions of the screen
    /// @param value the new dimensions
    virtual void dimensions(ssize16 value) override {
        if (value.width < 1 || value.height < 1) {
            return;
        }
        m_dimensions = value;
        // TODO: implement a resize event
    }
    /// @brief Indicates the bounds of the screen. This is
    /// (0,0)-(Width-1,Height-1)
    /// @return an srect16 containing the bounds
    virtual srect16 bounds() const override { return dimensions().bounds(); }
    /// @brief Indicates whether the screen is currently in the middle of
    /// flushing. Unless update(false) is called or checked unsafely from
    /// another thread, this will always be false.
    /// @return True if the screen is currently flushing, otherwise false.
    virtual bool flushing() const override { return m_flushing != 0; }
    /// @brief Indicates the update mode for the screen
    /// @return The update mode
    virtual screen_update_mode update_mode() const override {
        return m_update_mode;
    }
    /// @brief Sets the update mode for the screen
    /// @param value The update mode
    virtual void update_mode(screen_update_mode value) override {
        m_update_mode = value;
    }
    /// @brief The strategy used to update the screen, either favoring minimum redraws, minimum transfers, or balanced
    /// @return The screen update strategy
    virtual screen_update_strategy update_strategy() const override {
        return m_update_strategy;
    }
    /// @brief The strategy used to update the screen, either favoring minimum redraws, minimum transfers, or balanced
    /// @param value The screen update strategy
    virtual void update_strategy(screen_update_strategy value) override {
        m_update_strategy = value;
    }
    /// @brief Indicates the size of the transfer buffer(s)
    /// @return a size_t containing the size of the buffer
    virtual size_t buffer_size() const override { return m_buffer_size; }
    /// @brief Sets the size of the transfer buffer(s)
    /// @param value the new buffer size
    virtual void buffer_size(size_t value) override { m_buffer_size = value; }
    /// @brief Gets the first or only buffer
    /// @return A pointer to the buffer
    virtual uint8_t* buffer1() override { return m_buffer1; }
    /// @brief Sets the first or only buffer
    /// @param buffer A pointer to the new buffer
    virtual void buffer1(uint8_t* buffer) override {
        m_buffer1 = buffer;
        if (m_write_buffer == nullptr || m_write_buffer != m_buffer2) {
            m_write_buffer = buffer;
        }
    }
    /// @brief Gets the second buffer
    /// @return A pointer to the buffer
    virtual uint8_t* buffer2() override { return m_buffer2; }
    /// @brief Sets the second buffer
    /// @param buffer A pointer to the new buffer
    virtual void buffer2(uint8_t* buffer) override {
        m_buffer2 = buffer;
        if (m_write_buffer == nullptr || m_write_buffer != m_buffer1) {
            m_write_buffer = buffer;
        }
    }
    /// @brief The background color of the screen, in the screen's native pixel
    /// format.
    /// @return The background color
    pixel_type background_color() const { return m_background_color; }
    /// @brief Sets the background color of the screen, in the screen's native
    /// pixel format
    /// @param value The background color
    void background_color(pixel_type value) {
        m_background_color = value;
        invalidate();
    }
    /// @brief Invalidates the entire screen
    /// @return The result of the operation
    virtual uix_result invalidate() override {
        validate_all();
        return this->invalidate(this->bounds());
    }
    /// @brief Invalidates a particular rectangular region
    /// @param rect The rectangular region to invalidate
    /// @return The result of the operation
    virtual uix_result invalidate(const srect16& rect) override {
        if (bounds().intersects(rect)) {
            rect16 r = (rect16)rect.crop(bounds());
            r.normalize_inplace();
            for (rect16* it = m_dirty_rects.begin(); it != m_dirty_rects.end();
                 ++it) {
                if (it->contains(r)) {
                    // // Serial.printf("Dirty rects count:
                    // %d\n",m_dirty_rects.size());
                    return uix_result::success;
                }
            }
            bool done = false;
            while (!done) {
                done = true;
                for (rect16* it = m_dirty_rects.begin();
                     it != m_dirty_rects.end(); ++it) {
                    if (!it->contains(r) && !r.contains(*it) &&
                        r.intersects(*it)) {
                        r = r.merge(*it);
                        done = false;
                        break;
                    }
                }
            }
            for (rect16* it = m_dirty_rects.begin(); it != m_dirty_rects.end();
                 ++it) {
                if (r.contains(*it)) {
                    m_dirty_rects.erase(it, it);
                    --it;
                }
            }
            // // Serial.printf("Dirty rects count: %d\n",m_dirty_rects.size());
            return m_dirty_rects.push_back(r) ? uix_result::success
                                              : uix_result::out_of_memory;
        }
        // // Serial.printf("Dirty rects count: %d\n",m_dirty_rects.size());
        return uix_result::success;
    }
    /// @brief Marks all dirty rectangles as valid
    /// @return The result of the operation
    virtual uix_result validate_all() override {
        // // Serial.println("validate all");
        m_dirty_rects.clear();
        return uix_result::success;
    }
    /// @brief Unregisters all of the controls
    /// @return The result of the operation
    uix_result unregister_controls() {
        bool should_invalidate = m_controls.size() == 0;
        validate_all();
        m_controls.clear();
        if (should_invalidate) {
            return invalidate();
        }
        return uix_result::success;
    }
    /// @brief Registers a control with the screen
    /// @param control The control to register
    /// @return The result of the operation
    uix_result register_control(control_type& control) {
        tracker_entry entry;
        entry.ctrl = &control;
        entry.state = 0;
        if (m_controls.push_back(entry)) {
            control.parent(*this);
            return invalidate(control.bounds());
        }
        return uix_result::out_of_memory;
    }
    /// @brief Call when a flush has finished so the screen can recycle the
    /// buffers. Should either be called in the flush callback implementation
    /// (no DMA) or via a DMA completion callback that signals when the previous
    /// transfer was completed.
    virtual void flush_complete() override { m_flushing = 0; }
    /// @brief sets the palette for the screen
    /// @param value a pointer to the palette instance
    void palette(const palette_type* value) { m_palette = value; }
    /// @brief indicates the palette for the screen
    /// @return A pointer to the palette instance
    const palette_type* palette() const { return m_palette; }
    /// @brief Retrieves the on_flush_callback pointer
    /// @return A pointer to the callback method
    virtual on_flush_callback_type on_flush_callback() const override {
        return m_on_flush_callback;
    }
    /// @brief Retrieves the flush callback state
    /// @return The user defined flush callback state
    virtual void* on_flush_callback_state() const override {
        return m_on_flush_callback_state;
    }
    /// @brief Sets the flush callback
    /// @param callback The callback that transfers data to the display
    /// @param state A user defined state value to pass to the callback
    virtual void on_flush_callback(on_flush_callback_type callback,
                                   void* state = nullptr) override {
        m_on_flush_callback = callback;
        m_on_flush_callback_state = state;
    }
    /// @brief Indicates the wait callback for wait style DMA completion
    /// @return A pointer to the callback method
    virtual on_wait_flush_callback_type on_wait_flush_callback()
        const override {
        return m_on_wait_flush_callback;
    }
    /// @brief Retrieves the wait callback state
    /// @return The user defined wait callback state
    virtual void* on_wait_flush_callback_state() const override {
        return m_on_wait_flush_callback_state;
    }
    /// @brief Sets the wait callback
    /// @param callback The callback that tells the MCU to wait for a previous
    /// DMA transfer to complete
    /// @param state A user defined state value to pass to the callback
    virtual void on_wait_flush_callback(on_wait_flush_callback_type callback,
                                        void* state = nullptr) override {
        m_on_wait_flush_callback = callback;
        m_on_wait_flush_callback_state = state;
    }
    /// @brief Retrieves the touch callback
    /// @return A pointer to the callback method
    virtual on_touch_callback_type on_touch_callback() const override {
        return m_on_touch_callback;
    }
    /// @brief Retrieves the touch callback state
    /// @return The user defined touch callback state
    virtual void* on_touch_callback_state() const override {
        return m_on_touch_callback_state;
    }
    /// @brief Sets the touch callback
    /// @param callback The callback that reports locations from a touch screen
    /// or pointer
    /// @param state A user defined state value to pass to the callback
    virtual void on_touch_callback(on_touch_callback_type callback,
                                   void* state = nullptr) override {
        m_on_touch_callback = callback;
        m_on_touch_callback_state = state;
    }
    virtual bool flush_pending() const {
        return m_flush_pending || m_flushing;
    }
    /// @brief Updates the screen, processing touch input and updating and
    /// flushing invalid portions of the screen to the display
    /// @param full True to fully update the display, false to only update one
    /// subrect iteration rather than all dirty rectangles
    /// @return The result of the operation
    virtual uix_result update(bool full = true) override {
        uix_result res = update_impl();
        if (res != uix_result::success) {
            return res;
        }
        while (full && m_rendering) {
            res = update_impl();
            if (m_flush_pending) {
                return uix_result::success;
            }
            if (res != uix_result::success) {
                return res;
            }
        }
        return uix_result::success;
    }

    /// @brief Indicates if the screen has any dirty regions to update and flush
    /// @return True if the screen needs updating, otherwise false
    virtual bool dirty() const override {
        return this->m_dirty_rects.size() != 0;
    }
};
/// @brief A convenience wrapper for screen_ex<> that is simpler to use
/// @tparam PixelType The type of pixel used in the display, like
/// gfx::rgb_pixel<16>
/// @tparam PaletteType The palette type, provided with the display drivers that
/// use them (if using htcw drivers)
template <typename PixelType,
          typename PaletteType = gfx::palette<PixelType, PixelType>>
using screen = screen_ex<gfx::bitmap<PixelType, PaletteType>>;
}  // namespace uix
#endif