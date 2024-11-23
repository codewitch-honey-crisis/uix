#pragma once
#include "hamming_window.hpp"
#include <stdint.h>
#include "kiss_fftr.h"

template<size_t WindowSize = 512> class processor {
 private:
  constexpr static size_t compute_fft_size()
  {
    size_t result = 1;
    while (result < window_size) {
      result *= 2;
    }
    return result;
  }
  hamming_window<compute_fft_size()> m_hamming_window;
  kiss_fftr_cfg m_cfg;
  kiss_fft_cpx m_fft_output[compute_fft_size() / 2];

 public:
  static constexpr const size_t fft_size = compute_fft_size();
  static constexpr const size_t window_size = WindowSize;
  static constexpr const float window_mult = float(1)/float(WindowSize);
  float m_fft[window_size / 4];
  float m_samples[fft_size];
  const float *fft() const
  {
    return m_fft;
  }
  const float* samples() const {
    return m_samples;
  }
  processor()
  {
    
    // initialise kiss fftr
    m_cfg = kiss_fftr_alloc(fft_size, false, 0, 0);

    for (int i = 0; i < fft_size; i++) {
      m_samples[i] = 0;
    }
   
  }
  void update(const int16_t *samples)
  {
    int offset = (fft_size - window_size) / 2;
    float input_avg = 0.0f;

    for (int i = 0; i < window_size; i++) {
      const float input_sample = samples[i] * (1.0f/30.0f);
      m_samples[offset + i] = input_sample;
      input_avg += float(input_sample) * window_mult;
    }

    // Remove any DC offset. Various M5Stack Core2 devices have shown different DC
    // offsets in their recordings, and this resolves that at least for the
    // sampling window. This does mean that signals with periods longer than the
    // sampling window will be negatively affected.
    for (int i = 0; i < window_size; i++) {
      m_samples[offset + i] -= input_avg;
    }

    // apply the hamming window
    m_hamming_window.apply(m_samples);

    // do the fft
    kiss_fftr(m_cfg, m_samples, m_fft_output);

    for (int i = 0; i < window_size / 4; i++) {
      const float real = m_fft_output[i].r;
      const float imag = m_fft_output[i].i;
      m_fft[i] = sqrtf((real * real) + (imag * imag));
    }
  }
};
