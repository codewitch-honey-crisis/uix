#ifndef HTCW_UIX_PAINTER_HPP
#define HTCW_UIX_PAINTER_HPP
#include "uix_core.hpp"
namespace uix {
    /// @brief A general purpose painter control that allows freeform drawing
    /// @tparam ControlSurfaceType The type of control surface - usually the screen's control_surface_type
    template<typename ControlSurfaceType> class painter : public control<ControlSurfaceType> {
    public:
        using type = painter;
        using base_type = control<ControlSurfaceType>;
        using pixel_type = typename ControlSurfaceType::pixel_type;
        using palette_type = typename ControlSurfaceType::palette_type;
        using control_surface_type = ControlSurfaceType;
        /// @brief The callback type for paint operations
        typedef void(*on_paint_callback_type)(control_surface_type& destination,const srect16& clip,void* state);
        /// @brief The callback type for the touch operation
        typedef void(*on_touch_callback_type)(size_t locations_size,const spoint16* locations,void* state);
        /// @brief The callback type for touch release operation
        typedef void(*on_release_callback_type)(void* state);
    private:
        on_paint_callback_type m_on_paint_cb;
        void* m_on_paint_cb_state;
        on_touch_callback_type m_on_touch_cb;
        void* m_on_touch_cb_state;
        on_release_callback_type m_on_release_cb;
        void* m_on_release_cb_state;
    protected:
        /// @brief For derivative classes, moves the control
        /// @param rhs The painter to move
        void do_move_control(painter& rhs) {
            this->base_type::do_move_control(rhs);
            m_on_paint_cb = rhs.m_on_paint_cb;
            rhs.m_on_paint_cb = nullptr;
            m_on_paint_cb_state = rhs.m_on_paint_cb_state;
            m_on_touch_cb = rhs.m_on_touch_cb;
            rhs.m_on_touch_cb = nullptr;
            m_on_touch_cb_state = rhs.m_on_touch_cb_state;
            m_on_release_cb = rhs.m_on_release_cb;
            rhs.m_on_release_cb = nullptr;
            m_on_release_cb_state = rhs.m_on_release_cb_state;
        }
        /// @brief For derivative classes, copies the control
        /// @param rhs The painter to copy
        void do_copy_control(const painter& rhs) {
            this->base_type::do_copy_control(rhs);
            m_on_paint_cb = rhs.m_on_paint_cb;
            m_on_paint_cb_state = rhs.m_on_paint_cb_state;
            m_on_touch_cb = rhs.m_on_touch_cb;
            m_on_touch_cb_state = rhs.m_on_touch_cb_state;
            m_on_release_cb = rhs.m_on_release_cb;
            m_on_release_cb_state = rhs.m_on_release_cb_state;
        }
        
    public:
        /// @brief Moves a painter control
        /// @param rhs The control to move
        painter(painter&& rhs) {
            do_move_control(rhs);
        }
        /// @brief Moves a painter control
        /// @param rhs The control to move
        /// @return this
        painter& operator=(painter&& rhs) {
            do_move_control(rhs);
            return *this;
        }
        /// @brief Copies a painter control
        /// @param rhs The control to copy
        painter(const painter& rhs) {
            do_copy_control(rhs);
        }
        /// @brief Copies a painter control
        /// @param rhs The control to copy
        /// @return this
        painter& operator=(const painter& rhs) {
            do_copy_control(rhs);
            return *this;
        }
        /// @brief Constructs a painter from a given parent with an optional palette
        /// @param parent The parent the control is bound to - usually the screen
        /// @param palette The palette associated with the control. This is usually the screen's palette.
        painter(invalidation_tracker& parent, const palette_type* palette = nullptr) : base_type(parent,palette),m_on_paint_cb(nullptr),m_on_paint_cb_state(nullptr) {
        }
        /// @brief Constructs a painter from a given parent with an optional palette
        painter() : base_type(),m_on_paint_cb(nullptr),m_on_paint_cb_state(nullptr) {
        }
        /// @brief Called when the painter is painted
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
        /// @param callback The function to call when the painter needs to be painted
        /// @param state A user defined state to pass to the paint callback
        void on_paint_callback(on_paint_callback_type callback, void* state = nullptr) {
            m_on_paint_cb_state = state;
            m_on_paint_cb = callback;
        }
        /// @brief Called when the painter is touched
        /// @param locations_size The count of locations
        /// @param locations The locations
        /// @return True if handled, otherwise false
        virtual bool on_touch(size_t locations_size,const spoint16* locations) override {
            if(m_on_touch_cb!=nullptr) {
                m_on_touch_cb(locations_size,locations,m_on_touch_cb_state);
                return true;
            }
            return false;
        }
        /// @brief Retrieves the touch callback
        /// @return A pointer to the callback
        on_touch_callback_type on_touch_callback() const {
            return m_on_touch_cb;
        }
        /// @brief Retrieves the user defined touch callback state
        /// @return The user defined state
        void* on_touch_callback_state() const {
            return m_on_touch_cb_state;
        }
        /// @brief Sets the on_touch callback
        /// @param callback The function to call when the painter is touched
        /// @param state A user defined state to pass to the touch callback
        void on_touch_callback(on_touch_callback_type callback, void* state = nullptr) {
            m_on_touch_cb_state = state;
            m_on_touch_cb = callback;
        }
         /// @brief Called when the button is released.
        virtual void on_release() override {
            if(m_on_release_cb!=nullptr) {
                m_on_release_cb(m_on_release_cb_state);
            }
        }
        /// @brief Retrieves the release callback
        /// @return A pointer to the callback
        on_release_callback_type on_release_callback() const {
            return m_on_release_cb;
        }
        /// @brief Retrieves the user defined release callback state
        /// @return The user defined state
        void* on_release_callback_state() const {
            return m_on_release_cb_state;
        }
        /// @brief Sets the on_release callback
        /// @param callback The function to call when the painter is released
        /// @param state A user defined state to pass to the touch callback
        void on_release_callback(on_release_callback_type callback, void* state = nullptr) {
            m_on_release_cb_state = state;
            m_on_release_cb = callback;
        }       
    };
}
#endif