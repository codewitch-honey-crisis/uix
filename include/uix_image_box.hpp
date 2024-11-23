#ifndef HTCW_UIX_IMAGE_BOX_HPP
#define HTCW_UIX_IMAGE_BOX_HPP
#include "uix_core.hpp"
namespace uix {
    /// @brief Represents an image box control
    /// @tparam ControlSurfaceType 
    template<typename ControlSurfaceType> class image_box : public control<ControlSurfaceType> {
    public:
        using type = image_box;
        using base_type = control<ControlSurfaceType>;
        using pixel_type = typename ControlSurfaceType::pixel_type;
        using palette_type = typename ControlSurfaceType::palette_type;
        using control_surface_type = ControlSurfaceType;
        typedef void(*callback_type)(void* state);
    private:
        gfx::image* m_image;
        callback_type m_on_load_cb;
        void* m_on_load_cb_state;
        callback_type m_on_unload_cb;
        void* m_on_unload_cb_state;
        void* (*m_allocator)(size_t);
        void (*m_deallocator)(void*);
        uint8_t* m_render_cache;
    protected:
        void do_move_control(image_box& rhs) {
            this->base_type::do_move_control(rhs);
            m_image=rhs.m_image;
            rhs.m_image = nullptr;
            m_on_load_cb = rhs.m_on_load_cb;
            rhs.m_on_load_cb = nullptr;
            m_on_load_cb_state = rhs.m_on_load_cb_state;
            m_on_unload_cb = rhs.m_on_unload_cb;
            rhs.m_on_unload_cb = nullptr;
            m_on_unload_cb_state = rhs.m_on_unload_cb_state;
            m_allocator = rhs.m_allocator;
            m_deallocator = rhs.m_deallocator;
            m_render_cache = rhs.m_render_cache;
            rhs.m_render_cache = nullptr;
        }
        void do_copy_control(const image_box& rhs) {
            this->base_type::do_copy_control(rhs);
            m_image=rhs.m_image;
            m_on_load_cb = rhs.m_on_load_cb;
            m_on_load_cb_state = rhs.m_on_unload_cb_state;
            m_on_unload_cb = rhs.m_on_unload_cb;
            m_on_unload_cb_state = rhs.m_on_unload_cb_state;
            m_allocator = rhs.m_allocator;
            m_deallocator = rhs.m_deallocator;
            m_render_cache = nullptr;
        }
    public:
        /// @brief Moves an image_box
        /// @param rhs The image_box to move
        image_box(image_box&& rhs) {
            do_move_control(rhs);
        }
        /// @brief Moves an image_box
        /// @param rhs The image_box to move
        /// @return This
        image_box& operator=(image_box&& rhs) {
            do_move_control(rhs);
            return *this;
        }
        /// @brief Copies an image_box
        /// @param rhs The image_box to copy
        image_box(const image_box& rhs) {
            do_copy_control(rhs);
        }
        /// @brief Copies an image_box
        /// @param rhs The image_box to copy
        /// @return this
        image_box& operator=(const image_box& rhs) {
            do_copy_control(rhs);
            return *this;
        }
        /// @brief Constructs an image_box with the given parent and optional palette
        /// @param parent The parent - usually a screen
        /// @param palette The associated palette, usually from the screen
        image_box(invalidation_tracker& parent, const palette_type* palette = nullptr,void*(allocator)(size_t) = ::malloc,void(deallocator)(void*) = ::free) : base_type(parent,palette),m_image(nullptr), m_on_load_cb(nullptr),m_on_load_cb_state(nullptr),m_allocator(allocator),m_deallocator(deallocator) {
        }
        /// @brief Constructs an image_box with the given parent and optional palette
        image_box(void*(allocator)(size_t) = ::malloc,void(deallocator)(void*) = ::free) : base_type(),m_image(nullptr), m_on_load_cb(nullptr),m_on_load_cb_state(nullptr),m_allocator(allocator),m_deallocator(deallocator) {
        }
        /// @brief Indicates the image to display
        /// @return A pointer to the image
        gfx::image* image() const {
            return m_image;
        }
        /// @brief Sets the image to displate
        /// @param image The image to set
        void image(gfx::image& image, bool invalidate = true) {
            if(invalidate && &image!=m_image) {
                this->invalidate();
            }
            m_image = &image;
        }
        /// @brief Called once before the control is first rendered during update()
        virtual void on_before_paint() override {
            if(m_on_load_cb!=nullptr) {
                m_on_load_cb(m_on_load_cb_state);
            }
            if(m_image!=nullptr) {
                m_image->initialize();
                if(m_allocator!=nullptr && m_deallocator!=nullptr) {
                    using bmp_t = gfx::bitmap<typename control_surface_type::pixel_type,typename control_surface_type::palette_type>;
                    m_render_cache = nullptr;//(uint8_t*)m_allocator(bmp_t::sizeof_buffer((size16)this->dimensions()));
                    if(m_render_cache!=nullptr) {
                        bmp_t bmp((size16)this->dimensions(),m_render_cache,this->palette());
                        gfx::draw::image(bmp,bmp.bounds(),*m_image,bmp.bounds());
                        if(m_on_unload_cb!=nullptr) {
                            m_on_unload_cb(m_on_unload_cb_state);
                        }
                    }
                }
            }
        }
        /// @brief Called once after the control is last rendered during update()
        virtual void on_after_paint() override {
            if(m_render_cache!=nullptr) {
                m_deallocator(m_render_cache);
            }
        }
        /// @brief Called when the image_box is painted
        /// @param destination The destination to paint to
        /// @param clip The clipping rectangle
        virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
            // if(m_render_cache!=nullptr) {
            //     using bmp_t = gfx::bitmap<typename control_surface_type::pixel_type,typename control_surface_type::palette_type>;
            //     bmp_t bmp((size16)this->dimensions(),m_render_cache,this->palette());
            //     gfx::draw::bitmap(destination,destination.bounds(),bmp,bmp.bounds());
            // } else {
                if(m_on_load_cb!=nullptr) {
                    m_on_load_cb(m_on_load_cb_state);
                }
                if(m_image!=nullptr) {
                    gfx::draw::image(destination,destination.bounds(),*m_image);
                } 
                if(m_on_unload_cb!=nullptr) {
                    m_on_unload_cb(m_on_unload_cb_state);
                }
            //}
            
        }
        /// @brief Returns the on_load_callback handler
        /// @return A pointer to the on_load callback
        callback_type on_load_callback() {
            return m_on_load_cb;
        }
        /// @brief Returns the user defined on_load_callback handler state
        /// @return A pointer to the on_load callback state
        void* on_load_callback_state() {
            return m_on_load_cb_state;
        }
        /// @brief Sets a callback which is called before the image_box is loaded. Often used to fetch the image_box from the source
        /// @param callback The callback
        /// @param state A user defined state passed to the callback
        void on_load_callback(callback_type callback, void* state = nullptr) {
            m_on_load_cb_state = state;
            m_on_load_cb = callback;
            this->invalidate();
        }
        /// @brief Returns the on_unload_callback handler
        /// @return A pointer to the on_unload callback
        callback_type on_unload_callback() {
            return m_on_unload_cb;
        }
        /// @brief Returns the user defined on_unload_callback handler state
        /// @return A pointer to the on_unload callback state
        void* on_unload_callback_state() {
            return m_on_unload_cb_state;
        }
        /// @brief Sets a callback which is called after the image_box is done being used. Often used to close the source
        /// @param callback The callback
        /// @param state A user defined state passed to the callback
        void on_unload_callback(callback_type callback, void* state = nullptr) {
            m_on_unload_cb_state = state;
            m_on_unload_cb = callback;
        }
    };
}
#endif