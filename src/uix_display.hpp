#ifndef HTCW_UIX_DISPLAY
#define HTCW_UIX_DISPLAY

#include "uix_core.hpp"
#include "uix_screen.hpp"
#include <stdint.h>
#include <stddef.h>

namespace uix {
    class display {
        screen_base* m_active_screen;
        screen_base::on_flush_callback_type m_on_flush_callback;
        void* m_on_flush_callback_state;
        screen_base::on_wait_flush_callback_type m_on_wait_flush_callback;
        void* m_on_wait_flush_callback_state;
        screen_base::on_touch_callback_type m_on_touch_callback;
        void* m_on_touch_callback_state;
        size_t m_buffer_size;
        uint8_t* m_buffer1, *m_buffer2;
        screen_update_mode m_update_mode;
    public:
        // constructs a new instance
        display();
        /// @brief Indicates the update mode for the screen
        /// @return The update mode
        screen_update_mode update_mode() const;
        /// @brief Sets the update mode for the screen
        /// @param mode The update mode
        void update_mode(screen_update_mode mode);
        /// @brief Indicates the size of the transfer buffer(s)
        /// @return a size_t containing the size of the buffer
        size_t buffer_size() const;
        /// @brief Sets the size of the transfer buffer(s)
        /// @param value the new buffer size
        void buffer_size(size_t value);
        /// @brief Gets the first or only buffer
        /// @return A pointer to the buffer
        uint8_t* buffer1();
        /// @brief Sets the first or only buffer
        /// @param buffer A pointer to the new buffer
        void buffer1(uint8_t* buffer);
        /// @brief Gets the second buffer
        /// @return A pointer to the buffer
        uint8_t* buffer2();
        /// @brief Sets the second buffer
        /// @param buffer A pointer to the new buffer
        void buffer2(uint8_t* buffer);
        /// @brief Retrieves the on_flush_callback pointer
        /// @return A pointer to the callback method
        screen_base::on_flush_callback_type on_flush_callback() const;
        /// @brief Retrieves the flush callback state
        /// @return The user defined flush callback state
        void* on_flush_callback_state() const;
        /// @brief Sets the flush callback
        /// @param callback The callback that transfers data to the display
        /// @param state A user defined state value to pass to the callback
        void on_flush_callback(screen_base::on_flush_callback_type callback, void* state = nullptr);
        /// @brief Indicates the wait callback for wait style DMA completion
        /// @return A pointer to the callback method
        screen_base::on_wait_flush_callback_type on_wait_flush_callback() const;
        /// @brief Retrieves the wait callback state
        /// @return The user defined wait callback state
        void* on_wait_flush_callback_state() const;
        /// @brief Sets the wait callback
        /// @param callback The callback that tells the MCU to wait for a previous DMA transfer to complete
        /// @param state A user defined state value to pass to the callback
        void on_wait_flush_callback(screen_base::on_wait_flush_callback_type callback, void* state = nullptr);
        /// @brief Retrieves the touch callback
        /// @return A pointer to the callback method
        screen_base::on_touch_callback_type on_touch_callback() const;
        /// @brief Retrieves the touch callback state
        /// @return The user defined touch callback state
        void* on_touch_callback_state() const;
        /// @brief Sets the touch callback
        /// @param callback The callback that reports locations from a touch screen or pointer
        /// @param state A user defined state value to pass to the callback
        void on_touch_callback(screen_base::on_touch_callback_type callback, void* state = nullptr);
        /// @brief Indicates the active screen
        /// @return returns the active screen for this display, if any.
        screen_base& active_screen() const;
        /// @brief Sets the active screen for the control
        /// @param value The screen to set
        void active_screen(screen_base& value);
        /// @brief Call when a flush has finished so the screen can recycle the buffers. Should either be called in the flush callback implementation (no DMA) or via a DMA completion callback that signals when the previous transfer was completed.
        void flush_complete();
        /// @brief Indicates that the display is currently waiting to be able to flush
        bool flush_pending() const;
        /// @brief Indicates that the display is currently flushing
        bool flushing() const;
        /// @brief Updates the display
        /// @param full True to do a full update, false to update maximum of one flush.
        /// @return True if the screen was updated, otherwise false
        uix_result update(bool full = true);
        /// @brief Indicates if the screen has any dirty regions to update and flush
        /// @return True if the screen needs updating, otherwise false
        bool dirty() const;
    };
}
#endif // HTCW_UIX_DISPLAY