#ifndef HTCW_UIX_SCREEN_HPP
#define HTCW_UIX_SCREEN_HPP
#include "uix_core.hpp"
#include <htcw_data.hpp>
namespace uix {
    template<uint16_t Width, uint16_t Height, typename BitmapType, uint8_t HorizontalAlignment = 1, uint8_t VerticalAlignment = 1>
    class screen_ex final : public invalidation_tracker {
    public:
        using type = screen_ex;
        using native_bitmap_type = gfx::bitmap<typename BitmapType::pixel_type,typename BitmapType::palette_type>;
        using bitmap_type = BitmapType;
        using pixel_type = typename bitmap_type::pixel_type;
        using palette_type = typename bitmap_type::palette_type;
        using control_surface_type = control_surface<BitmapType>;
        using control_type = control<control_surface_type>;
        typedef void(*wait_flush_callback_type)(void* state);
        typedef void(*on_flush_callback_type)(const rect16& bounds,const void* bmp,void* state);
        typedef void(*on_touch_callback_type)(point16* out_locations,size_t* in_out_locations_size,void* state);
        constexpr static const uint8_t horizontal_alignment = HorizontalAlignment;
        constexpr static const uint8_t vertical_alignment = VerticalAlignment;
    private:
        screen_ex(const screen_ex& rhs)=delete;
        screen_ex& operator=(const screen_ex& rhs)=delete;
        void do_move(screen_ex& rhs) {
            m_buffer_size = rhs.m_buffer_size;
            rhs.m_buffer_size = 0;
            m_write_buffer = rhs.m_write_buffer;
            m_buffer1 = rhs.m_buffer1;
            m_buffer2 = rhs.m_buffer2;
            m_palette = rhs.m_palette;
            m_flushing = rhs.m_flushing;
            m_wait_flush_callback = rhs.m_wait_flush_callback;
            rhs.m_wait_flush_callback = nullptr;
            m_wait_flush_callback_state = rhs.m_wait_flush_callback_state;
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
            mem_area = rhs.mem_area;
        }
        size_t compute_mem_area() const {
            size_t cur = 0;
            size_t i;
            for(i = 1;i<uint16_t(-1);++i) {
                cur = bitmap_type::sizeof_buffer(size16(i,1));
                if(cur>m_buffer_size) {
                    break;
                }
            }
            return i-1;
        }
        size_t cached_mem_area() {
            if(mem_area==0) {
                mem_area = compute_mem_area();
            }
            return mem_area;
        }
        template<typename T>
        constexpr static T h_align_up(T value) {
            if (value % horizontal_alignment != 0)
                value += (T)(horizontal_alignment - value % horizontal_alignment);
            return value;
        }
        template<typename T>
        constexpr static T h_align_down(T value) {
            value -= value % horizontal_alignment;
            return value;
        }
        template<typename T>
        constexpr static T v_align_up(T value) {
            if (value % vertical_alignment != 0)
                value += (T)(vertical_alignment - value % vertical_alignment);
            return value;
        }
        template<typename T>
        constexpr static T v_align_down(T value) {
            value -= value % vertical_alignment;
            return value;
        }
        constexpr static rect16 align_up(const rect16& value) {
            int x2 = h_align_up(value.x2);
            if(horizontal_alignment!=1) {
                --x2;
            }
            int y2 = v_align_up(value.y2);
            if(vertical_alignment!=1) {
                --y2;
            }
            return rect16(h_align_down(value.x1),v_align_down(value.y1),x2,y2);
        }
        bool switch_buffers() {
            if(m_buffer2!=nullptr) {
                if(m_buffer1==m_write_buffer) {
                    m_write_buffer = m_buffer2;
                } else {
                    if(m_wait_flush_callback!=nullptr) {
                        m_wait_flush_callback(m_wait_flush_callback_state);
                        m_flushing=0;
                    }
                    m_write_buffer = m_buffer1;
                }
                return true;
            } else {
                if(m_wait_flush_callback!=nullptr) {
                    m_wait_flush_callback(m_wait_flush_callback_state);
                    m_flushing=0;
                }
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
                point16 locs[5];
                spoint16 slocs[5];
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
                        for(size_t i = 0;i<locs_size;++i) {
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
                            for(size_t i = 0;i<locs_size;++i) {
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
                    m_buffer_size!=0 &&
                    m_buffer1!=nullptr&&
                    m_flushing<(1+(m_buffer2!=nullptr)) && 
                    m_dirty_rects.size()!=0) {
                //Serial.println("!");
                if(m_it_dirties==nullptr) {
                    // m_it_dirties is null when not rendering
                    // so basically when it's null this is the first call
                    // and we initialize some stuff
                    m_it_dirties = m_dirty_rects.cbegin();
                    const rect16 aligned = align_up(*m_it_dirties);
                    size_t bmp_stride = native_bitmap_type::sizeof_buffer(size16(aligned.width(),1));
                    size_t bmp_min = native_bitmap_type::sizeof_buffer(size16(aligned.width(),v_align_up(1)));
                    m_bmp_lines = v_align_down(m_buffer_size/bmp_stride);
                    if(m_bmp_lines>dimensions().height) {
                        m_bmp_lines=dimensions().height;
                    }
                    if(bmp_min>m_buffer_size) {
                        return uix_result::out_of_memory;
                    }
                    m_bmp_y = 0;
                } else {
                    // if we're past the current 
                    // dirty rectangle bounds:
                    rect16 aligned = align_up(*m_it_dirties);
                    if(m_bmp_y+aligned.y1+m_bmp_lines>=aligned.y2) {
                        // go to the next dirty rectangle
                        ++m_it_dirties;
                        if(m_it_dirties==m_dirty_rects.cend()) {
                            // if we're at the end, shut it down
                            // and clear all dirty rects
                            m_it_dirties = nullptr;
                            return validate_all();
                        }
                        aligned = align_up(*m_it_dirties);
                        // now we compute the bitmap stride (one line, in bytes)
                        size_t bmp_stride = native_bitmap_type::sizeof_buffer(size16(aligned.width(),1));
                        size_t bmp_min = native_bitmap_type::sizeof_buffer(size16(aligned.width(),v_align_up(1)));
                        // now we figure out how many lines we can have in these
                        // subrects based on the total memory we're working with
                        m_bmp_lines = v_align_down(m_buffer_size/bmp_stride);
                        if(m_bmp_lines>dimensions().height) {
                            m_bmp_lines=dimensions().height;
                        }
                        // if we don't have enough space for at least one line,
                        // error out
                        if(bmp_min>m_buffer_size) {
                            return uix_result::out_of_memory;
                        }
                        // start at the top of the dirty rectangle:
                        m_bmp_y = 0;
                    } else {
                        // move down to the next subrect
                        m_bmp_y+=m_bmp_lines;
                    }
                }
                const rect16 aligned = align_up(*m_it_dirties);
                // create a subrect the same width as the dirty, and m_bmp_lines high
                // starting at m_bmp_y within the dirty rectangle
                srect16 subrect(aligned.x1,aligned.y1+m_bmp_y,aligned.x2, aligned.y1+m_bmp_lines+m_bmp_y-1);
                // make sure the subrect is cropped within the bounds
                // of the dirties. sometimes the last one overhangs.
                subrect=subrect.crop((srect16)aligned);
                // create a bitmap for the subrect over the write buffer
                uint8_t* buf = (uint8_t*)m_write_buffer;
                //assert(bitmap_type::sizeof_buffer((size16)subrect.dimensions())<=m_buffer_size);
                bitmap_type bmp((size16)subrect.dimensions(),buf,m_palette);
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
                m_on_flush_callback((rect16)subrect,bmp.begin(),m_on_flush_callback_state);
                // the above may return immediately before the 
                // transfer is complete. To take advantage of
                // this, rather than wait, we swap out to a
                // second buffer and continue drawing while
                // the transfer is in progress.
                switch_buffers();
                //Serial.println("#");
            }
            return uix_result::success;
        }
        using dirty_rects_type = data::simple_vector<rect16>;
        using controls_type = data::simple_vector<control_type*>;
        size_t m_buffer_size;
        volatile uint8_t* m_write_buffer;
        uint8_t* m_buffer1, *m_buffer2;
        const palette_type* m_palette;
        volatile int m_flushing;
        wait_flush_callback_type m_wait_flush_callback;
        void* m_wait_flush_callback_state;
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
        size_t mem_area;
    public:
        screen_ex(size_t buffer_size, uint8_t* buffer, uint8_t* buffer2 = nullptr, const palette_type* palette = nullptr)
                : m_buffer_size(buffer_size), 
                    m_write_buffer(buffer),
                    m_buffer1(buffer),
                    m_buffer2(buffer2),
                    m_palette(palette),
                    m_flushing(0),
                    m_wait_flush_callback(nullptr),
                    m_wait_flush_callback_state(nullptr),
                    m_on_flush_callback(nullptr),
                    m_on_flush_callback_state(nullptr),
                    m_background_color(pixel_type()),
                    m_it_dirties(nullptr),
                    m_bmp_lines(0),
                    m_bmp_y(0),
                    m_on_touch_callback(nullptr),
                    m_on_touch_callback_state(nullptr),
                    m_last_touched(nullptr),
                    mem_area(0)
                {
        }
        screen_ex()
                : m_buffer_size(0), 
                    m_write_buffer(nullptr),
                    m_buffer1(nullptr),
                    m_buffer2(nullptr),
                    m_palette(nullptr),
                    m_flushing(0),
                    m_wait_flush_callback(nullptr),
                    m_wait_flush_callback_state(nullptr),
                    m_on_flush_callback(nullptr),
                    m_on_flush_callback_state(nullptr),
                    m_background_color(pixel_type()),
                    m_it_dirties(nullptr),
                    m_bmp_lines(0),
                    m_bmp_y(0),
                    m_on_touch_callback(nullptr),
                    m_on_touch_callback_state(nullptr),
                    m_last_touched(nullptr),
                    mem_area(0)
                {
        }
        screen_ex(screen_ex&& rhs) {
            do_move(rhs);
        }
        screen_ex& operator=(screen_ex&& rhs) {
            do_move(rhs);
            return *this;
        }
        ssize16 dimensions() const {
            return {Width,Height};
        }
        srect16 bounds() const {
            return dimensions().bounds();
        }
        bool flushing() const {
            return m_flushing!=0;
        }
        size_t buffer_size() const {
            return m_buffer_size;
        }
        void buffer_size(size_t value) {
            m_buffer_size = value;
        }
        uint8_t* buffer1() {
            return m_buffer1;
        }
        void buffer1(uint8_t* buffer) {
            m_buffer1=buffer;
            if(m_write_buffer==nullptr || m_write_buffer!=m_buffer2) {
                m_write_buffer = buffer;
            }
        }
        uint8_t* buffer2() {
            return m_buffer2;
        }
        void buffer2(uint8_t* buffer) {
            m_buffer2 = buffer;
            if(m_write_buffer==nullptr || m_write_buffer!=m_buffer1) {
                m_write_buffer = buffer;
            }
        }
        
        pixel_type background_color() const {
            return m_background_color;
        }
        void background_color(pixel_type value) {
            m_background_color = value;
            invalidate(bounds());
        }
        uix_result invalidate() {
            return this->invalidate(this->bounds());
        }
        virtual uix_result invalidate(const srect16& rect) override {
            if(bounds().intersects(rect)) {
                rect16 r = (rect16)rect.crop(bounds());
                r.normalize_inplace();
                for(rect16* it = m_dirty_rects.begin();it!=m_dirty_rects.end();++it) {
                    if(it->contains(r)) {
                        return uix_result::success;
                    } else if(r.contains(*it)) {
                        it->x1 = r.x1;
                        it->y1 = r.y1;
                        it->x2 = r.x2;
                        it->y2 = r.y2;
                        return uix_result::success;
                    } else if(it->intersects(r)) {
                        it->x1 = r.x1<it->x1?r.x1:it->x1;
                        it->y1 = r.y1<it->y1?r.y1:it->y1;
                        it->x2 = r.x2>it->x2?r.x2:it->x2;
                        it->y2 = r.y2>it->y2?r.y2:it->y2;
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
            validate_all();
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
        void palette(const palette_type* value) {
            m_palette = value;
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
        wait_flush_callback_type wait_flush_callback() const {
            return m_wait_flush_callback;
        }
        void* wait_flush_callback_state() const {
            return m_wait_flush_callback_state;
        }
        void wait_flush_callback(wait_flush_callback_type callback, void* state = nullptr) {
            m_wait_flush_callback = callback;
            m_wait_flush_callback_state = state;
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
        bool is_dirty() const {
            return this->m_dirty_rects.size()!=0;
        }
    };
    template<uint16_t Width, uint16_t Height, typename PixelType, typename PaletteType = gfx::palette<PixelType,PixelType>>
    using screen = screen_ex<Width,Height, gfx::bitmap<PixelType,PaletteType>>;
}
#endif