#ifndef HTCW_UIX_CANVAS_HPP
#define HTCW_UIX_CANVAS_HPP
#include "uix_core.hpp"
namespace uix {
    template<typename ControlSurfaceType> class canvas : public control<ControlSurfaceType> {
    public:
        using type = canvas;
        using base_type = control<ControlSurfaceType>;
        using pixel_type = typename ControlSurfaceType::pixel_type;
        using palette_type = typename ControlSurfaceType::palette_type;
        using control_surface_type = ControlSurfaceType;
        typedef void(*on_paint_callback_type)(control_surface_type& destination,const srect16& clip,void* state);
    private:
        on_paint_callback_type m_on_paint_cb;
        void* m_on_paint_cb_state;
        void do_move(canvas& rhs) {
            this->do_move_control(rhs);
            m_on_paint_cb = rhs.m_on_paint_cb;
            rhs.m_on_paint_cb = nullptr;
            m_on_paint_cb_state = rhs.m_on_paint_cb_state;
        }
        void do_copy(const canvas& rhs) {
            this->do_copy_control(rhs);
            m_on_paint_cb = rhs.m_on_paint_cb;
            m_on_paint_cb_state = rhs.m_on_paint_cb_state;
        }
        
    public:
        canvas(canvas&& rhs) {
            do_move(rhs);
        }
        canvas& operator=(canvas&& rhs) {
            do_move(rhs);
            return *this;
        }
        canvas(const canvas& rhs) {
            do_copy(rhs);
        }
        canvas& operator=(const canvas& rhs) {
            do_copy(rhs);
            return *this;
        }
        canvas(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent,palette),m_on_paint_cb(nullptr),m_on_paint_cb_state(nullptr) {
        }
        virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
            if(m_on_paint_cb!=nullptr) {
                m_on_paint_cb(destination,clip,m_on_paint_cb_state);
            }
        }
        void on_paint(on_paint_callback_type callback, void* state = nullptr) {
            m_on_paint_cb_state = state;
            m_on_paint_cb = callback;
        }
        
    };
}
#endif