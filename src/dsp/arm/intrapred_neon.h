/*
 * Copyright 2019 The libgav1 Authors
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

#ifndef LIBGAV1_SRC_DSP_ARM_INTRAPRED_NEON_H_
#define LIBGAV1_SRC_DSP_ARM_INTRAPRED_NEON_H_

#include "src/dsp/dsp.h"
#include "src/utils/cpu.h"

namespace libgav1 {
namespace dsp {

// Initializes Dsp::intra_predictors, Dsp::cfl_intra_predictors, and
// Dsp::cfl_subsamplers, see the defines below for specifics. These functions
// are not thread-safe.
void IntraPredInit_NEON();
void IntraPredSmoothInit_NEON();

}  // namespace dsp
}  // namespace libgav1

#if LIBGAV1_ENABLE_NEON
// 4x4
#define LIBGAV1_Dsp8bpp_TransformSize4x4_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x4_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x4_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x4_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x4_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x4_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x4_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize4x4_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x4_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x4_CflSubsampler444 LIBGAV1_CPU_NEON

// 4x8
#define LIBGAV1_Dsp8bpp_TransformSize4x8_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x8_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x8_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x8_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x8_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x8_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x8_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize4x8_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x8_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x8_CflSubsampler444 LIBGAV1_CPU_NEON

// 4x16
#define LIBGAV1_Dsp8bpp_TransformSize4x16_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x16_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x16_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x16_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x16_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x16_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x16_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize4x16_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x16_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize4x16_CflSubsampler444 LIBGAV1_CPU_NEON

// 8x4
#define LIBGAV1_Dsp8bpp_TransformSize8x4_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x4_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x4_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x4_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x4_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x4_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x4_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize8x4_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x4_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x4_CflSubsampler444 LIBGAV1_CPU_NEON

// 8x8
#define LIBGAV1_Dsp8bpp_TransformSize8x8_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x8_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x8_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x8_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x8_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x8_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x8_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize8x8_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x8_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x8_CflSubsampler444 LIBGAV1_CPU_NEON

// 8x16
#define LIBGAV1_Dsp8bpp_TransformSize8x16_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x16_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x16_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x16_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x16_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x16_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x16_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize8x16_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x16_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x16_CflSubsampler444 LIBGAV1_CPU_NEON

// 8x32
#define LIBGAV1_Dsp8bpp_TransformSize8x32_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x32_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x32_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x32_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x32_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x32_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x32_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize8x32_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x32_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize8x32_CflSubsampler444 LIBGAV1_CPU_NEON

// 16x4
#define LIBGAV1_Dsp8bpp_TransformSize16x4_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x4_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x4_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x4_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x4_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x4_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x4_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize16x4_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x4_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x4_CflSubsampler444 LIBGAV1_CPU_NEON

// 16x8
#define LIBGAV1_Dsp8bpp_TransformSize16x8_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x8_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x8_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x8_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x8_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x8_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x8_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize16x8_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x8_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x8_CflSubsampler444 LIBGAV1_CPU_NEON

// 16x16
#define LIBGAV1_Dsp8bpp_TransformSize16x16_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x16_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x16_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x16_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x16_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x16_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x16_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize16x16_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x16_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x16_CflSubsampler444 LIBGAV1_CPU_NEON

// 16x32
#define LIBGAV1_Dsp8bpp_TransformSize16x32_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x32_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x32_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x32_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x32_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x32_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x32_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize16x32_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x32_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x32_CflSubsampler444 LIBGAV1_CPU_NEON

// 16x64
#define LIBGAV1_Dsp8bpp_TransformSize16x64_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x64_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x64_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x64_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x64_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x64_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize16x64_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

// 32x8
#define LIBGAV1_Dsp8bpp_TransformSize32x8_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x8_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x8_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x8_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x8_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x8_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x8_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize32x8_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x8_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x8_CflSubsampler444 LIBGAV1_CPU_NEON

// 32x16
#define LIBGAV1_Dsp8bpp_TransformSize32x16_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x16_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x16_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x16_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x16_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x16_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x16_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize32x16_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x16_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x16_CflSubsampler444 LIBGAV1_CPU_NEON

// 32x32
#define LIBGAV1_Dsp8bpp_TransformSize32x32_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x32_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x32_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x32_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x32_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x32_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x32_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp8bpp_TransformSize32x32_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x32_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x32_CflSubsampler444 LIBGAV1_CPU_NEON

// 32x64
#define LIBGAV1_Dsp8bpp_TransformSize32x64_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x64_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x64_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x64_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x64_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x64_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize32x64_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

// 64x16
#define LIBGAV1_Dsp8bpp_TransformSize64x16_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x16_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x16_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x16_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x16_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x16_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x16_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

// 64x32
#define LIBGAV1_Dsp8bpp_TransformSize64x32_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x32_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x32_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x32_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x32_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x32_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x32_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

// 64x64
#define LIBGAV1_Dsp8bpp_TransformSize64x64_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x64_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x64_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x64_IntraPredictorPaeth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x64_IntraPredictorSmooth LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x64_IntraPredictorSmoothVertical \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp8bpp_TransformSize64x64_IntraPredictorSmoothHorizontal \
  LIBGAV1_CPU_NEON

// 10 bit
// 4x4
#define LIBGAV1_Dsp10bpp_TransformSize4x4_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x4_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x4_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x4_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize4x4_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x4_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x4_CflSubsampler444 LIBGAV1_CPU_NEON

// 4x8
#define LIBGAV1_Dsp10bpp_TransformSize4x8_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x8_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x8_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x8_IntraPredictorHorizontal \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x8_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize4x8_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x8_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x8_CflSubsampler444 LIBGAV1_CPU_NEON

// 4x16
#define LIBGAV1_Dsp10bpp_TransformSize4x16_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x16_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x16_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x16_IntraPredictorHorizontal \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x16_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize4x16_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x16_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize4x16_CflSubsampler444 LIBGAV1_CPU_NEON

// 8x4
#define LIBGAV1_Dsp10bpp_TransformSize8x4_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x4_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x4_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x4_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize8x4_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x4_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x4_CflSubsampler444 LIBGAV1_CPU_NEON

// 8x8
#define LIBGAV1_Dsp10bpp_TransformSize8x8_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x8_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x8_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x8_IntraPredictorHorizontal \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x8_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize8x8_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x8_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x8_CflSubsampler444 LIBGAV1_CPU_NEON

// 8x16
#define LIBGAV1_Dsp10bpp_TransformSize8x16_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x16_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x16_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x16_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize8x16_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x16_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x16_CflSubsampler444 LIBGAV1_CPU_NEON

// 8x32
#define LIBGAV1_Dsp10bpp_TransformSize8x32_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x32_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x32_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x32_IntraPredictorHorizontal \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x32_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize8x32_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x32_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize8x32_CflSubsampler444 LIBGAV1_CPU_NEON

// 16x4
#define LIBGAV1_Dsp10bpp_TransformSize16x4_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x4_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x4_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x4_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize16x4_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x4_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x4_CflSubsampler444 LIBGAV1_CPU_NEON

// 16x8
#define LIBGAV1_Dsp10bpp_TransformSize16x8_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x8_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x8_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x8_IntraPredictorHorizontal \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x8_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize16x8_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x8_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x8_CflSubsampler444 LIBGAV1_CPU_NEON

// 16x16
#define LIBGAV1_Dsp10bpp_TransformSize16x16_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x16_IntraPredictorDcLeft \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x16_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x16_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize16x16_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x16_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x16_CflSubsampler444 LIBGAV1_CPU_NEON

// 16x32
#define LIBGAV1_Dsp10bpp_TransformSize16x32_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x32_IntraPredictorDcLeft \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x32_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x32_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize16x32_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x32_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x32_CflSubsampler444 LIBGAV1_CPU_NEON

// 16x64
#define LIBGAV1_Dsp10bpp_TransformSize16x64_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x64_IntraPredictorDcLeft \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x64_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize16x64_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

// 32x8
#define LIBGAV1_Dsp10bpp_TransformSize32x8_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x8_IntraPredictorDcLeft LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x8_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x8_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize32x8_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x8_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x8_CflSubsampler444 LIBGAV1_CPU_NEON

// 32x16
#define LIBGAV1_Dsp10bpp_TransformSize32x16_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x16_IntraPredictorDcLeft \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x16_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x16_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize32x16_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x16_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x16_CflSubsampler444 LIBGAV1_CPU_NEON

// 32x32
#define LIBGAV1_Dsp10bpp_TransformSize32x32_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x32_IntraPredictorDcLeft \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x32_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x32_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

#define LIBGAV1_Dsp10bpp_TransformSize32x32_CflIntraPredictor LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x32_CflSubsampler420 LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x32_CflSubsampler444 LIBGAV1_CPU_NEON

// 32x64
#define LIBGAV1_Dsp10bpp_TransformSize32x64_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x64_IntraPredictorDcLeft \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x64_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x64_IntraPredictorHorizontal \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize32x64_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

// 64x16
#define LIBGAV1_Dsp10bpp_TransformSize64x16_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize64x16_IntraPredictorDcLeft \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize64x16_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize64x16_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

// 64x32
#define LIBGAV1_Dsp10bpp_TransformSize64x32_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize64x32_IntraPredictorDcLeft \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize64x32_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize64x32_IntraPredictorVertical \
  LIBGAV1_CPU_NEON

// 64x64
#define LIBGAV1_Dsp10bpp_TransformSize64x64_IntraPredictorDcTop LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize64x64_IntraPredictorDcLeft \
  LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize64x64_IntraPredictorDc LIBGAV1_CPU_NEON
#define LIBGAV1_Dsp10bpp_TransformSize64x64_IntraPredictorVertical \
  LIBGAV1_CPU_NEON
#endif  // LIBGAV1_ENABLE_NEON

#endif  // LIBGAV1_SRC_DSP_ARM_INTRAPRED_NEON_H_
