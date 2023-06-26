#ifndef HTCW_UIX_CORE_HPP
#define HTCW_UIX_CORE_HPP
#include <gfx_core.hpp>
#include <gfx_pixel.hpp>
#include <gfx_palette.hpp>
#include <gfx_positioning.hpp>
#include <gfx_bitmap.hpp>
#include <gfx_font.hpp>
#include <gfx_open_font.hpp>
#include <gfx_viewport.hpp>
namespace uix {
    using point16 = gfx::point16;
    using spoint16 = gfx::spoint16;
    using rect16 = gfx::rect16;
    using srect16 = gfx::srect16;
    using size16 = gfx::size16;
    using ssize16 = gfx::ssize16;
    enum struct uix_result {
        success = 0,
        canceled,
        invalid_argument,
        not_supported,
        io_error,
        device_error,
        out_of_memory,
        invalid_format,
        no_palette,
        invalid_state,
        unknown_error
    };
    enum struct uix_justify {
        top_left = 0,
        top_middle = 1,
        top_right = 2,
        center_left = 3,
        center = 4,
        center_right = 5,
        bottom_left = 6,
        bottom_middle = 7,
        bottom_right = 8
    };
    namespace helpers {
        // implement std::move to limit dependencies on the STL, which may not be there
        template< class T > struct uix_remove_reference      { typedef T type; };
        template< class T > struct uix_remove_reference<T&>  { typedef T type; };
        template< class T > struct uix_remove_reference<T&&> { typedef T type; };
        template <typename T>
        typename uix_remove_reference<T>::type&& uix_move(T&& arg) {
            return static_cast<typename uix_remove_reference<T>::type&&>(arg);
        }
    }
    template<typename BitmapType>
    class control_surface final {
    public:
        using type = control_surface;
        using pixel_type = typename BitmapType::pixel_type;
        using palette_type = typename BitmapType::palette_type;
        using bitmap_type= BitmapType;
        using caps = gfx::gfx_caps<false,false,false,false,false,true,false>;
    private:
        bitmap_type& m_bitmap;
        srect16 m_rect;
        void do_move(control_surface& rhs) {
            m_bitmap = rhs.m_bitmap;
            m_rect = rhs.m_rect;
        }
        void do_copy(control_surface& rhs) {
            m_bitmap = rhs.m_bitmap;
            m_rect = rhs.m_rect;
        }

    public:
        control_surface(control_surface&& rhs) : m_bitmap(rhs.m_bitmap) {
            do_move(rhs);
        }
        control_surface& operator=(control_surface&& rhs) {
            do_move(rhs);
            return *this;
        }
        control_surface(const control_surface& rhs) {
            do_copy(rhs);
        }
        control_surface& operator=(const control_surface& rhs) {
            do_copy(rhs);
            return *this;
        }
        control_surface(bitmap_type& bmp,const srect16& rect) : m_bitmap(bmp) {
            m_rect = rect;
        }
        const palette_type* palette() const {
            return m_bitmap.palette();
        }
        size16 dimensions() const {
            return (size16)m_rect.dimensions();
        }
        rect16 bounds() const {
            return rect16(point16::zero(),dimensions());
        }
        gfx::gfx_result point(point16 location, pixel_type* out_pixel) const {
            location.offset_inplace(m_rect.x1,m_rect.y1);
            return m_bitmap.point(location,out_pixel);
        }
        gfx::gfx_result point(point16 location, pixel_type pixel) {
            spoint16 loc = ((spoint16)location).offset(m_rect.x1,m_rect.y1);
            return m_bitmap.point((point16)loc,pixel);
            return gfx::gfx_result::success;
        }
        gfx::gfx_result fill(const rect16& bounds, pixel_type pixel) {
            if(bounds.intersects(this->dimensions().bounds())) {
                srect16 b = ((srect16)bounds);
                b=b.offset(m_rect.x1,m_rect.y1);
                if(b.intersects((srect16)m_bitmap.bounds())) {
                    b=b.crop((srect16)m_bitmap.bounds());
                    return m_bitmap.fill((rect16)b,pixel);
                }
            }
            return gfx::gfx_result::success;
        }
        gfx::gfx_result clear(const rect16& bounds) {
            return fill(bounds,pixel_type());
        }
    };
    class invalidation_tracker {
    public:
        virtual uix_result invalidate(const srect16& rect)=0;
        virtual uix_result validate_all()=0;
    };
    template<typename ControlSurfaceType>
    class control {
    public:
        using type = control;
        using pixel_type = typename ControlSurfaceType::pixel_type;
        using palette_type = typename ControlSurfaceType::palette_type;
        using control_surface_type = ControlSurfaceType;
    private:        
        srect16 m_bounds;
        const palette_type* m_palette;
        bool m_visible;
        invalidation_tracker* m_parent;
    protected:
        control() : m_bounds({0,0,49,24}),m_palette(nullptr),m_visible(true),m_parent(nullptr) {
        }
        control(invalidation_tracker& parent, const palette_type* palette = nullptr) : m_bounds({0,0,49,24}),m_palette(palette),m_visible(true),m_parent(&parent) {
            
        }
        void do_copy_control(const control& rhs) {
            m_bounds = rhs.m_bounds;
            m_palette = rhs.m_palette;
            m_visible = rhs.m_visible;
            m_parent = rhs.m_parent;
        }
        void do_move_control(control& rhs) {
            m_bounds = rhs.m_bounds;
            m_palette = rhs.m_palette;
            m_visible = rhs.m_visible;
            m_parent = rhs.m_parent;
        }
    public:
        control(control&& rhs) {
            do_move_control(rhs);
        }
        control& operator=(control&& rhs) {
            do_move_control(rhs);
            return *this;
        }
        control(const control& rhs) {
            do_copy_control(rhs);
        }
        control& operator=(const control& rhs) {
            do_copy_control(rhs);
            return *this;
        }
        const palette_type* palette() const {return m_palette;}
        ssize16 dimensions() const {
            return m_bounds.dimensions();
        }
        srect16 bounds() const {
            return m_bounds;
        }
        virtual void bounds(const srect16& value) {
            if(m_visible) {
                if(m_parent!=nullptr) {
                    m_parent->invalidate(m_bounds);
                    m_parent->invalidate(value);
                }
            }
            m_bounds = value;
        }
        virtual void on_paint(control_surface_type& destination,const srect16& clip) {
        }
        virtual bool on_touch(size_t locations_size,const spoint16* locations) {
            return false;
        };
        virtual void on_release() {
        };
        bool visible() const {
            return m_visible;
        }
        void visible(bool value) {
            if(value!=m_visible) {
                m_visible = value;
                this->invalidate();
            }
        }
        invalidation_tracker& parent() {
            return *m_parent;
        }
        void parent(invalidation_tracker& parent) {
            m_parent=&parent;
        }
        uix_result invalidate() {
            if(m_parent==nullptr) {
                return uix_result::invalid_state;
            }
            return m_parent->invalidate(m_bounds);
        }
        uix_result invalidate(const srect16& bounds) {
            if(m_parent==nullptr) {
                return uix_result::invalid_state;
            }
            srect16 b = bounds.offset(this->bounds().location());
            if(b.intersects(this->bounds())) {
                b=b.crop(this->bounds());
                return m_parent->invalidate(b);
            }
            return uix_result::success;
        }
    };
}
#endif