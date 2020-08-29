/*
 * Copyright 2020 The libgav1 Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBGAV1_SRC_DSP_X86_MOTION_FIELD_PROJECTION_SSE4_H_
#define LIBGAV1_SRC_DSP_X86_MOTION_FIELD_PROJECTION_SSE4_H_

#include "src/dsp/dsp.h"
#include "src/utils/cpu.h"

namespace libgav1 {
namespace dsp {

// Initializes Dsp::motion_field_projection_kernel. This function is not
// thread-safe.
void MotionFieldProjectionInit_SSE4_1();

}  // namespace dsp
}  // namespace libgav1

#if LIBGAV1_TARGETING_SSE4_1
#define LIBGAV1_Dsp8bpp_MotionFieldProjectionKernel LIBGAV1_CPU_SSE4_1
#endif  // LIBGAV1_TARGETING_SSE4_1

#endif  // LIBGAV1_SRC_DSP_X86_MOTION_FIELD_PROJECTION_SSE4_H_
