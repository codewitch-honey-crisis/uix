#ifndef HTCW_UIX_IMAGE_HPP
#define HTCW_UIX_IMAGE_HPP
#include "uix_core.hpp"
namespace uix {
    /// @brief Represents an image
    /// @tparam ControlSurfaceType 
    template<typename ControlSurfaceType> class image : public control<ControlSurfaceType> {
    public:
        using type = image;
        using base_type = control<ControlSurfaceType>;
        using pixel_type = typename ControlSurfaceType::pixel_type;
        using palette_type = typename ControlSurfaceType::palette_type;
        using control_surface_type = ControlSurfaceType;
        typedef void(*callback_type)(void* state);
    private:
        io::stream* m_stream;
        bool m_reset_stream;
        callback_type m_on_load_cb;
        void* m_on_load_cb_state;
        callback_type m_on_unload_cb;
        void* m_on_unload_cb_state;
        void* (*m_allocator)(size_t);
        void (*m_deallocator)(void*);
        uint8_t* m_render_cache;
    protected:
        void do_move_control(image& rhs) {
            this->base_type::do_move_control(rhs);
            m_stream = rhs.m_stream;
            m_reset_stream = rhs.m_reset_stream;
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
        void do_copy_control(const image& rhs) {
            this->base_type::do_copy_control(rhs);
            m_stream = rhs.m_stream;
            m_reset_stream = rhs.m_reset_stream;
            m_on_load_cb = rhs.m_on_load_cb;
            m_on_load_cb_state = rhs.m_on_unload_cb_state;
            m_on_unload_cb = rhs.m_on_unload_cb;
            m_on_unload_cb_state = rhs.m_on_unload_cb_state;
            m_allocator = rhs.m_allocator;
            m_deallocator = rhs.m_deallocator;
            m_render_cache = nullptr;
        }
    public:
        /// @brief Moves an image
        /// @param rhs The image to move
        image(image&& rhs) {
            do_move_control(rhs);
        }
        /// @brief Moves an image
        /// @param rhs The image to move
        /// @return This
        image& operator=(image&& rhs) {
            do_move_control(rhs);
            return *this;
        }
        /// @brief Copies an image
        /// @param rhs The image to copy
        image(const image& rhs) {
            do_copy_control(rhs);
        }
        /// @brief Copies an image
        /// @param rhs The image to copy
        /// @return this
        image& operator=(const image& rhs) {
            do_copy_control(rhs);
            return *this;
        }
        /// @brief Constructs an image with the given parent and optional palette
        /// @param parent The parent - usually a screen
        /// @param palette The associated palette, usually from the screen
        image(invalidation_tracker& parent, const palette_type* palette = nullptr,void*(allocator)(size_t) = ::malloc,void(deallocator)(void*) = ::free) : base_type(parent,palette),m_stream(nullptr), m_reset_stream(true), m_on_load_cb(nullptr),m_on_load_cb_state(nullptr),m_allocator(allocator),m_deallocator(deallocator) {
        }
        /// @brief Constructs an image with the given parent and optional palette
        image(void*(allocator)(size_t) = ::malloc,void(deallocator)(void*) = ::free) : base_type(),m_stream(nullptr), m_reset_stream(true), m_on_load_cb(nullptr),m_on_load_cb_state(nullptr),m_allocator(allocator),m_deallocator(deallocator) {
        }
        /// @brief Indicates the stream that contains the image
        /// @return A pointer to the stream
        io::stream* stream() {
            return m_stream;
        }
        /// @brief Sets the stream that contains the image
        /// @param stream The stream to set
        void stream(io::stream* stream, bool invalidate = true) {
            if(invalidate && m_stream!=stream) {
                this->invalidate();
            }
            m_stream = stream;
        }
        /// @brief Indicates whether the stream is automatically seeked to the start before the image is read from it
        /// @return True if the stream will be reset, otherwise false
        bool reset_stream() const {
            return m_reset_stream;
        }
        /// @brief Sets whether or not the stream is seeked to the start before the image is read from it
        /// @param value True if the stream should be reset, otherwise false
        void reset_stream(bool value) {
            m_reset_stream = value;
        }
        /// @brief Called once before the control is first rendered during update()
        virtual void on_before_paint() override {
            if(m_on_load_cb!=nullptr) {
                m_on_load_cb(m_on_load_cb_state);
            }
            
            if(m_stream!=nullptr && m_allocator!=nullptr && m_deallocator!=nullptr) {
                using bmp_t = gfx::bitmap<typename control_surface_type::pixel_type,typename control_surface_type::palette_type>;
                m_render_cache = (uint8_t*)m_allocator(bmp_t::sizeof_buffer((size16)this->dimensions()));
                if(m_render_cache!=nullptr) {
                    bmp_t bmp((size16)this->dimensions(),m_render_cache,this->palette());
                    if(m_reset_stream) {
                        m_stream->seek(0);         
                    }
                    gfx::draw::image(bmp,bmp.bounds(),m_stream,bmp.bounds());
                    if(m_on_unload_cb!=nullptr) {
                        m_on_unload_cb(m_on_unload_cb_state);
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
        /// @brief Called when the image is painted
        /// @param destination The destination to paint to
        /// @param clip The clipping rectangle
        virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
            if(m_render_cache!=nullptr) {
                using bmp_t = gfx::bitmap<typename control_surface_type::pixel_type,typename control_surface_type::palette_type>;
                bmp_t bmp((size16)this->dimensions(),m_render_cache,this->palette());
                gfx::draw::bitmap(destination,destination.bounds(),bmp,bmp.bounds());
            } else {
                if(m_reset_stream && m_stream!=nullptr && m_stream->caps().seek) {
                    m_stream->seek(0);
                }
                if(m_on_load_cb!=nullptr) {
                    m_on_load_cb(m_on_load_cb_state);
                }
                if(m_stream!=nullptr) {
                    gfx::gfx_result res = gfx::draw::image(destination,destination.bounds(),m_stream);
                } 
                if(m_on_unload_cb!=nullptr) {
                    m_on_unload_cb(m_on_unload_cb_state);
                }
            }
            
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
        /// @brief Sets a callback which is called before the image is loaded. Often used to fetch the image from the source
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
        /// @brief Sets a callback which is called after the image is done being used. Often used to close the source
        /// @param callback The callback
        /// @param state A user defined state passed to the callback
        void on_unload_callback(callback_type callback, void* state = nullptr) {
            m_on_unload_cb_state = state;
            m_on_unload_cb = callback;
        }
    };
}
#endif