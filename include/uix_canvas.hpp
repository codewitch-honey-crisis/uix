#ifndef HTCW_UIX_CANVAS_HPP
#define HTCW_UIX_CANVAS_HPP
#include "uix_core.hpp"
namespace uix {
    template<typename PixelType,typename PaletteType = gfx::palette<PixelType,PixelType>> class canvas : public control<PixelType,PaletteType> {
    public:
        using type = canvas;
        using base_type = control<PixelType, PaletteType>;
        using pixel_type = PixelType;
        using palette_type = PaletteType;
        using control_surface_type = typename base_type::control_surface_type;
        typedef void(*on_paint_callback_type)(control_surface_type& destination,const srect16& clip,void* state);
    private:
        on_paint_callback_type m_on_paint_cb;
        void* m_on_paint_cb_state;
        canvas(const canvas& rhs)=delete;
        canvas& operator=(const canvas& rhs)=delete;
        void do_move(canvas& rhs) {
            do_move_control(rhs);
            m_on_paint_cb = rhs.m_on_paint_cb;
            rhs.m_on_paint_cb = nullptr;
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
        
        canvas(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent,palette),m_on_paint_cb(nullptr) {
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