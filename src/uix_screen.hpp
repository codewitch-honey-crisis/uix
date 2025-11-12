#ifndef HTCW_UIX_SCREEN_HPP
#define HTCW_UIX_SCREEN_HPP
#include <htcw_data.hpp>

#include "uix_core.hpp"
namespace uix {
enum struct screen_update_mode {
    partial = 0,
    direct = 1
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
        m_bmp_lines = rhs.m_bmp_lines;
        rhs.m_bmp_lines = 0;
        m_bmp_y = rhs.m_bmp_y;
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
        if (m_it_dirties == nullptr && m_on_touch_callback != nullptr) {
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
            case screen_update_mode::partial: {
                // rendering process
                // note we skip this until we have a free buffer
                if (m_on_flush_callback != nullptr && m_buffer_size != 0 &&
                    m_buffer1 != nullptr && m_dirty_rects.size() != 0) {
                    // wait for flush completion
                    if (m_buffer2 == nullptr) {
                        if (m_flushing) {
                            return uix_result::success;
                        }
                    }
                    if (m_it_dirties == nullptr) {
                        // m_it_dirties is null when not rendering
                        // so basically when it's null this is the first call
                        // and we initialize some stuff
                        m_it_dirties = m_dirty_rects.cbegin();
                        const rect16 aligned = align_up(*m_it_dirties);
                        size_t bmp_stride = native_bitmap_type::sizeof_buffer(
                            size16(aligned.width(), 1));
                        size_t bmp_min = native_bitmap_type::sizeof_buffer(
                            size16(aligned.width(), v_align_up(1)));
                        m_bmp_lines = v_align_down(m_buffer_size / bmp_stride);
                        if (m_bmp_lines > dimensions().height) {
                            m_bmp_lines = dimensions().height;
                        }
                        while (native_bitmap_type::sizeof_buffer(aligned.width(), m_bmp_lines) > m_buffer_size) {
                            m_bmp_lines -= 1;
                        }
                        if (bmp_min > m_buffer_size) {
                            return uix_result::out_of_memory;
                        }
                        m_bmp_y = 0;
                    } else {
                        // if we're past the current
                        // dirty rectangle bounds:
                        rect16 aligned = align_up(*m_it_dirties);
                        if (m_bmp_y + aligned.y1 + m_bmp_lines >= aligned.y2) {
                            // go to the next dirty rectangle
                            ++m_it_dirties;
                            if (m_it_dirties == m_dirty_rects.cend()) {
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
                                // clear all dirty rects
                                m_it_dirties = nullptr;
                                return validate_all();
                            }
                            aligned = align_up(*m_it_dirties);
                            // now we compute the bitmap stride (one line, in bytes)
                            size_t bmp_stride = native_bitmap_type::sizeof_buffer(
                                size16(aligned.width(), 1));
                            size_t bmp_min = native_bitmap_type::sizeof_buffer(
                                size16(aligned.width(), v_align_up(1)));
                            // now we figure out how many lines we can have in these
                            // subrects based on the total memory we're working with
                            m_bmp_lines = v_align_down(m_buffer_size / bmp_stride);
                            if (m_bmp_lines > dimensions().height) {
                                m_bmp_lines = dimensions().height;
                            }
                            while (native_bitmap_type::sizeof_buffer(aligned.width(), m_bmp_lines) > m_buffer_size) {
                                m_bmp_lines -= 1;
                            }
                            // if we don't have enough space for at least one line,
                            // error out
                            if (bmp_min > m_buffer_size) {
                                return uix_result::out_of_memory;
                            }
                            // start at the top of the dirty rectangle:
                            m_bmp_y = 0;
                        } else {
                            while (native_bitmap_type::sizeof_buffer(aligned.width(), m_bmp_lines) > m_buffer_size) {
                                m_bmp_lines -= 1;
                            }
                            // move down to the next subrect
                            m_bmp_y += m_bmp_lines;
                        }
                    }
                    const rect16 aligned = align_up(*m_it_dirties);
                    // create a subrect the same width as the dirty, and m_bmp_lines
                    // high starting at m_bmp_y within the dirty rectangle
                    srect16 subrect(aligned.x1, aligned.y1 + m_bmp_y, aligned.x2,
                                    aligned.y1 + m_bmp_lines + m_bmp_y - 1);
                    // make sure the subrect is cropped within the bounds
                    // of the dirties. sometimes the last one overhangs.
                    subrect = subrect.crop((srect16)aligned);
                    // create a bitmap for the subrect over the write buffer
                    uint8_t* buf = (uint8_t*)m_write_buffer;
                    // assert(bitmap_type::sizeof_buffer((size16)subrect.dimensions())<=m_buffer_size);
                    bitmap_type bmp((size16)subrect.dimensions(), buf, m_palette);
                    // fill it with the screen color
                    // Serial.println("Start painting controls");

                    bmp.fill(bmp.bounds(), m_background_color);
                    // Serial.println("Background painted (buffer touched)");
                    // for each control
                    for (typename controls_type::iterator ctl_it = m_controls.begin();
                         ctl_it != m_controls.end(); ++ctl_it) {
                        control_type* pctl = ctl_it->ctrl;
                        // if it's visible and intersects this subrect
                        if (pctl->visible() && pctl->bounds().intersects(subrect)) {
                            // create the offset surface rectangle for drawing
                            srect16 surface_rect = pctl->bounds();
                            spoint16 bmp_offset(surface_rect.x1 - subrect.x1,
                                                surface_rect.y1 - subrect.y1);
                            surface_rect.offset_inplace(-subrect.x1, -subrect.y1);
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
                        }
                    }
                    // Serial.println("Done painting controls");
                    if (m_buffer2 != nullptr) {
                        if (m_flushing) {
                            m_flush_pending_bounds = (rect16)subrect;
                            m_flush_pending = true;
                            // Serial.println("Awaiting DMA transfer");
                            return uix_result::success;
                        }
                    }
                    // Serial.println("DMA available. Switching buffers");
                    // switch out m_write_buffer so if it points to m_buffer1 it now
                    // points to m_buffer2 and vice versa. if there's just one buffer,
                    // we don't change anything.
                    switch_buffers();
                    // Serial.println("Initiating flush");
                    // tell it we're flushing and run the callback
                    m_flushing = 1;
                    // initiate the DMA transfer on whatever was *previously*
                    // m_write_buffer before switch_buffers was called.
                    m_on_flush_callback(
                        (rect16)subrect, buf,
                        m_on_flush_callback_state);  // initiate DMA transfer
                    // Serial.println("Flush started");
                    // the above may return immediately before the
                    // transfer is complete. To take advantage of
                    // this, rather than wait, we swap out to a
                    // second buffer and continue drawing while
                    // the transfer is in progress. That's what
                    // switch_buffers() is doing beforehand just above
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
    uint16_t m_bmp_lines;
    uint16_t m_bmp_y;
    on_touch_callback_type m_on_touch_callback;
    void* m_on_touch_callback_state;
    typename controls_type::iterator m_last_touched;
    bool m_flush_pending;
    rect16 m_flush_pending_bounds;
    screen_update_mode m_update_mode;

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
          m_bmp_lines(0),
          m_bmp_y(0),
          m_on_touch_callback(nullptr),
          m_on_touch_callback_state(nullptr),
          m_last_touched(nullptr),
          m_flush_pending(false),
          m_update_mode(screen_update_mode::partial) {}
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
          m_bmp_lines(0),
          m_bmp_y(0),
          m_on_touch_callback(nullptr),
          m_on_touch_callback_state(nullptr),
          m_last_touched(nullptr),
          m_flush_pending(false) {}
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
    virtual screen_update_mode update_mode() const {
        return m_update_mode;
    }
    /// @brief Sets the update mode for the screen
    /// @param value The update mode
    virtual void update_mode(screen_update_mode value) {
        m_update_mode = value;
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
        while (full && m_it_dirties != nullptr) {
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