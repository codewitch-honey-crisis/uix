#include <uix_display.hpp>

namespace uix {
        size_t display::buffer_size() const {
            return m_buffer_size;
        }
        void display::buffer_size(size_t value) {
            m_buffer_size = value;
        }
        uint8_t* display::buffer1() {
            return m_buffer1;
        }
        void display::buffer1(uint8_t* buffer) {
            m_buffer1=buffer;
        }
        uint8_t* display::buffer2() {
            return m_buffer2;
        }
        void display::buffer2(uint8_t* buffer) {
            m_buffer2 = buffer;
        }
        screen_base::on_flush_callback_type display::on_flush_callback() const {
            return m_on_flush_callback;
        }
        void* display::on_flush_callback_state() const {
            return m_on_flush_callback_state;
        }
        void display::on_flush_callback(screen_base::on_flush_callback_type callback, void* state) {
            m_on_flush_callback = callback;
            m_on_flush_callback_state = state;
        }
        screen_base::wait_flush_callback_type display::wait_flush_callback() const {
            return m_wait_flush_callback;
        }
        void* display::wait_flush_callback_state() const {
            return m_wait_flush_callback_state;
        }
        void display::wait_flush_callback(screen_base::wait_flush_callback_type callback, void* state) {
            m_wait_flush_callback = callback;
            m_wait_flush_callback_state = state;
        }
        screen_base::on_touch_callback_type display::on_touch_callback() const {
            return m_on_touch_callback;
        }
        void* display::on_touch_callback_state() const {
            return m_on_touch_callback_state;
        }
        void display::on_touch_callback(screen_base::on_touch_callback_type callback, void* state) {
            m_on_touch_callback = callback;
            m_on_touch_callback_state = state;
        }
        screen_base* display::active_screen() const {
            return m_active_screen;
        }
        void display::active_screen(screen_base* value) {
            if(m_active_screen!=nullptr) {
                m_active_screen->on_flush_callback(nullptr);
                m_active_screen->wait_flush_callback(nullptr);
                m_active_screen->on_touch_callback(nullptr);
            }
            m_active_screen = value;
            if(m_active_screen!=nullptr) {
                m_active_screen->on_flush_callback(m_on_flush_callback,m_on_flush_callback_state);
                m_active_screen->wait_flush_callback(m_wait_flush_callback);
                m_active_screen->on_touch_callback(m_on_touch_callback,m_on_touch_callback_state);
                m_active_screen->buffer_size(m_buffer_size);
                m_active_screen->buffer1(m_buffer1);
                m_active_screen->buffer2(m_buffer2);
                m_active_screen->invalidate();
            }
        }
        void display::flush_complete() {
            if(m_active_screen!=nullptr) {
                m_active_screen->flush_complete();
            }
        }
        uix_result display::update(bool full) {
            if(m_active_screen!=nullptr) {
                return m_active_screen->update(full);
            }
            return uix_result::success;
        }
        bool display::dirty() const {
            if(m_active_screen!=nullptr) {
                return m_active_screen->dirty();
            }
            return false;
        }
}


