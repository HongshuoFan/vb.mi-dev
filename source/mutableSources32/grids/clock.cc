// Copyright 2011 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// (non)Global clock.
// vb, hopefully not global anymore in the end...

#include "grids/clock.h"
#include "grids/resources.h"

namespace grids {


void Clock::Update(uint16_t bpm, ClockResolution resolution) {
  bpm_ = bpm;
//  phase_increment_ = pgm_read_dword(lut_res_tempo_phase_increment + bpm);
    phase_increment_ = *(lut_res_tempo_phase_increment + bpm);
  if (resolution == CLOCK_RESOLUTION_4_PPQN) {
    phase_increment_ >>= 1;
  } else if (resolution == CLOCK_RESOLUTION_24_PPQN) {
    phase_increment_ = (phase_increment_ << 1) + phase_increment_;
  }
}
    
    void Clock::Update_vb(uint16_t bpm, ClockResolution resolution, double c) {
        bpm_ = bpm;
        phase_increment_ = bpm * c;
//        if (resolution == CLOCK_RESOLUTION_4_PPQN) {
//            phase_increment_ >>= 1;
//        } else if (resolution == CLOCK_RESOLUTION_24_PPQN) {
            phase_increment_ = (phase_increment_ << 1) + phase_increment_;
//        }
    }

}  // namespace grids
