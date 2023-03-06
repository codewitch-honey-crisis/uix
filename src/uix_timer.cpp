#include <uix_timer.hpp>

using namespace uix;
void timer::do_move(timer& rhs) {
    m_last = rhs.m_last;
    m_interval = rhs.m_interval;
    rhs.m_interval = 0;
    m_on_tick_callback = rhs.m_on_tick_callback;
    m_on_tick_callback_state = rhs.m_on_tick_callback_state;
}
timer::timer(unsigned long interval,on_tick_callback_type on_tick_callback, void* on_tick_callback_state) : m_last(clock_type::now()),m_interval(interval),m_on_tick_callback(on_tick_callback),m_on_tick_callback_state(on_tick_callback_state) {

}
timer::timer(timer&& rhs) {
    do_move(rhs);
}
timer& timer::operator=(timer&& rhs) {
    do_move(rhs);
    return *this;
}
unsigned long timer::interval() const {
    return m_interval;
}
void timer::interval(unsigned long value) {
    m_interval = value;
}
timer::on_tick_callback_type timer::on_tick_callback() const {
    return m_on_tick_callback;
}
void timer::on_tick_callback(on_tick_callback_type callback, void* state) {
    m_on_tick_callback = callback;
    m_on_tick_callback_state = state;
}
void* timer::on_tick_callback_state() const {
    return m_on_tick_callback_state;
}
void timer::reset() {
    m_last = clock_type::now();
}
void timer::update() {
    auto per = clock_type::now()-m_last;
    double elapsed_secs = per.count()/clock_hz;
    if(elapsed_secs!=0.0) {
        double elapsed_ticks = elapsed_secs*1000;
        if(elapsed_ticks>=m_interval) {
            if(m_on_tick_callback!=nullptr) {
                m_on_tick_callback(m_on_tick_callback_state);
            }
            m_last = clock_type::now();
        }
    }
}
