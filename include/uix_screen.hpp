#ifndef HTCW_UIX_SCREEN_HPP
#define HTCW_UIX_SCREEN_HPP
#include "uix_core.hpp"
namespace uix {
    template<uint16_t Width, uint16_t Height, typename PixelType,typename PaletteType = gfx::palette<PixelType,PixelType>>
    class screen final : public invalidation_tracker {
    public:
        using type = screen;
        using pixel_type = PixelType;
        using palette_type = PaletteType;
        using bitmap_type = gfx::bitmap<pixel_type,palette_type>;
        using control_type = control<pixel_type,palette_type>;
        using control_surface_type = control_surface<pixel_type,palette_type>;
        typedef void(*on_flush_callback_type)(point16 location,gfx::bitmap<pixel_type,palette_type>& bmp,void* state);
        typedef void(*on_touch_callback_type)(point16* out_locations,size_t* in_out_locations_size,void* state);
    private:
        screen(const screen& rhs)=delete;
        screen& operator=(const screen& rhs)=delete;
        void do_move(screen& rhs) {
            m_buffer_size = rhs.m_buffer_size;
            rhs.m_buffer_size = 0;
            m_write_buffer = rhs.m_write_buffer;
            m_buffer1 = rhs.m_buffer1;
            m_buffer2 = rhs.m_buffer2;
            m_palette = rhs.m_palette;
            m_flushing = rhs.m_flushing;
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
            m_on_touch_callback = rhs.m_on_touch_callback;
            rhs.m_on_touch_callback = nullptr;
            m_on_touch_callback_state = rhs.m_on_touch_callback_state;
        }
        bool switch_buffers() {
            if(m_buffer2!=nullptr) {
                if(m_buffer1==m_write_buffer) {
                    m_write_buffer = m_buffer2;
                } else {
                    m_write_buffer = m_buffer1;
                }
                return true;
            }
            return false;
        }
        control_type** find_touch_target(spoint16 pt, control_type** pend = nullptr) {
            // loop through the controls in z-order back to front
            // find the last/front-most control whose bounds()
            // intersect the first touch point
            if(pend==nullptr) {
                pend = m_controls.end();
            }
            control_type** target = nullptr;
            for(control_type** ctl_it = m_controls.begin();ctl_it!=pend;++ctl_it) {
                control_type* pctl = *ctl_it;
                if(pctl->visible() && pctl->bounds().intersects(pt)) {
                    target = ctl_it;
                }
            }
            return target;
        }
        uix_result update_impl() {
            // if not rendering, process touch
            if(m_it_dirties==nullptr&& m_on_touch_callback!=nullptr) {
                point16 locs[2];
                spoint16 slocs[2];
                size_t locs_size = sizeof(locs);
                m_on_touch_callback(locs,&locs_size,m_on_touch_callback_state);
                if(locs_size>0) {
                    // if we currently have a touched control
                    // forward all successive messages to that control
                    // even if they're outside the control bounds.
                    // that way we can do dragging if necessary.
                    // this works like MS Windows.
                    if(m_last_touched!=nullptr) {
                        // offset the touch points to the control and then 
                        // call on_touch for the control
                        for(int i = 0;i<locs_size;++i) {
                            slocs[i].x = locs[i].x-(int16_t)m_last_touched->bounds().x1;
                            slocs[i].y = locs[i].y-(int16_t)m_last_touched->bounds().y1;
                        }
                        m_last_touched->on_touch(locs_size,slocs);
          
                    } else {
                         // loop through the controls in z-order back to front
                        // find the last/front-most control whose bounds()
                        // intersect the first touch point
                        spoint16 tpt = (spoint16)locs[0];
                        control_type** ptarget = find_touch_target(tpt);
                        if(ptarget!=nullptr) {
                            for(int i = 0;i<locs_size;++i) {
                                slocs[i].x = locs[i].x-(int16_t)(*ptarget)->bounds().x1;
                                slocs[i].y = locs[i].y-(int16_t)(*ptarget)->bounds().y1;
                            }
                            while(ptarget!=nullptr && !(*ptarget)->on_touch(locs_size,slocs)) {
                                ptarget = find_touch_target(tpt,ptarget);
                            }
                            m_last_touched = *ptarget;
                        }
                    }
                } else {
                    // released. if we have an active control let it know.
                    if(m_last_touched!=nullptr) {
                        m_last_touched->on_release();
                        m_last_touched = nullptr;

                    }
                }
            }
            // rendering process
            // note we skip this until we have a free buffer
            if(m_on_flush_callback!=nullptr && 
                    m_flushing<(1+(m_buffer2!=nullptr)) && 
                    m_dirty_rects.size()!=0) {
                if(m_it_dirties==nullptr) {
                    // m_it_dirties is null when not rendering
                    // so basically when it's null this is the first call
                    // and we initialize some stuff
                    m_it_dirties = m_dirty_rects.cbegin();
                    size_t bmp_stride = bitmap_type::sizeof_buffer(size16(m_it_dirties->width(),1));
                    m_bmp_lines = m_buffer_size/bmp_stride;
                    if(bmp_stride>m_buffer_size) {
                        return uix_result::out_of_memory;
                    }
                    m_bmp_y = 0;
                } else {
                    // if we're past the current 
                    // dirty rectangle bounds:
                    if(m_bmp_y+m_it_dirties->y1+m_bmp_lines>m_it_dirties->y2) {
                        // go to the next dirty rectangle
                        ++m_it_dirties;
                        if(m_it_dirties==m_dirty_rects.cend()) {
                            // if we're at the end, shut it down
                            // and clear all dirty rects
                            m_it_dirties = nullptr;
                            return validate_all();
                        }
                        // now we compute the bitmap stride (one line, in bytes)
                        size_t bmp_stride = bitmap_type::sizeof_buffer(size16(m_it_dirties->width(),1));
                        // now we figure out how many lines we can have in these
                        // subrects based on the total memory we're working with
                        m_bmp_lines = m_buffer_size/bmp_stride;
                        // if we don't have enough space for at least one line,
                        // error out
                        if(bmp_stride>m_buffer_size) {
                            return uix_result::out_of_memory;
                        }
                        // start at the top of the dirty rectangle:
                        m_bmp_y = 0;
                    } else {
                        // move down to the next subrect
                        m_bmp_y+=m_bmp_lines;
                    }
                }
                // create a subrect the same width as the dirty, and m_bmp_lines high
                // starting at m_bmp_y within the dirty rectangle
                srect16 subrect(m_it_dirties->x1,m_it_dirties->y1+m_bmp_y,m_it_dirties->x2, m_it_dirties->y1+m_bmp_lines+m_bmp_y-1);
                // make sure the subrect is cropped within the bounds
                // of the dirties. sometimes the last one overhangs.
                subrect=subrect.crop((srect16)*m_it_dirties);
                // create a bitmap for the subrect over the write buffer
                bitmap_type bmp((size16)subrect.dimensions(),m_write_buffer,m_palette);
                // fill it with the screen color
                bmp.fill(bmp.bounds(),m_background_color);
                // for each control
                for(control_type** ctl_it = m_controls.begin();ctl_it!=m_controls.end();++ctl_it) {
                    control_type* pctl = *ctl_it;
                    // if it's visible and intersects this subrect
                    if(pctl->visible() && pctl->bounds().intersects(subrect)) {
                        // create the offset surface rectangle for drawing
                        srect16 surface_rect = pctl->bounds();
                        surface_rect.offset_inplace(-subrect.x1,-subrect.y1);
                        // create the clip rectangle for the control
                        srect16 surface_clip = pctl->bounds().crop(subrect);
                        surface_clip.offset_inplace(-pctl->bounds().x1,-pctl->bounds().y1);
                        // create the control surface
                        control_surface_type surface(bmp,surface_rect);
                        // and paint
                        pctl->on_paint(surface,surface_clip);
                    }
                }
                // tell it we're flushing and run the callback
                ++m_flushing;
                m_on_flush_callback((point16)subrect.top_left(),bmp,m_on_flush_callback_state);
                // the above may return immediately before the 
                // transfer is complete. To take advantage of
                // this, rather than wait, we swap out to a
                // second buffer and continue drawing while
                // the transfer is in progress.
                switch_buffers();
            }
            return uix_result::success;
        }
        using dirty_rects_type = data::simple_vector<rect16>;
        using controls_type = data::simple_vector<control_type*>;
        size_t m_buffer_size;
        uint8_t* m_write_buffer;
        uint8_t* m_buffer1, *m_buffer2;
        const palette_type* m_palette;
        int m_flushing;
        on_flush_callback_type m_on_flush_callback;
        void* m_on_flush_callback_state;
        dirty_rects_type m_dirty_rects;
        controls_type m_controls;
        pixel_type m_background_color;
        const rect16* m_it_dirties;
        uint16_t m_bmp_lines;
        uint16_t m_bmp_y;
        on_touch_callback_type m_on_touch_callback;
        void* m_on_touch_callback_state;
        control_type* m_last_touched;
    public:
        screen(size_t buffer_size, uint8_t* buffer, uint8_t* buffer2 = nullptr, const palette_type* palette = nullptr)
                : m_buffer_size(buffer_size), 
                    m_write_buffer(buffer),
                    m_buffer1(buffer),
                    m_buffer2(buffer2),
                    m_palette(palette),
                    m_flushing(0),
                    m_on_flush_callback(nullptr),
                    m_on_flush_callback_state(nullptr),
                    m_background_color(pixel_type()),
                    m_it_dirties(nullptr),
                    m_bmp_lines(0),
                    m_bmp_y(0),
                    m_on_touch_callback(nullptr),
                    m_on_touch_callback_state(nullptr),
                    m_last_touched(nullptr)
                {
        }
        screen(screen&& rhs) {
            do_move(rhs);
        }
        screen& operator=(screen&& rhs) {
            do_move(rhs);
            return *this;
        }
        ssize16 dimensions() const {
            return {Width,Height};
        }
        srect16 bounds() const {
            return dimensions().bounds();
        }
        pixel_type background_color() const {
            return m_background_color;
        }
        void background_color(pixel_type value) {
            m_background_color = value;
            invalidate(bounds());
        }
        virtual uix_result invalidate(const srect16& rect) override {
            if(bounds().intersects(rect)) {
                rect16 r = (rect16)rect.crop(bounds());
                for(rect16* it = m_dirty_rects.begin();it!=m_dirty_rects.end();++it) {
                    if(it->contains(r)) {
                        return uix_result::success;
                    } else if(r.contains(*it)) {
                        it->x1 = r.x1;
                        it->y1 = r.y1;
                        it->x2 = r.x2;
                        it->y2 = r.y2;
                        return uix_result::success;
                    }
                }
                return m_dirty_rects.push_back(r)?uix_result::success:uix_result::out_of_memory;
            }
            return uix_result::success;
        }
        virtual uix_result validate_all() override {
            m_dirty_rects.clear();
            return uix_result::success;
        }
        uix_result deregister_controls() {
            bool should_invalidate = m_controls.size()==0;
            m_controls.clear();
            if(should_invalidate) {
                return invalidate(bounds());
            }
            return uix_result::success;
        }
        uix_result register_control(control_type& control) {
            if(m_controls.push_back(&control)) {
                return invalidate(control.bounds());
            }
            return uix_result::out_of_memory;
        }
        void set_flush_complete() {
            m_flushing = false;
        }
        const palette_type* palette() const {
            return m_palette;
        }
        on_flush_callback_type on_flush_callback() const {
            return m_on_flush_callback;
        }
        void* on_flush_callback_state() const {
            return m_on_flush_callback_state;
        }
        void on_flush_callback(on_flush_callback_type callback, void* state = nullptr) {
            m_on_flush_callback = callback;
            m_on_flush_callback_state = state;
        }
        on_touch_callback_type on_touch_callback() const {
            return m_on_touch_callback;
        }
        void* on_touch_callback_state() const {
            return m_on_touch_callback_state;
        }
        void on_touch_callback(on_touch_callback_type callback, void* state = nullptr) {
            m_on_touch_callback = callback;
            m_on_touch_callback_state = state;
        }
        uix_result update(bool full = true) {
            uix_result res = update_impl();
            if(res!=uix_result::success) {
                return res;
            }
            while(full && m_it_dirties!=nullptr) {
                res = update_impl();
                if(res!=uix_result::success) {
                    return res;
                }   
            }
            return uix_result::success;
        }
    };
}
#endif