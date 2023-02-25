#ifndef HTCW_UIX_SCREEN_HPP
#define HTCW_UIX_SCREEN_HPP
#include <uix_core.hpp>
#include <htcw_data.hpp>
#include <string.h>
namespace uix {

    template<uint16_t Width, uint16_t Height, typename PixelType, typename PaletteType = gfx::palette<PixelType,PixelType>>
    class screen {
    public:
        using type = screen;
        using pixel_type = PixelType;
        using palette_type = PaletteType;
        using control_type = control<pixel_type,palette_type>;
        using bitmap_type = gfx::bitmap<pixel_type,palette_type>;
        using control_surface_type = typename control<pixel_type,palette_type>::control_surface_type;
        typedef void(*render_callback_type)(point16 location,gfx::bitmap<pixel_type,palette_type>& bmp,void* state);
    private:
        using vector_type = data::simple_vector<control_type*>;
        const palette_type* m_palette;
        pixel_type m_background_color;
        vector_type m_controls;
        uint8_t* m_render_buffer;
        bool m_valid;
        static bool is_already_contained(const data::simple_vector<rect16>& dirties,const rect16* in_out_rect) {
            for(const rect16* it = dirties.cbegin();it!=dirties.cend();++it) {
                if(it->contains(*in_out_rect)) {
                    in_out_rect = nullptr;
                    return true;
                } else if(in_out_rect->contains(*it)) {
                    return true;
                }
            }
            return false;
        }
        void do_move(screen& rhs) {
            m_palette = rhs.m_palette;
            m_background_color = rhs.m_background_color;
            m_controls = helpers::uix_move(rhs.m_controls);
            m_valid = rhs.m_valid;
        }
        screen(const screen& rhs)=delete;
        screen& operator=(const screen& rhs)=delete;
    public:
        screen(const palette_type* palette = nullptr) : m_palette(palette),m_background_color(pixel_type()),m_controls(),m_valid(false) {
            
        }
        screen(screen&& rhs) {
            do_move(rhs);
        }
        void invalidate() {
            m_valid = false;
        }
        bool valid() const { 
            return m_valid;
        }
        screen& operator=(screen&& rhs) {
            do_move(rhs);
            return *this;
        }
        pixel_type background_color() const {
            return m_background_color;
        }
        void background_color(pixel_type value) {
            m_background_color = value;
            invalidate();
        }
        const palette_type* palette() {
            return m_palette;
        }
        size16 dimensions() const {
            return {Width,Height};
        }
        rect16 bounds() const {
            return rect16(point16::zero(),dimensions());
        }
        uix_result deregister_controls() {
            bool should_invalidate = m_controls.size()==0;
            m_controls.clear();
            if(should_invalidate) {
                invalidate();
            }
            return uix_result::success;
        }
        uix_result register_control(control_type& control) {
            if(m_controls.push_back(&control)) {
                invalidate();
                return uix_result::success;
            }
            return uix_result::out_of_memory;
        }
        uix_result render(render_callback_type render_callback,void* state = nullptr, size_t max_memory = 32*1024, uint8_t* render_buffer = nullptr, void*(allocator)(size_t size)=::malloc,void(deallocator)(void*)=::free) {
            data::simple_vector<rect16> dirties;
            if(!m_valid) {
                // invalidate the entire screen
                dirties.push_back(bounds());
            } else {
                // go through all the controls
                for(control_type** it = m_controls.begin();it!=m_controls.end();++it) {
                    // it is double indirected - make it easier to use
                    const control_type *pit = *it;
                    // only care about controls that must be redrawn
                    if(!pit->valid() && pit->visible()) {
                        rect16 r = (rect16)pit->bounds().crop((srect16)this->bounds());
                        rect16* pr = &r;
                        // do we already have a rectangle that contains this rectangle,
                        // or otherwise does this rectangle contain another rectangle
                        // in the list?
                        if(is_already_contained(dirties,pr)) {
                            if(pr!=nullptr) {
                                // r contains an existing rectangle
                                pr->x1 = r.x1;
                                pr->y1 = r.y1;
                                pr->x2 = r.x2;
                                pr->y2 = r.y2;
                            } 
                        } else {
                            // add the main rect to the list
                            dirties.push_back(r);
                        }
                    }
                }
            }
            if(dirties.size()==0) {
                // nothing more to do
                return uix_result::success;
            }
            uint32_t max_area = 0;
            rect16 largest_rect;
            // find the largest rect
            for(const rect16* it = dirties.cbegin();it!=dirties.cend();++it) {
                uint32_t area = it->width()*it->height();
                if(area>max_area) {
                    max_area = area;
                    largest_rect = *it;
                }
            }
            // total target size of bitmap in bytes - start with the same size as the largest_rect
            size_t bmp_size = bitmap_type::sizeof_buffer(largest_rect.dimensions());
            // total size in bytes of one line, padded to byte boundaries
            size_t bmp_stride_size = bitmap_type::sizeof_buffer({largest_rect.width(),1});
            // max memory must hold at least a single line
            if(max_memory<bmp_stride_size) {
                return uix_result::invalid_argument;
            }
            // if the size is too big for max_memory, take the biggest size we can.
            if(bmp_size>max_memory) {
                bmp_size = bitmap_type::sizeof_buffer({largest_rect.width(),uint16_t(max_memory/bmp_stride_size)});
            }
            // our bitmap buffer can be passed on, or otherwise it will be allocated here
            // bmp_data holds the pointer either way.
            uint8_t* bmp_data = m_render_buffer!=nullptr?m_render_buffer:(uint8_t*)allocator(bmp_size);
            // the following only happens if allocator above failed:
            // if we can't allocate it all keep halving the amount to try
            // until we succeed - as long as the size is >= one line
            while(bmp_data==nullptr && bmp_size>bmp_stride_size) {
                bmp_size/=2;
                bmp_data = (uint8_t*)allocator(bmp_size);
            }
            // if we still couldn't allocate return out of memory
            if(bmp_data==nullptr) {
                return uix_result::out_of_memory;
            }
            // for each dirty rectangle 
            for(const rect16* it = dirties.cbegin();it!=dirties.cend();++it) {
                rect16 r = *it;
                // get the size of the dirty rect in bytes
                size_t r_size = bitmap_type::sizeof_buffer(r.dimensions());
                // compute how many lines of it we can hold at once
                size_t lines = bmp_size/bitmap_type::sizeof_buffer({r.width(),1});
                // wrap bmp_data with a bitmap_type instance
                // and init with screen color
                bitmap_type bmp({r.width(),(uint16_t)lines},bmp_data);
                bmp.fill(bmp.bounds(),m_background_color);
                int h = (int)r.height();
                // do for each set of lines
                for(int y = 0;y<h;y+=lines) {
                    // compute the subrectangle of this dirty rectangle
                    // this is what we render to (physical coordinates)
                    srect16 sr(r.x1,r.y1+y,r.x2,r.y1+lines+y-1);
                    bool found = false;
                    // for each control
                    for(control_type** cit = m_controls.begin();cit!=m_controls.end();++cit) {
                        control_type* pcit = *cit;
                        srect16 r_offset = pcit->bounds();
                        // does this control lie within the dirty rect?
                        if(pcit->visible() && r_offset.intersects(sr)) {
                            // found a control
                            found = true;
                            // crop our working rect to the subrect
                            //r_offset = r_offset.crop(sr);
                            // now offset it by the subrect to set the logical coordinates
                            // of the upper left of the control surface to (0,0)
                            r_offset.offset_inplace(-sr.x1,-sr.y1);
                            // further modify the rect so as to make sure
                            // that when y gets incremented we shift the whole thing up
                            // such that we're rendering over and over as we move through
                            // successive segments of the dirty rect.
                            //r_offset.offset_inplace(0,-y);
                            Serial.printf("surface: (%d,%d)-(%d,%d)\n",(int)r_offset.x1,(int)r_offset.y1,(int)r_offset.x2,(int)r_offset.y2);
                            // establish the bitmap backed control surface at the proper logical coordinates
                            control_surface_type surface(bmp,r_offset);
                            // render the control
                            pcit->on_render(surface);
                        }
                    }
                    Serial.printf("render at (%d,%d)\n",(int)sr.x1,(int)sr.y1);
                    // send the bitmap the display
                    render_callback(point16(sr.x1,sr.y1),bmp,state);
                    
                    // erase the background for the next iteration
                    if(y<h && found) {
                        bmp.fill(bmp.bounds(),m_background_color);
                    }
                }
            }
            if(m_render_buffer!=nullptr) {
                deallocator(bmp_data);
            }
            m_valid = true;
            return uix_result::success;
        }
    };
}

#endif