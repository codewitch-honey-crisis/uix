#ifndef HTCW_UIX_CORE_HPP
#define HTCW_UIX_CORE_HPP
#include <gfx_core.hpp>
#include <gfx_encoding.hpp>
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
    using pointf = gfx::pointf;
    using rect16 = gfx::rect16;
    using srect16 = gfx::srect16;
    using rectf = gfx::rectf;
    using size16 = gfx::size16;
    using ssize16 = gfx::ssize16;
    using sizef = gfx::sizef;
    using path16 = gfx::path16;
    using spath16 = gfx::spath16;
    using pathf = gfx::pathf;
    using uix_encoding = gfx::gfx_encoding;
    /// @brief Indicates an error or success result
    enum struct uix_result {
        /// @brief The operation completed successfully
        success = 0,
        /// @brief The operation was canceled
        canceled,
        /// @brief One or more arguments is invalid
        invalid_argument,
        /// @brief The operation is not supported
        not_supported,
        /// @brief There was an I/O error during the operation
        io_error,
        /// @brief The underlying device returned an error
        device_error,
        /// @brief Not enough memory was available to complete the operation
        out_of_memory,
        /// @brief The format of the input was invalid
        invalid_format,
        /// @brief An indexed color operation was attempted without providing a palette.
        no_palette,
        /// @brief The operation cannot be completed because the containing facility is in an invalid state
        invalid_state,
        /// @brief An error occured, but the type of error could not be determined
        unknown_error
    };
    /// @brief Indicates the justification of a display element
    enum struct uix_justify {
        /// @brief The content is aligned to the upper left
        top_left = 0,
        /// @brief The content is aligned to the upper middle
        top_middle = 1,
        /// @brief The content is aligned to the upper right
        top_right = 2,
        /// @brief The content is vertically centered and to the left
        center_left = 3,
        /// @brief The content is centered vertically and horizontally
        center = 4,
        /// @brief The content is vertically centered and to the right
        center_right = 5,
        /// @brief The content is aligned to the lower left
        bottom_left = 6,
        /// @brief The content is aligned to the bottom and horizontally centered
        bottom_middle = 7,
        /// @brief The content is aligned to the lower right
        bottom_right = 8
    };
    /// @brief The orientation of the control
    enum struct uix_orientation {
        horizontal = 0,
        vertical = 1
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
    /// @brief Represents a surface on which drawing takes place. Control surfaces are translated to their physical screen coordinates.
    /// @tparam BitmapType The type of draw target that backs the control surface - usually a standard in memory bitmap<> 
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
        /// @brief Moves the control surface
        /// @param rhs The control surface to move
        control_surface(control_surface&& rhs) : m_bitmap(rhs.m_bitmap) {
            do_move_control(rhs);
        }
        /// @brief Moves the control surface
        /// @param rhs The control surface to move
        /// @return this
        control_surface& operator=(control_surface&& rhs) {
            do_move_control(rhs);
            return *this;
        }
        /// @brief Copies the control surface
        /// @param rhs The control surface to copy
        control_surface(const control_surface& rhs) {
            do_copy_control(rhs);
        }
        /// @brief Copies the control surface
        /// @param rhs The control surface to copy
        /// @return this
        control_surface& operator=(const control_surface& rhs) {
            do_copy_control(rhs);
            return *this;
        }
        /// @brief Constructs a control surface from a backing bitmap
        /// @param bmp The backing bitmap to use
        /// @param rect The location of the control surface within the bitmap
        control_surface(bitmap_type& bmp,const srect16& rect) : m_bitmap(bmp) {
            m_rect = rect;
        }
        /// @brief Retrieves the palette, if any
        /// @return The palette
        const palette_type* palette() const {
            return m_bitmap.palette();
        }
        /// @brief Reports the size of the draw surface
        /// @return a size16 with the width and height of the draw surface
        size16 dimensions() const {
            return (size16)m_rect.dimensions();
        }
        /// @brief Reports the bounds of the draw surface from (0,0)
        /// @return a rect16 anchored to (0,0) and extending to the size of the draw surface
        rect16 bounds() const {
            return rect16(point16::zero(),dimensions());
        }
        /// @brief Reports the color of the pixel at a location
        /// @param location The location to check
        /// @param out_pixel A pointer to the pixel data to fill
        /// @return The result of the operation
        gfx::gfx_result point(point16 location, pixel_type* out_pixel) const {
            location.offset_inplace(m_rect.x1,m_rect.y1);
            return m_bitmap.point(location,out_pixel);
        }
        /// @brief Sets the color of a pixel at a location
        /// @param location The location
        /// @param pixel The new color
        /// @return The result of the operation
        gfx::gfx_result point(point16 location, pixel_type pixel) {
            spoint16 loc = ((spoint16)location).offset(m_rect.x1,m_rect.y1);
            return m_bitmap.point((point16)loc,pixel);
            return gfx::gfx_result::success;
        }
        /// @brief Fills a rectangular region with a color
        /// @param bounds The rect16 coordinates to fill
        /// @param pixel The new color
        /// @return The result of the operation
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
        /// @brief Clears a rectangular region, setting it to the default color
        /// @param bounds The rect16 coordinates to clear
        gfx::gfx_result clear(const rect16& bounds) {
            return fill(bounds,pixel_type());
        }
    };
    /// @brief Tracks dirty rectangles
    class invalidation_tracker {
    public:
        /// @brief Invalidate a rectangular region
        /// @param rect The region to invalidate
        /// @return The result of the operation
        virtual uix_result invalidate(const srect16& rect)=0;
        /// @brief Marks all dirty rectangles as clean
        /// @return The result of the operation
        virtual uix_result validate_all()=0;
    };
    /// @brief Represents the base type for all controls
    /// @tparam ControlSurfaceType The type of control_surface to use. Usually this comes from the screen<>.
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
        /// @brief Constructs an empty control instance
        control() : m_bounds({0,0,49,24}),m_palette(nullptr),m_visible(true),m_parent(nullptr) {
        }
        /// @brief Constructs a control given a parent and an optional palette
        /// @param parent The parent invalidation tracker - usually a screen
        /// @param palette The palette. Typically the screen's palette()
        control(invalidation_tracker& parent, const palette_type* palette = nullptr) : m_bounds({0,0,49,24}),m_palette(palette),m_visible(true),m_parent(&parent) {
            
        }
        /// @brief Copies a control into this instance
        /// @param rhs The control to copy
        void do_copy_control(const control& rhs) {
            m_bounds = rhs.m_bounds;
            m_palette = rhs.m_palette;
            m_visible = rhs.m_visible;
            m_parent = rhs.m_parent;
        }
        /// @brief Moves a control into this instance
        /// @param rhs The control to move
        void do_move_control(control& rhs) {
            m_bounds = rhs.m_bounds;
            m_palette = rhs.m_palette;
            m_visible = rhs.m_visible;
            m_parent = rhs.m_parent;
        }
    public:
        /// @brief Moves a control
        /// @param rhs The control to move
        control(control&& rhs) {
            do_move_control(rhs);
        }
        /// @brief Moves a control
        /// @param rhs The control to move
        /// @return this
        control& operator=(control&& rhs) {
            do_move_control(rhs);
            return *this;
        }
        /// @brief Copies a control
        /// @param rhs The control to copy
        control(const control& rhs) {
            do_copy_control(rhs);
        }
        /// @brief Copies a control
        /// @param rhs The control to copy
        /// @return this
        control& operator=(const control& rhs) {
            do_copy_control(rhs);
            return *this;
        }
        /// @brief Returns the associated palette if any.
        /// @return A pointer to the palette or null
        const palette_type* palette() const {return m_palette;}

        /// @brief Sets the associated palette.
        /// @param value the palette or null.
        /// @return A pointer to the palette or null
        void palette(const palette_type* value) { m_palette=value;}

        /// @brief Indicates the dimensions of the control, in pixels.
        /// @return a ssize16 indicating the width and height of the control
        ssize16 dimensions() const {
            return m_bounds.dimensions();
        }
        invalidation_tracker& parent() const {
            return *m_parent;
        }
        /// @brief Indicates the bounds of the control
        /// @return an srect16 indicating the location and size of the control
        srect16 bounds() const {
            return m_bounds;
        }
        /// @brief Sets the bounds of the control
        /// @param value An srect16 indicating the location and size
        virtual void bounds(const srect16& value) {
            if(m_visible) {
                if(m_parent!=nullptr) {
                    m_parent->invalidate(m_bounds);
                    m_parent->invalidate(value);
                }
            }
            m_bounds = value;
        }
        /// @brief Override to handle drawing the control
        /// @param destination The control_surface<> to draw to
        /// @param clip The clipping rectangle
        virtual void on_paint(control_surface_type& destination,const srect16& clip) {
        }
        /// @brief Override to handle when the control is touched
        /// @param locations_size The count of touch locations
        /// @param locations The locations, translated to local control coordinates
        /// @return True if touch was handled, or false if it wasn't.
        virtual bool on_touch(size_t locations_size,const spoint16* locations) {
            return false;
        }
        /// @brief Override to handle when the touched control has been released
        virtual void on_release() {
        }
        /// @brief Called once before the control is first rendered during update()
        virtual void on_before_render() {
        }
        /// @brief Called once after the control is last rendered during update()
        virtual void on_after_render() {
        }
        /// @brief Indicates whether the control is shown
        /// @return True if visible, otherwise false
        bool visible() const {
            return m_visible;
        }
        /// @brief Sets a value determining if the control is shown
        /// @param value True if visible, otherwise false
        void visible(bool value) {
            if(value!=m_visible) {
                m_visible = value;
                this->invalidate();
            }
        }
        /// @brief Indicates the parent of the control
        /// @return A reference to the parent
        invalidation_tracker& parent() {
            return *m_parent;
        }
        /// @brief Sets the parent of the control
        /// @param parent The parent
        void parent(invalidation_tracker& parent) {
            m_parent=&parent;
        }
        /// @brief Invalidates the control
        /// @return The result of the operation
        uix_result invalidate() {
            if(m_parent==nullptr) {
                return uix_result::invalid_state;
            }
            return m_parent->invalidate(m_bounds);
        }
        /// @brief Invalidates a rect within the control
        /// @param bounds An srect16 to invalidate in control local coordinates
        /// @return The result of the operation
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