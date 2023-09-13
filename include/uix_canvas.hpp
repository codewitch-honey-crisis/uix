#ifndef HTCW_UIX_CANVAS_HPP
#define HTCW_UIX_CANVAS_HPP
#include "uix_core.hpp"
namespace uix {
    /// @brief A general purpose drawing canvas
    /// @tparam ControlSurfaceType The type of control surface - usually the screen
    template<typename ControlSurfaceType> class canvas : public control<ControlSurfaceType> {
    public:
        using type = canvas;
        using base_type = control<ControlSurfaceType>;
        using pixel_type = typename ControlSurfaceType::pixel_type;
        using palette_type = typename ControlSurfaceType::palette_type;
        using control_surface_type = ControlSurfaceType;
        /// @brief The callback type for paint operations
        typedef void(*on_paint_callback_type)(control_surface_type& destination,const srect16& clip,void* state);
    private:
        on_paint_callback_type m_on_paint_cb;
        void* m_on_paint_cb_state;
    protected:
        /// @brief For derivative classes, moves the control
        /// @param rhs The canvas to move
        void do_move_control(canvas& rhs) {
            this->base_type::do_move_control(rhs);
            m_on_paint_cb = rhs.m_on_paint_cb;
            rhs.m_on_paint_cb = nullptr;
            m_on_paint_cb_state = rhs.m_on_paint_cb_state;
        }
        /// @brief For derivative classes, copies the control
        /// @param rhs The canvas to copy
        void do_copy_control(const canvas& rhs) {
            this->base_type::do_copy_control(rhs);
            m_on_paint_cb = rhs.m_on_paint_cb;
            m_on_paint_cb_state = rhs.m_on_paint_cb_state;
        }
        
    public:
        /// @brief Moves a canvas control
        /// @param rhs The control to move
        canvas(canvas&& rhs) {
            do_move_control(rhs);
        }
        /// @brief Moves a canvas control
        /// @param rhs The control to move
        /// @return this
        canvas& operator=(canvas&& rhs) {
            do_move_control(rhs);
            return *this;
        }
        /// @brief Copies a canvas control
        /// @param rhs The control to copy
        canvas(const canvas& rhs) {
            do_copy_control(rhs);
        }
        /// @brief Copies a canvas control
        /// @param rhs The control to copy
        /// @return this
        canvas& operator=(const canvas& rhs) {
            do_copy_control(rhs);
            return *this;
        }
        /// @brief Constructs a canvas from a given parent with an optional palette
        /// @param parent The parent the control is bound to - usually the screen
        /// @param palette The palette associated with the control. This is usually the screen's palette.
        canvas(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent,palette),m_on_paint_cb(nullptr),m_on_paint_cb_state(nullptr) {
        }
        /// @brief Called when the canvas is painted
        /// @param destination The draw destination
        /// @param clip The clipping rectangle
        virtual void on_paint(control_surface_type& destination, const srect16& clip) override {
            if(m_on_paint_cb!=nullptr) {
                m_on_paint_cb(destination,clip,m_on_paint_cb_state);
            }
        }
        /// @brief Retrieves the paint callback
        /// @return A pointer to the callback
        on_paint_callback_type on_paint_callback() const {
            return m_on_paint_cb;
        }
        /// @brief Retrieves the user defined paint callback state
        /// @return The user defined state
        void* on_paint_callback_state() const {
            return m_on_paint_cb_state;
        }
        /// @brief Sets the on_paint callback
        /// @param callback The function to call when the canvas needs to be painted
        /// @param state A user defined state to pass to the paint callback
        void on_paint_callback(on_paint_callback_type callback, void* state = nullptr) {
            m_on_paint_cb_state = state;
            m_on_paint_cb = callback;
        }
        
    };
}
#endif