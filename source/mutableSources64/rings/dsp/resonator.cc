// Copyright 2015 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Resonator.

#include "rings/dsp/resonator.h"

//#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/cosine_oscillator.h"
#include "stmlib/dsp/parameter_interpolator.h"

#include "rings/resources.h"

//#include <iostream>


//extern double gSampleRate;

namespace rings {

using namespace std;
using namespace stmlib;

void Resonator::Init() {
  for (int32_t i = 0; i < kMaxModes; ++i) {
    f_[i].Init();
  }

  set_frequency(220.0 / Dsp::getSr());
  set_structure(0.25);
  set_brightness(0.5);
  set_damping(0.3);
  set_position(0.999);
  set_resolution(kMaxModes);
    previous_position_ = 0.0;
}

int32_t Resonator::ComputeFilters() {
  double stiffness = Interpolate(lut_stiffness, structure_, 256.0);
  double harmonic = frequency_;
  double stretch_factor = 1.0;
  double q = 500.0 * Interpolate(
      lut_4_decades,
      damping_,
      256.0);
  double brightness_attenuation = 1.0 - structure_;
  // Reduces the range of brightness when structure is very low, to prevent
  // clipping.
  brightness_attenuation *= brightness_attenuation;
  brightness_attenuation *= brightness_attenuation;
  brightness_attenuation *= brightness_attenuation;
  double brightness = brightness_ * (1.0 - 0.2 * brightness_attenuation);
  double q_loss = brightness * (2.0 - brightness) * 0.85 + 0.15;
  double q_loss_damping_rate = structure_ * (2.0 - structure_) * 0.1;
  int32_t num_modes = 0;
  for (int32_t i = 0; i < min(kMaxModes, resolution_); ++i) {
    double partial_frequency = harmonic * stretch_factor;
    if (partial_frequency >= 0.49) {
      partial_frequency = 0.49;
    } else {
      num_modes = i + 1;
    }
    f_[i].set_f_q<FREQUENCY_FAST>(
        partial_frequency,
        1.0 + partial_frequency * q);
    stretch_factor += stiffness;
    if (stiffness < 0.0) {
      // Make sure that the partials do not fold back into negative frequencies.
      stiffness *= 0.93;
    } else {
      // This helps adding a few extra partials in the highest frequencies.
      stiffness *= 0.9;
    }
    // This prevents the highest partials from decaying too fast.
    q_loss += q_loss_damping_rate * (1.0 - q_loss);
    harmonic += frequency_;
    q *= q_loss;
  }
  
  return num_modes;
}

void Resonator::Process(const double* in, double* out, double* aux, size_t size) {
  int32_t num_modes = ComputeFilters();
  
  ParameterInterpolator position(&previous_position_, position_, size);
  while (size--) {
    CosineOscillator amplitudes;
      double freq = position.Next();

    amplitudes.Init<COSINE_OSCILLATOR_APPROXIMATE>(freq);
    //amplitudes.Init<COSINE_OSCILLATOR_EXACT>(freq);

    double input = *in++ * 0.125;
    double odd = 0.0;
    double even = 0.0;
    amplitudes.Start();

    for (int32_t i = 0; i < num_modes;) {
      odd += amplitudes.Next() * f_[i++].Process<FILTER_MODE_BAND_PASS>(input);
      even += amplitudes.Next() * f_[i++].Process<FILTER_MODE_BAND_PASS>(input);
    }
    *out++ = odd;
    *aux++ = even;
  }
}

}  // namespace rings
