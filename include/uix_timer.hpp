#ifndef HTCW_UIX_TIMER_HPP
#define HTCW_UIX_TIMER_HPP
#include "uix_core.hpp"
#include <chrono>
namespace uix {
    class timer final {
    public:
        typedef void(*on_tick_callback_type)(void* state);
    private:
        using clock_type = std::chrono::steady_clock;
        using tp_type = clock_type::time_point;
        using dur_type = clock_type::duration;
        constexpr static const double clock_period = ((double)clock_type::period::num/(double)clock_type::period::den);
        constexpr static const double clock_hz = ((double)clock_type::period::den/(double)clock_type::period::num);
        tp_type m_last;
        unsigned long m_interval;
        on_tick_callback_type m_on_tick_callback;
        void* m_on_tick_callback_state;
        timer(const timer& rhs)=delete;
        timer& operator=(const timer& rhs)=delete;
        void do_move(timer& rhs);
    public:
        timer(unsigned long interval = 0,on_tick_callback_type on_tick_callback = nullptr, void* on_tick_callback_state = nullptr);
        timer(timer&& rhs);
        timer& operator=(timer&& rhs);
        unsigned long interval() const;
        void interval(unsigned long value);
        on_tick_callback_type on_tick_callback() const;
        void on_tick_callback(on_tick_callback_type callback, void* state = nullptr);
        void* on_tick_callback_state() const;
        void reset();
        void update();
    };
}
#endif