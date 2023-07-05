#ifndef HTCW_UIX_IMAGE_HPP
#define HTCW_UIX_IMAGE_HPP
#include "uix_core.hpp"
namespace uix {
    template<typename ControlSurfaceType> class image : public control<ControlSurfaceType> {
    public:
        using type = image;
        using base_type = control<ControlSurfaceType>;
        using pixel_type = typename ControlSurfaceType::pixel_type;
        using palette_type = typename ControlSurfaceType::palette_type;
        using control_surface_type = ControlSurfaceType;
        typedef void(*on_load_callback_type)(void* state);
    private:
        io::stream* m_stream;
        on_load_callback_type m_on_load_cb;
        void* m_on_load_cb_state;
        void do_move(image& rhs) {
            this->do_move_control(rhs);
            m_stream = rhs.m_stream;
            m_on_load_cb = rhs.m_on_load_cb;
            rhs.m_on_load_cb = nullptr;
            m_on_load_cb_state = rhs.m_on_load_cb_state;
        }
        void do_copy(const image& rhs) {
            this->do_copy_control(rhs);
            m_stream = rhs.m_stream;
            m_on_load_cb = rhs.m_on_load_cb;
            m_on_load_cb_state = rhs.m_on_load_cb_state;
        }
    public:
        image(image&& rhs) {
            do_move(rhs);
        }
        image& operator=(image&& rhs) {
            do_move(rhs);
            return *this;
        }
        image(const image& rhs) {
            do_copy(rhs);
        }
        image& operator=(const image& rhs) {
            do_copy(rhs);
            return *this;
        }
        image(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent,palette),m_stream(nullptr), m_on_load_cb(nullptr),m_on_load_cb_state(nullptr) {
        }
        io::stream* stream() {
            return m_stream;
        }
        void stream(io::stream* stream) {
            if(m_stream!=stream) {
                m_stream = stream;
                this->invalidate();
            }
        }
        virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
            if(m_on_load_cb!=nullptr) {
                m_on_load_cb(m_on_load_cb_state);
            }
            if(m_stream!=nullptr) {
                gfx::draw::image(destination,destination.bounds(),m_stream);
            }
        }
        void on_load(on_load_callback_type callback, void* state = nullptr) {
            m_on_load_cb_state = state;
            m_on_load_cb = callback;
            this->invalidate();
        }
        
    };
}
#endif