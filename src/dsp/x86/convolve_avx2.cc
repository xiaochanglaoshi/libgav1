// Copyright 2020 The libgav1 Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/dsp/convolve.h"
#include "src/utils/cpu.h"

#if LIBGAV1_TARGETING_AVX2
#include <immintrin.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>

#include "src/dsp/constants.h"
#include "src/dsp/dsp.h"
#include "src/dsp/x86/common_avx2.h"
#include "src/dsp/x86/common_sse4.h"
#include "src/utils/common.h"
#include "src/utils/constants.h"

namespace libgav1 {
namespace dsp {
namespace low_bitdepth {
namespace {

#include "src/dsp/convolve.inc"

// Multiply every entry in |src[]| by the corresponding entry in |taps[]| and
// sum. The filters in |taps[]| are pre-shifted by 1. This prevents the final
// sum from outranging int16_t.
template <int filter_index>
__m128i SumOnePassTaps(const __m128i* const src, const __m128i* const taps) {
  __m128i sum;
  if (filter_index < 2) {
    // 6 taps.
    const __m128i v_madd_21 = _mm_maddubs_epi16(src[0], taps[0]);  // k2k1
    const __m128i v_madd_43 = _mm_maddubs_epi16(src[1], taps[1]);  // k4k3
    const __m128i v_madd_65 = _mm_maddubs_epi16(src[2], taps[2]);  // k6k5
    sum = _mm_add_epi16(v_madd_21, v_madd_43);
    sum = _mm_add_epi16(sum, v_madd_65);
  } else if (filter_index == 2) {
    // 8 taps.
    const __m128i v_madd_10 = _mm_maddubs_epi16(src[0], taps[0]);  // k1k0
    const __m128i v_madd_32 = _mm_maddubs_epi16(src[1], taps[1]);  // k3k2
    const __m128i v_madd_54 = _mm_maddubs_epi16(src[2], taps[2]);  // k5k4
    const __m128i v_madd_76 = _mm_maddubs_epi16(src[3], taps[3]);  // k7k6
    const __m128i v_sum_3210 = _mm_add_epi16(v_madd_10, v_madd_32);
    const __m128i v_sum_7654 = _mm_add_epi16(v_madd_54, v_madd_76);
    sum = _mm_add_epi16(v_sum_7654, v_sum_3210);
  } else if (filter_index == 3) {
    // 2 taps.
    sum = _mm_maddubs_epi16(src[0], taps[0]);  // k4k3
  } else {
    // 4 taps.
    const __m128i v_madd_32 = _mm_maddubs_epi16(src[0], taps[0]);  // k3k2
    const __m128i v_madd_54 = _mm_maddubs_epi16(src[1], taps[1]);  // k5k4
    sum = _mm_add_epi16(v_madd_32, v_madd_54);
  }
  return sum;
}

// Multiply every entry in |src[]| by the corresponding entry in |taps[]| and
// sum. The filters in |taps[]| are pre-shifted by 1. This prevents the final
// sum from outranging int16_t.
template <int filter_index>
__m256i SumOnePassTaps(const __m256i* const src, const __m256i* const taps) {
  __m256i sum;
  if (filter_index < 2) {
    // 6 taps.
    const __m256i v_madd_21 = _mm256_maddubs_epi16(src[0], taps[0]);  // k2k1
    const __m256i v_madd_43 = _mm256_maddubs_epi16(src[1], taps[1]);  // k4k3
    const __m256i v_madd_65 = _mm256_maddubs_epi16(src[2], taps[2]);  // k6k5
    sum = _mm256_add_epi16(v_madd_21, v_madd_43);
    sum = _mm256_add_epi16(sum, v_madd_65);
  } else if (filter_index == 2) {
    // 8 taps.
    const __m256i v_madd_10 = _mm256_maddubs_epi16(src[0], taps[0]);  // k1k0
    const __m256i v_madd_32 = _mm256_maddubs_epi16(src[1], taps[1]);  // k3k2
    const __m256i v_madd_54 = _mm256_maddubs_epi16(src[2], taps[2]);  // k5k4
    const __m256i v_madd_76 = _mm256_maddubs_epi16(src[3], taps[3]);  // k7k6
    const __m256i v_sum_3210 = _mm256_add_epi16(v_madd_10, v_madd_32);
    const __m256i v_sum_7654 = _mm256_add_epi16(v_madd_54, v_madd_76);
    sum = _mm256_add_epi16(v_sum_7654, v_sum_3210);
  } else if (filter_index == 3) {
    // 2 taps.
    sum = _mm256_maddubs_epi16(src[0], taps[0]);  // k4k3
  } else {
    // 4 taps.
    const __m256i v_madd_32 = _mm256_maddubs_epi16(src[0], taps[0]);  // k3k2
    const __m256i v_madd_54 = _mm256_maddubs_epi16(src[1], taps[1]);  // k5k4
    sum = _mm256_add_epi16(v_madd_32, v_madd_54);
  }
  return sum;
}

template <int filter_index>
__m256i SumHorizontalTaps(const __m256i* const src,
                          const __m256i* const v_tap) {
  __m256i v_src[4];
  const __m256i src_long = *src;
  const __m256i src_long_dup_lo = _mm256_unpacklo_epi8(src_long, src_long);
  const __m256i src_long_dup_hi = _mm256_unpackhi_epi8(src_long, src_long);

  if (filter_index < 2) {
    // 6 taps.
    v_src[0] = _mm256_alignr_epi8(src_long_dup_hi, src_long_dup_lo, 3);   // _21
    v_src[1] = _mm256_alignr_epi8(src_long_dup_hi, src_long_dup_lo, 7);   // _43
    v_src[2] = _mm256_alignr_epi8(src_long_dup_hi, src_long_dup_lo, 11);  // _65
  } else if (filter_index == 2) {
    // 8 taps.
    v_src[0] = _mm256_alignr_epi8(src_long_dup_hi, src_long_dup_lo, 1);   // _10
    v_src[1] = _mm256_alignr_epi8(src_long_dup_hi, src_long_dup_lo, 5);   // _32
    v_src[2] = _mm256_alignr_epi8(src_long_dup_hi, src_long_dup_lo, 9);   // _54
    v_src[3] = _mm256_alignr_epi8(src_long_dup_hi, src_long_dup_lo, 13);  // _76
  } else if (filter_index == 3) {
    // 2 taps.
    v_src[0] = _mm256_alignr_epi8(src_long_dup_hi, src_long_dup_lo, 7);  // _43
  } else if (filter_index > 3) {
    // 4 taps.
    v_src[0] = _mm256_alignr_epi8(src_long_dup_hi, src_long_dup_lo, 5);  // _32
    v_src[1] = _mm256_alignr_epi8(src_long_dup_hi, src_long_dup_lo, 9);  // _54
  }
  return SumOnePassTaps<filter_index>(v_src, v_tap);
}

template <int filter_index>
__m256i SimpleHorizontalTaps(const __m256i* const src,
                             const __m256i* const v_tap) {
  __m256i sum = SumHorizontalTaps<filter_index>(src, v_tap);

  // Normally the Horizontal pass does the downshift in two passes:
  // kInterRoundBitsHorizontal - 1 and then (kFilterBits -
  // kInterRoundBitsHorizontal). Each one uses a rounding shift. Combining them
  // requires adding the rounding offset from the skipped shift.
  constexpr int first_shift_rounding_bit = 1 << (kInterRoundBitsHorizontal - 2);

  sum = _mm256_add_epi16(sum, _mm256_set1_epi16(first_shift_rounding_bit));
  sum = RightShiftWithRounding_S16(sum, kFilterBits - 1);
  return _mm256_packus_epi16(sum, sum);
}

template <int filter_index>
__m256i HorizontalTaps8To16(const __m256i* const src,
                            const __m256i* const v_tap) {
  const __m256i sum = SumHorizontalTaps<filter_index>(src, v_tap);

  return RightShiftWithRounding_S16(sum, kInterRoundBitsHorizontal - 1);
}

template <int filter_index>
__m128i SumHorizontalTaps2x2(const uint8_t* src, const ptrdiff_t src_stride,
                             const __m128i* const v_tap) {
  // 00 01 02 03 04 05 06 07 10 11 12 13 14 15 16 17
  const __m128i v_src = LoadHi8(LoadLo8(&src[0]), &src[src_stride]);

  if (filter_index == 3) {
    // 03 04 04 05 05 06 06 07 13 14 14 15 15 16 16 17
    const __m128i v_src_43 = _mm_shuffle_epi8(
        v_src, _mm_set_epi32(0x0f0e0e0d, 0x0d0c0c0b, 0x07060605, 0x05040403));
    const __m128i v_sum_43 = _mm_maddubs_epi16(v_src_43, v_tap[0]);  // k4k3
    return v_sum_43;
  }

  // 02 03 03 04 04 05 05 06 12 13 13 14 14 15 15 16
  const __m128i v_src_32 = _mm_shuffle_epi8(
      v_src, _mm_set_epi32(0x0e0d0d0c, 0x0c0b0b0a, 0x06050504, 0x04030302));
  // 04 05 05 06 06 07 07 xx 14 15 15 16 16 17 17 xx
  const __m128i v_src_54 = _mm_shuffle_epi8(
      v_src, _mm_set_epi32(0x800f0f0e, 0x0e0d0d0c, 0x80070706, 0x06050504));
  const __m128i v_madd_32 = _mm_maddubs_epi16(v_src_32, v_tap[0]);  // k3k2
  const __m128i v_madd_54 = _mm_maddubs_epi16(v_src_54, v_tap[1]);  // k5k4
  const __m128i v_sum_5432 = _mm_add_epi16(v_madd_54, v_madd_32);
  return v_sum_5432;
}

template <int filter_index>
__m128i SimpleHorizontalTaps2x2(const uint8_t* src, const ptrdiff_t src_stride,
                                const __m128i* const v_tap) {
  __m128i sum = SumHorizontalTaps2x2<filter_index>(src, src_stride, v_tap);

  // Normally the Horizontal pass does the downshift in two passes:
  // kInterRoundBitsHorizontal - 1 and then (kFilterBits -
  // kInterRoundBitsHorizontal). Each one uses a rounding shift. Combining them
  // requires adding the rounding offset from the skipped shift.
  constexpr int first_shift_rounding_bit = 1 << (kInterRoundBitsHorizontal - 2);

  sum = _mm_add_epi16(sum, _mm_set1_epi16(first_shift_rounding_bit));
  sum = RightShiftWithRounding_S16(sum, kFilterBits - 1);
  return _mm_packus_epi16(sum, sum);
}

template <int filter_index>
__m128i HorizontalTaps8To16_2x2(const uint8_t* src, const ptrdiff_t src_stride,
                                const __m128i* const v_tap) {
  const __m128i sum =
      SumHorizontalTaps2x2<filter_index>(src, src_stride, v_tap);

  return RightShiftWithRounding_S16(sum, kInterRoundBitsHorizontal - 1);
}

// Filter 2xh sizes.
template <int num_taps, int step, int filter_index, bool is_2d = false,
          bool is_compound = false>
void FilterHorizontal(const uint8_t* src, const ptrdiff_t src_stride,
                      void* const dest, const ptrdiff_t pred_stride,
                      const int /*width*/, const int height,
                      const __m128i* const v_tap) {
  auto* dest8 = static_cast<uint8_t*>(dest);
  auto* dest16 = static_cast<uint16_t*>(dest);

  // Horizontal passes only need to account for |num_taps| 2 and 4 when
  // |width| <= 4.
  assert(num_taps <= 4);
  if (num_taps <= 4) {
    if (!is_compound) {
      int y = 0;
      do {
        if (is_2d) {
          const __m128i sum =
              HorizontalTaps8To16_2x2<filter_index>(src, src_stride, v_tap);
          Store4(&dest16[0], sum);
          dest16 += pred_stride;
          Store4(&dest16[0], _mm_srli_si128(sum, 8));
          dest16 += pred_stride;
        } else {
          const __m128i sum =
              SimpleHorizontalTaps2x2<filter_index>(src, src_stride, v_tap);
          Store2(dest8, sum);
          dest8 += pred_stride;
          Store2(dest8, _mm_srli_si128(sum, 4));
          dest8 += pred_stride;
        }

        src += src_stride << 1;
        y += 2;
      } while (y < height - 1);

      // The 2d filters have an odd |height| because the horizontal pass
      // generates context for the vertical pass.
      if (is_2d) {
        assert(height % 2 == 1);
        __m128i sum;
        const __m128i input = LoadLo8(&src[2]);
        if (filter_index == 3) {
          // 03 04 04 05 05 06 06 07 ....
          const __m128i v_src_43 =
              _mm_srli_si128(_mm_unpacklo_epi8(input, input), 3);
          sum = _mm_maddubs_epi16(v_src_43, v_tap[0]);  // k4k3
        } else {
          // 02 03 03 04 04 05 05 06 06 07 ....
          const __m128i v_src_32 =
              _mm_srli_si128(_mm_unpacklo_epi8(input, input), 1);
          // 04 05 05 06 06 07 07 08 ...
          const __m128i v_src_54 = _mm_srli_si128(v_src_32, 4);
          const __m128i v_madd_32 =
              _mm_maddubs_epi16(v_src_32, v_tap[0]);  // k3k2
          const __m128i v_madd_54 =
              _mm_maddubs_epi16(v_src_54, v_tap[1]);  // k5k4
          sum = _mm_add_epi16(v_madd_54, v_madd_32);
        }
        sum = RightShiftWithRounding_S16(sum, kInterRoundBitsHorizontal - 1);
        Store4(dest16, sum);
      }
    }
  }
}

// Filter widths >= 4.
template <int num_taps, int step, int filter_index, bool is_2d = false,
          bool is_compound = false>
void FilterHorizontal(const uint8_t* src, const ptrdiff_t src_stride,
                      void* const dest, const ptrdiff_t pred_stride,
                      const int width, const int height,
                      const __m256i* const v_tap) {
  auto* dest8 = static_cast<uint8_t*>(dest);
  auto* dest16 = static_cast<uint16_t*>(dest);

  if (width >= 32) {
    int y = height;
    do {
      int x = 0;
      do {
        if (is_2d || is_compound) {
          // Load into 2 128 bit lanes.
          const __m256i src_long =
              SetrM128i(LoadUnaligned16(&src[x]), LoadUnaligned16(&src[x + 8]));
          const __m256i result =
              HorizontalTaps8To16<filter_index>(&src_long, v_tap);
          const __m256i src_long2 = SetrM128i(LoadUnaligned16(&src[x + 16]),
                                              LoadUnaligned16(&src[x + 24]));
          const __m256i result2 =
              HorizontalTaps8To16<filter_index>(&src_long2, v_tap);
          if (is_2d) {
            StoreAligned32(&dest16[x], result);
            StoreAligned32(&dest16[x + 16], result2);
          } else {
            StoreUnaligned32(&dest16[x], result);
            StoreUnaligned32(&dest16[x + 16], result2);
          }
        } else {
          // Load src used to calculate dest8[7:0] and dest8[23:16].
          const __m256i src_long = LoadUnaligned32(&src[x]);
          const __m256i result =
              SimpleHorizontalTaps<filter_index>(&src_long, v_tap);
          // Load src used to calculate dest8[15:8] and dest8[31:24].
          const __m256i src_long2 = LoadUnaligned32(&src[x + 8]);
          const __m256i result2 =
              SimpleHorizontalTaps<filter_index>(&src_long2, v_tap);
          // Combine results and store.
          StoreUnaligned32(&dest8[x], _mm256_unpacklo_epi64(result, result2));
        }
        x += step * 4;
      } while (x < width);
      src += src_stride;
      dest8 += pred_stride;
      dest16 += pred_stride;
    } while (--y != 0);
  } else if (width == 16) {
    int y = height;
    if (is_2d) y -= 1;
    do {
      if (is_2d || is_compound) {
        // Load into 2 128 bit lanes.
        const __m256i src_long =
            SetrM128i(LoadUnaligned16(&src[0]), LoadUnaligned16(&src[8]));
        const __m256i result =
            HorizontalTaps8To16<filter_index>(&src_long, v_tap);
        const __m256i src_long2 =
            SetrM128i(LoadUnaligned16(&src[src_stride]),
                      LoadUnaligned16(&src[8 + src_stride]));
        const __m256i result2 =
            HorizontalTaps8To16<filter_index>(&src_long2, v_tap);
        if (is_2d) {
          StoreAligned32(&dest16[0], result);
          StoreAligned32(&dest16[pred_stride], result2);
        } else {
          StoreUnaligned32(&dest16[0], result);
          StoreUnaligned32(&dest16[pred_stride], result2);
        }
      } else {
        // Load into 2 128 bit lanes.
        const __m256i src_long = SetrM128i(LoadUnaligned16(&src[0]),
                                           LoadUnaligned16(&src[src_stride]));
        const __m256i result =
            SimpleHorizontalTaps<filter_index>(&src_long, v_tap);
        const __m256i src_long2 = SetrM128i(
            LoadUnaligned16(&src[8]), LoadUnaligned16(&src[8 + src_stride]));
        const __m256i result2 =
            SimpleHorizontalTaps<filter_index>(&src_long2, v_tap);
        const __m256i packed_result = _mm256_unpacklo_epi64(result, result2);
        StoreUnaligned16(&dest8[0], _mm256_castsi256_si128(packed_result));
        StoreUnaligned16(&dest8[pred_stride],
                         _mm256_extracti128_si256(packed_result, 1));
      }
      src += src_stride * 2;
      dest8 += pred_stride * 2;
      dest16 += pred_stride * 2;
      y -= 2;
    } while (y != 0);

    // The 2d filters have an odd |height| during the horizontal pass, so
    // filter the remaining row.
    if (is_2d) {
      const __m256i src_long =
          SetrM128i(LoadUnaligned16(&src[0]), LoadUnaligned16(&src[8]));
      const __m256i result =
          HorizontalTaps8To16<filter_index>(&src_long, v_tap);
      StoreAligned32(&dest16[0], result);
    }

  } else if (width == 8) {
    int y = height;
    if (is_2d) y -= 1;
    do {
      // Load into 2 128 bit lanes.
      const __m128i this_row = LoadUnaligned16(&src[0]);
      const __m128i next_row = LoadUnaligned16(&src[src_stride]);
      const __m256i src_long = SetrM128i(this_row, next_row);
      if (is_2d || is_compound) {
        const __m256i result =
            HorizontalTaps8To16<filter_index>(&src_long, v_tap);
        if (is_2d) {
          StoreAligned16(&dest16[0], _mm256_castsi256_si128(result));
          StoreAligned16(&dest16[pred_stride],
                         _mm256_extracti128_si256(result, 1));
        } else {
          StoreUnaligned16(&dest16[0], _mm256_castsi256_si128(result));
          StoreUnaligned16(&dest16[pred_stride],
                           _mm256_extracti128_si256(result, 1));
        }
      } else {
        const __m128i this_row = LoadUnaligned16(&src[0]);
        const __m128i next_row = LoadUnaligned16(&src[src_stride]);
        // Load into 2 128 bit lanes.
        const __m256i src_long = SetrM128i(this_row, next_row);
        const __m256i result =
            SimpleHorizontalTaps<filter_index>(&src_long, v_tap);
        StoreLo8(&dest8[0], _mm256_castsi256_si128(result));
        StoreLo8(&dest8[pred_stride], _mm256_extracti128_si256(result, 1));
      }
      src += src_stride * 2;
      dest8 += pred_stride * 2;
      dest16 += pred_stride * 2;
      y -= 2;
    } while (y != 0);

    // The 2d filters have an odd |height| during the horizontal pass, so
    // filter the remaining row.
    if (is_2d) {
      const __m256i src_long = _mm256_castsi128_si256(LoadUnaligned16(&src[0]));
      const __m256i result =
          HorizontalTaps8To16<filter_index>(&src_long, v_tap);
      StoreAligned16(&dest16[0], _mm256_castsi256_si128(result));
    }

  } else {  // width == 4
    int y = height;
    if (is_2d) y -= 1;
    do {
      // Load into 2 128 bit lanes.
      const __m128i this_row = LoadUnaligned16(&src[0]);
      const __m128i next_row = LoadUnaligned16(&src[src_stride]);
      const __m256i src_long = SetrM128i(this_row, next_row);
      if (is_2d || is_compound) {
        const __m256i result =
            HorizontalTaps8To16<filter_index>(&src_long, v_tap);
        StoreLo8(&dest16[0], _mm256_castsi256_si128(result));
        StoreLo8(&dest16[pred_stride], _mm256_extracti128_si256(result, 1));
      } else {
        const __m128i this_row = LoadUnaligned16(&src[0]);
        const __m128i next_row = LoadUnaligned16(&src[src_stride]);
        // Load into 2 128 bit lanes.
        const __m256i src_long = SetrM128i(this_row, next_row);
        const __m256i result =
            SimpleHorizontalTaps<filter_index>(&src_long, v_tap);
        Store4(&dest8[0], _mm256_castsi256_si128(result));
        Store4(&dest8[pred_stride], _mm256_extracti128_si256(result, 1));
      }
      src += src_stride * 2;
      dest8 += pred_stride * 2;
      dest16 += pred_stride * 2;
      y -= 2;
    } while (y != 0);

    // The 2d filters have an odd |height| during the horizontal pass, so
    // filter the remaining row.
    if (is_2d) {
      const __m256i src_long = _mm256_castsi128_si256(LoadUnaligned16(&src[0]));
      const __m256i result =
          HorizontalTaps8To16<filter_index>(&src_long, v_tap);
      StoreLo8(&dest16[0], _mm256_castsi256_si128(result));
    }
  }
}

template <int num_taps, bool is_2d_vertical = false>
LIBGAV1_ALWAYS_INLINE void SetupTaps(const __m128i* const filter,
                                     __m128i* v_tap) {
  if (num_taps == 8) {
    v_tap[0] = _mm_shufflelo_epi16(*filter, 0x0);   // k1k0
    v_tap[1] = _mm_shufflelo_epi16(*filter, 0x55);  // k3k2
    v_tap[2] = _mm_shufflelo_epi16(*filter, 0xaa);  // k5k4
    v_tap[3] = _mm_shufflelo_epi16(*filter, 0xff);  // k7k6
    if (is_2d_vertical) {
      v_tap[0] = _mm_cvtepi8_epi16(v_tap[0]);
      v_tap[1] = _mm_cvtepi8_epi16(v_tap[1]);
      v_tap[2] = _mm_cvtepi8_epi16(v_tap[2]);
      v_tap[3] = _mm_cvtepi8_epi16(v_tap[3]);
    } else {
      v_tap[0] = _mm_unpacklo_epi64(v_tap[0], v_tap[0]);
      v_tap[1] = _mm_unpacklo_epi64(v_tap[1], v_tap[1]);
      v_tap[2] = _mm_unpacklo_epi64(v_tap[2], v_tap[2]);
      v_tap[3] = _mm_unpacklo_epi64(v_tap[3], v_tap[3]);
    }
  } else if (num_taps == 6) {
    const __m128i adjusted_filter = _mm_srli_si128(*filter, 1);
    v_tap[0] = _mm_shufflelo_epi16(adjusted_filter, 0x0);   // k2k1
    v_tap[1] = _mm_shufflelo_epi16(adjusted_filter, 0x55);  // k4k3
    v_tap[2] = _mm_shufflelo_epi16(adjusted_filter, 0xaa);  // k6k5
    if (is_2d_vertical) {
      v_tap[0] = _mm_cvtepi8_epi16(v_tap[0]);
      v_tap[1] = _mm_cvtepi8_epi16(v_tap[1]);
      v_tap[2] = _mm_cvtepi8_epi16(v_tap[2]);
    } else {
      v_tap[0] = _mm_unpacklo_epi64(v_tap[0], v_tap[0]);
      v_tap[1] = _mm_unpacklo_epi64(v_tap[1], v_tap[1]);
      v_tap[2] = _mm_unpacklo_epi64(v_tap[2], v_tap[2]);
    }
  } else if (num_taps == 4) {
    v_tap[0] = _mm_shufflelo_epi16(*filter, 0x55);  // k3k2
    v_tap[1] = _mm_shufflelo_epi16(*filter, 0xaa);  // k5k4
    if (is_2d_vertical) {
      v_tap[0] = _mm_cvtepi8_epi16(v_tap[0]);
      v_tap[1] = _mm_cvtepi8_epi16(v_tap[1]);
    } else {
      v_tap[0] = _mm_unpacklo_epi64(v_tap[0], v_tap[0]);
      v_tap[1] = _mm_unpacklo_epi64(v_tap[1], v_tap[1]);
    }
  } else {  // num_taps == 2
    const __m128i adjusted_filter = _mm_srli_si128(*filter, 1);
    v_tap[0] = _mm_shufflelo_epi16(adjusted_filter, 0x55);  // k4k3
    if (is_2d_vertical) {
      v_tap[0] = _mm_cvtepi8_epi16(v_tap[0]);
    } else {
      v_tap[0] = _mm_unpacklo_epi64(v_tap[0], v_tap[0]);
    }
  }
}

template <int num_taps, bool is_2d_vertical = false>
LIBGAV1_ALWAYS_INLINE void SetupTaps(const __m128i* const filter,
                                     __m256i* v_tap) {
  if (num_taps == 8) {
    if (is_2d_vertical) {
      v_tap[0] = _mm256_broadcastd_epi32(*filter);                      // k1k0
      v_tap[1] = _mm256_broadcastd_epi32(_mm_srli_si128(*filter, 4));   // k3k2
      v_tap[2] = _mm256_broadcastd_epi32(_mm_srli_si128(*filter, 8));   // k5k4
      v_tap[3] = _mm256_broadcastd_epi32(_mm_srli_si128(*filter, 12));  // k7k6
    } else {
      v_tap[0] = _mm256_broadcastw_epi16(*filter);                     // k1k0
      v_tap[1] = _mm256_broadcastw_epi16(_mm_srli_si128(*filter, 2));  // k3k2
      v_tap[2] = _mm256_broadcastw_epi16(_mm_srli_si128(*filter, 4));  // k5k4
      v_tap[3] = _mm256_broadcastw_epi16(_mm_srli_si128(*filter, 6));  // k7k6
    }
  } else if (num_taps == 6) {
    if (is_2d_vertical) {
      v_tap[0] = _mm256_broadcastd_epi32(_mm_srli_si128(*filter, 2));   // k2k1
      v_tap[1] = _mm256_broadcastd_epi32(_mm_srli_si128(*filter, 6));   // k4k3
      v_tap[2] = _mm256_broadcastd_epi32(_mm_srli_si128(*filter, 10));  // k6k5
    } else {
      v_tap[0] = _mm256_broadcastw_epi16(_mm_srli_si128(*filter, 1));  // k2k1
      v_tap[1] = _mm256_broadcastw_epi16(_mm_srli_si128(*filter, 3));  // k4k3
      v_tap[2] = _mm256_broadcastw_epi16(_mm_srli_si128(*filter, 5));  // k6k5
    }
  } else if (num_taps == 4) {
    if (is_2d_vertical) {
      v_tap[0] = _mm256_broadcastd_epi32(_mm_srli_si128(*filter, 4));  // k3k2
      v_tap[1] = _mm256_broadcastd_epi32(_mm_srli_si128(*filter, 8));  // k5k4
    } else {
      v_tap[0] = _mm256_broadcastw_epi16(_mm_srli_si128(*filter, 2));  // k3k2
      v_tap[1] = _mm256_broadcastw_epi16(_mm_srli_si128(*filter, 4));  // k5k4
    }
  } else {  // num_taps == 2
    if (is_2d_vertical) {
      v_tap[0] = _mm256_broadcastd_epi32(_mm_srli_si128(*filter, 6));  // k4k3
    } else {
      v_tap[0] = _mm256_broadcastw_epi16(_mm_srli_si128(*filter, 3));  // k4k3
    }
  }
}

template <int num_taps, bool is_compound>
__m128i SimpleSum2DVerticalTaps(const __m128i* const src,
                                const __m128i* const taps) {
  __m128i sum_lo = _mm_madd_epi16(_mm_unpacklo_epi16(src[0], src[1]), taps[0]);
  __m128i sum_hi = _mm_madd_epi16(_mm_unpackhi_epi16(src[0], src[1]), taps[0]);
  if (num_taps >= 4) {
    __m128i madd_lo =
        _mm_madd_epi16(_mm_unpacklo_epi16(src[2], src[3]), taps[1]);
    __m128i madd_hi =
        _mm_madd_epi16(_mm_unpackhi_epi16(src[2], src[3]), taps[1]);
    sum_lo = _mm_add_epi32(sum_lo, madd_lo);
    sum_hi = _mm_add_epi32(sum_hi, madd_hi);
    if (num_taps >= 6) {
      madd_lo = _mm_madd_epi16(_mm_unpacklo_epi16(src[4], src[5]), taps[2]);
      madd_hi = _mm_madd_epi16(_mm_unpackhi_epi16(src[4], src[5]), taps[2]);
      sum_lo = _mm_add_epi32(sum_lo, madd_lo);
      sum_hi = _mm_add_epi32(sum_hi, madd_hi);
      if (num_taps == 8) {
        madd_lo = _mm_madd_epi16(_mm_unpacklo_epi16(src[6], src[7]), taps[3]);
        madd_hi = _mm_madd_epi16(_mm_unpackhi_epi16(src[6], src[7]), taps[3]);
        sum_lo = _mm_add_epi32(sum_lo, madd_lo);
        sum_hi = _mm_add_epi32(sum_hi, madd_hi);
      }
    }
  }

  if (is_compound) {
    return _mm_packs_epi32(
        RightShiftWithRounding_S32(sum_lo, kInterRoundBitsCompoundVertical - 1),
        RightShiftWithRounding_S32(sum_hi,
                                   kInterRoundBitsCompoundVertical - 1));
  }

  return _mm_packs_epi32(
      RightShiftWithRounding_S32(sum_lo, kInterRoundBitsVertical - 1),
      RightShiftWithRounding_S32(sum_hi, kInterRoundBitsVertical - 1));
}

template <int num_taps, bool is_compound>
__m256i SimpleSum2DVerticalTaps(const __m256i* const src,
                                const __m256i* const taps) {
  __m256i sum_lo =
      _mm256_madd_epi16(_mm256_unpacklo_epi16(src[0], src[1]), taps[0]);
  __m256i sum_hi =
      _mm256_madd_epi16(_mm256_unpackhi_epi16(src[0], src[1]), taps[0]);
  if (num_taps >= 4) {
    __m256i madd_lo =
        _mm256_madd_epi16(_mm256_unpacklo_epi16(src[2], src[3]), taps[1]);
    __m256i madd_hi =
        _mm256_madd_epi16(_mm256_unpackhi_epi16(src[2], src[3]), taps[1]);
    sum_lo = _mm256_add_epi32(sum_lo, madd_lo);
    sum_hi = _mm256_add_epi32(sum_hi, madd_hi);
    if (num_taps >= 6) {
      madd_lo =
          _mm256_madd_epi16(_mm256_unpacklo_epi16(src[4], src[5]), taps[2]);
      madd_hi =
          _mm256_madd_epi16(_mm256_unpackhi_epi16(src[4], src[5]), taps[2]);
      sum_lo = _mm256_add_epi32(sum_lo, madd_lo);
      sum_hi = _mm256_add_epi32(sum_hi, madd_hi);
      if (num_taps == 8) {
        madd_lo =
            _mm256_madd_epi16(_mm256_unpacklo_epi16(src[6], src[7]), taps[3]);
        madd_hi =
            _mm256_madd_epi16(_mm256_unpackhi_epi16(src[6], src[7]), taps[3]);
        sum_lo = _mm256_add_epi32(sum_lo, madd_lo);
        sum_hi = _mm256_add_epi32(sum_hi, madd_hi);
      }
    }
  }

  if (is_compound) {
    return _mm256_packs_epi32(
        RightShiftWithRounding_S32(sum_lo, kInterRoundBitsCompoundVertical - 1),
        RightShiftWithRounding_S32(sum_hi,
                                   kInterRoundBitsCompoundVertical - 1));
  }

  return _mm256_packs_epi32(
      RightShiftWithRounding_S32(sum_lo, kInterRoundBitsVertical - 1),
      RightShiftWithRounding_S32(sum_hi, kInterRoundBitsVertical - 1));
}

template <int num_taps, bool is_compound = false>
void Filter2DVertical16xH(const uint16_t* src, void* const dst,
                          const ptrdiff_t dst_stride, const int width,
                          const int height, const __m256i* const taps) {
  assert(width >= 8);
  constexpr int next_row = num_taps - 1;
  // The Horizontal pass uses |width| as |stride| for the intermediate buffer.
  const ptrdiff_t src_stride = width;

  auto* dst8 = static_cast<uint8_t*>(dst);
  auto* dst16 = static_cast<uint16_t*>(dst);

  int x = 0;
  do {
    __m256i srcs[8];
    const uint16_t* src_x = src + x;
    srcs[0] = LoadAligned32(src_x);
    src_x += src_stride;
    if (num_taps >= 4) {
      srcs[1] = LoadAligned32(src_x);
      src_x += src_stride;
      srcs[2] = LoadAligned32(src_x);
      src_x += src_stride;
      if (num_taps >= 6) {
        srcs[3] = LoadAligned32(src_x);
        src_x += src_stride;
        srcs[4] = LoadAligned32(src_x);
        src_x += src_stride;
        if (num_taps == 8) {
          srcs[5] = LoadAligned32(src_x);
          src_x += src_stride;
          srcs[6] = LoadAligned32(src_x);
          src_x += src_stride;
        }
      }
    }

    auto* dst8_x = dst8 + x;
    auto* dst16_x = dst16 + x;
    int y = height;
    do {
      srcs[next_row] = LoadAligned32(src_x);
      src_x += src_stride;

      const __m256i sum =
          SimpleSum2DVerticalTaps<num_taps, is_compound>(srcs, taps);
      if (is_compound) {
        StoreUnaligned32(dst16_x, sum);
        dst16_x += dst_stride;
      } else {
        const __m128i packed_sum = _mm_packus_epi16(
            _mm256_castsi256_si128(sum), _mm256_extracti128_si256(sum, 1));
        StoreUnaligned16(dst8_x, packed_sum);
        dst8_x += dst_stride;
      }

      srcs[0] = srcs[1];
      if (num_taps >= 4) {
        srcs[1] = srcs[2];
        srcs[2] = srcs[3];
        if (num_taps >= 6) {
          srcs[3] = srcs[4];
          srcs[4] = srcs[5];
          if (num_taps == 8) {
            srcs[5] = srcs[6];
            srcs[6] = srcs[7];
          }
        }
      }
    } while (--y != 0);
    x += 16;
  } while (x < width);
}

template <int num_taps, bool is_compound = false>
void Filter2DVertical(const uint16_t* src, void* const dst,
                      const ptrdiff_t dst_stride, const int width,
                      const int height, const __m128i* const taps) {
  assert(width >= 8);
  constexpr int next_row = num_taps - 1;
  // The Horizontal pass uses |width| as |stride| for the intermediate buffer.
  const ptrdiff_t src_stride = width;

  auto* dst8 = static_cast<uint8_t*>(dst);
  auto* dst16 = static_cast<uint16_t*>(dst);

  int x = 0;
  do {
    __m128i srcs[8];
    const uint16_t* src_x = src + x;
    srcs[0] = LoadAligned16(src_x);
    src_x += src_stride;
    if (num_taps >= 4) {
      srcs[1] = LoadAligned16(src_x);
      src_x += src_stride;
      srcs[2] = LoadAligned16(src_x);
      src_x += src_stride;
      if (num_taps >= 6) {
        srcs[3] = LoadAligned16(src_x);
        src_x += src_stride;
        srcs[4] = LoadAligned16(src_x);
        src_x += src_stride;
        if (num_taps == 8) {
          srcs[5] = LoadAligned16(src_x);
          src_x += src_stride;
          srcs[6] = LoadAligned16(src_x);
          src_x += src_stride;
        }
      }
    }

    int y = 0;
    do {
      srcs[next_row] = LoadAligned16(src_x);
      src_x += src_stride;

      const __m128i sum =
          SimpleSum2DVerticalTaps<num_taps, is_compound>(srcs, taps);
      if (is_compound) {
        StoreUnaligned16(dst16 + x + y * dst_stride, sum);
      } else {
        StoreLo8(dst8 + x + y * dst_stride, _mm_packus_epi16(sum, sum));
      }

      srcs[0] = srcs[1];
      if (num_taps >= 4) {
        srcs[1] = srcs[2];
        srcs[2] = srcs[3];
        if (num_taps >= 6) {
          srcs[3] = srcs[4];
          srcs[4] = srcs[5];
          if (num_taps == 8) {
            srcs[5] = srcs[6];
            srcs[6] = srcs[7];
          }
        }
      }
    } while (++y < height);
    x += 8;
  } while (x < width);
}

// Take advantage of |src_stride| == |width| to process two rows at a time.
template <int num_taps, bool is_compound = false>
void Filter2DVertical4xH(const uint16_t* src, void* const dst,
                         const ptrdiff_t dst_stride, const int height,
                         const __m128i* const taps) {
  auto* dst8 = static_cast<uint8_t*>(dst);
  auto* dst16 = static_cast<uint16_t*>(dst);

  __m128i srcs[9];
  srcs[0] = LoadAligned16(src);
  src += 8;
  if (num_taps >= 4) {
    srcs[2] = LoadAligned16(src);
    src += 8;
    srcs[1] = _mm_unpacklo_epi64(_mm_srli_si128(srcs[0], 8), srcs[2]);
    if (num_taps >= 6) {
      srcs[4] = LoadAligned16(src);
      src += 8;
      srcs[3] = _mm_unpacklo_epi64(_mm_srli_si128(srcs[2], 8), srcs[4]);
      if (num_taps == 8) {
        srcs[6] = LoadAligned16(src);
        src += 8;
        srcs[5] = _mm_unpacklo_epi64(_mm_srli_si128(srcs[4], 8), srcs[6]);
      }
    }
  }

  int y = 0;
  do {
    srcs[num_taps] = LoadAligned16(src);
    src += 8;
    srcs[num_taps - 1] = _mm_unpacklo_epi64(
        _mm_srli_si128(srcs[num_taps - 2], 8), srcs[num_taps]);

    const __m128i sum =
        SimpleSum2DVerticalTaps<num_taps, is_compound>(srcs, taps);
    if (is_compound) {
      StoreUnaligned16(dst16, sum);
      dst16 += 4 << 1;
    } else {
      const __m128i results = _mm_packus_epi16(sum, sum);
      Store4(dst8, results);
      dst8 += dst_stride;
      Store4(dst8, _mm_srli_si128(results, 4));
      dst8 += dst_stride;
    }

    srcs[0] = srcs[2];
    if (num_taps >= 4) {
      srcs[1] = srcs[3];
      srcs[2] = srcs[4];
      if (num_taps >= 6) {
        srcs[3] = srcs[5];
        srcs[4] = srcs[6];
        if (num_taps == 8) {
          srcs[5] = srcs[7];
          srcs[6] = srcs[8];
        }
      }
    }
    y += 2;
  } while (y < height);
}

// Take advantage of |src_stride| == |width| to process four rows at a time.
template <int num_taps>
void Filter2DVertical2xH(const uint16_t* src, void* const dst,
                         const ptrdiff_t dst_stride, const int height,
                         const __m128i* const taps) {
  constexpr int next_row = (num_taps < 6) ? 4 : 8;

  auto* dst8 = static_cast<uint8_t*>(dst);

  __m128i srcs[9];
  srcs[0] = LoadAligned16(src);
  src += 8;
  if (num_taps >= 6) {
    srcs[4] = LoadAligned16(src);
    src += 8;
    srcs[1] = _mm_alignr_epi8(srcs[4], srcs[0], 4);
    if (num_taps == 8) {
      srcs[2] = _mm_alignr_epi8(srcs[4], srcs[0], 8);
      srcs[3] = _mm_alignr_epi8(srcs[4], srcs[0], 12);
    }
  }

  int y = 0;
  do {
    srcs[next_row] = LoadAligned16(src);
    src += 8;
    if (num_taps == 2) {
      srcs[1] = _mm_alignr_epi8(srcs[4], srcs[0], 4);
    } else if (num_taps == 4) {
      srcs[1] = _mm_alignr_epi8(srcs[4], srcs[0], 4);
      srcs[2] = _mm_alignr_epi8(srcs[4], srcs[0], 8);
      srcs[3] = _mm_alignr_epi8(srcs[4], srcs[0], 12);
    } else if (num_taps == 6) {
      srcs[2] = _mm_alignr_epi8(srcs[4], srcs[0], 8);
      srcs[3] = _mm_alignr_epi8(srcs[4], srcs[0], 12);
      srcs[5] = _mm_alignr_epi8(srcs[8], srcs[4], 4);
    } else if (num_taps == 8) {
      srcs[5] = _mm_alignr_epi8(srcs[8], srcs[4], 4);
      srcs[6] = _mm_alignr_epi8(srcs[8], srcs[4], 8);
      srcs[7] = _mm_alignr_epi8(srcs[8], srcs[4], 12);
    }

    const __m128i sum =
        SimpleSum2DVerticalTaps<num_taps, /*is_compound=*/false>(srcs, taps);
    const __m128i results = _mm_packus_epi16(sum, sum);

    Store2(dst8, results);
    dst8 += dst_stride;
    Store2(dst8, _mm_srli_si128(results, 2));
    // When |height| <= 4 the taps are restricted to 2 and 4 tap variants.
    // Therefore we don't need to check this condition when |height| > 4.
    if (num_taps <= 4 && height == 2) return;
    dst8 += dst_stride;
    Store2(dst8, _mm_srli_si128(results, 4));
    dst8 += dst_stride;
    Store2(dst8, _mm_srli_si128(results, 6));
    dst8 += dst_stride;

    srcs[0] = srcs[4];
    if (num_taps == 6) {
      srcs[1] = srcs[5];
      srcs[4] = srcs[8];
    } else if (num_taps == 8) {
      srcs[1] = srcs[5];
      srcs[2] = srcs[6];
      srcs[3] = srcs[7];
      srcs[4] = srcs[8];
    }

    y += 4;
  } while (y < height);
}

template <bool is_2d = false, bool is_compound = false>
LIBGAV1_ALWAYS_INLINE void DoHorizontalPass2xH(
    const uint8_t* const src, const ptrdiff_t src_stride, void* const dst,
    const ptrdiff_t dst_stride, const int width, const int height,
    const int filter_id, const int filter_index) {
  assert(filter_id != 0);
  __m128i v_tap[4];
  const __m128i v_horizontal_filter =
      LoadLo8(kHalfSubPixelFilters[filter_index][filter_id]);

  if (filter_index == 4) {  // 4 tap.
    SetupTaps<4>(&v_horizontal_filter, v_tap);
    FilterHorizontal<4, 8, 4, is_2d, is_compound>(
        src, src_stride, dst, dst_stride, width, height, v_tap);
  } else if (filter_index == 5) {  // 4 tap.
    SetupTaps<4>(&v_horizontal_filter, v_tap);
    FilterHorizontal<4, 8, 5, is_2d, is_compound>(
        src, src_stride, dst, dst_stride, width, height, v_tap);
  } else {  // 2 tap.
    SetupTaps<2>(&v_horizontal_filter, v_tap);
    FilterHorizontal<2, 8, 3, is_2d, is_compound>(
        src, src_stride, dst, dst_stride, width, height, v_tap);
  }
}

template <bool is_2d = false, bool is_compound = false>
LIBGAV1_ALWAYS_INLINE void DoHorizontalPass(
    const uint8_t* const src, const ptrdiff_t src_stride, void* const dst,
    const ptrdiff_t dst_stride, const int width, const int height,
    const int filter_id, const int filter_index) {
  assert(filter_id != 0);
  __m256i v_tap[4];
  const __m128i v_horizontal_filter =
      LoadLo8(kHalfSubPixelFilters[filter_index][filter_id]);

  if (filter_index == 2) {  // 8 tap.
    SetupTaps<8>(&v_horizontal_filter, v_tap);
    FilterHorizontal<8, 8, 2, is_2d, is_compound>(
        src, src_stride, dst, dst_stride, width, height, v_tap);
  } else if (filter_index == 1) {  // 6 tap.
    SetupTaps<6>(&v_horizontal_filter, v_tap);
    FilterHorizontal<6, 8, 1, is_2d, is_compound>(
        src, src_stride, dst, dst_stride, width, height, v_tap);
  } else if (filter_index == 0) {  // 6 tap.
    SetupTaps<6>(&v_horizontal_filter, v_tap);
    FilterHorizontal<6, 8, 0, is_2d, is_compound>(
        src, src_stride, dst, dst_stride, width, height, v_tap);
  } else if (filter_index == 4) {  // 4 tap.
    SetupTaps<4>(&v_horizontal_filter, v_tap);
    FilterHorizontal<4, 8, 4, is_2d, is_compound>(
        src, src_stride, dst, dst_stride, width, height, v_tap);
  } else if (filter_index == 5) {  // 4 tap.
    SetupTaps<4>(&v_horizontal_filter, v_tap);
    FilterHorizontal<4, 8, 5, is_2d, is_compound>(
        src, src_stride, dst, dst_stride, width, height, v_tap);
  } else {  // 2 tap.
    SetupTaps<2>(&v_horizontal_filter, v_tap);
    FilterHorizontal<2, 8, 3, is_2d, is_compound>(
        src, src_stride, dst, dst_stride, width, height, v_tap);
  }
}

void Convolve2D_AVX2(const void* const reference,
                     const ptrdiff_t reference_stride,
                     const int horizontal_filter_index,
                     const int vertical_filter_index,
                     const int horizontal_filter_id,
                     const int vertical_filter_id, const int width,
                     const int height, void* prediction,
                     const ptrdiff_t pred_stride) {
  const int horiz_filter_index = GetFilterIndex(horizontal_filter_index, width);
  const int vert_filter_index = GetFilterIndex(vertical_filter_index, height);
  const int vertical_taps = GetNumTapsInFilter(vert_filter_index);

  // The output of the horizontal filter is guaranteed to fit in 16 bits.
  alignas(32) uint16_t
      intermediate_result[kMaxSuperBlockSizeInPixels *
                          (kMaxSuperBlockSizeInPixels + kSubPixelTaps - 1)];
  const int intermediate_height = height + vertical_taps - 1;

  const ptrdiff_t src_stride = reference_stride;
  const auto* src = static_cast<const uint8_t*>(reference) -
                    (vertical_taps / 2 - 1) * src_stride - kHorizontalOffset;
  if (width > 2) {
    DoHorizontalPass</*is_2d=*/true>(src, src_stride, intermediate_result,
                                     width, width, intermediate_height,
                                     horizontal_filter_id, horiz_filter_index);
  } else {
    // Use non avx2 version for smaller widths.
    DoHorizontalPass2xH</*is_2d=*/true>(
        src, src_stride, intermediate_result, width, width, intermediate_height,
        horizontal_filter_id, horiz_filter_index);
  }

  // Vertical filter.
  auto* dest = static_cast<uint8_t*>(prediction);
  const ptrdiff_t dest_stride = pred_stride;
  assert(vertical_filter_id != 0);

  const __m128i v_filter =
      LoadLo8(kHalfSubPixelFilters[vert_filter_index][vertical_filter_id]);

  // Use 256 bits for width > 8.
  if (width > 8) {
    __m256i taps_256[4];
    const __m128i v_filter_ext = _mm_cvtepi8_epi16(v_filter);

    if (vertical_taps == 8) {
      SetupTaps<8, /*is_2d_vertical=*/true>(&v_filter_ext, taps_256);
      Filter2DVertical16xH<8>(intermediate_result, dest, dest_stride, width,
                              height, taps_256);
    } else if (vertical_taps == 6) {
      SetupTaps<6, /*is_2d_vertical=*/true>(&v_filter_ext, taps_256);
      Filter2DVertical16xH<6>(intermediate_result, dest, dest_stride, width,
                              height, taps_256);
    } else if (vertical_taps == 4) {
      SetupTaps<4, /*is_2d_vertical=*/true>(&v_filter_ext, taps_256);
      Filter2DVertical16xH<4>(intermediate_result, dest, dest_stride, width,
                              height, taps_256);
    } else {  // |vertical_taps| == 2
      SetupTaps<2, /*is_2d_vertical=*/true>(&v_filter_ext, taps_256);
      Filter2DVertical16xH<2>(intermediate_result, dest, dest_stride, width,
                              height, taps_256);
    }
  } else {  // width <= 8
    __m128i taps[4];
    // Use 128 bit code.  Copied from convolve_sse4.cc.
    // TODO(slavarnway): Move sse4 code to convolve_sse4.inc.
    if (vertical_taps == 8) {
      SetupTaps<8, /*is_2d_vertical=*/true>(&v_filter, taps);
      if (width == 2) {
        Filter2DVertical2xH<8>(intermediate_result, dest, dest_stride, height,
                               taps);
      } else if (width == 4) {
        Filter2DVertical4xH<8>(intermediate_result, dest, dest_stride, height,
                               taps);
      } else {
        Filter2DVertical<8>(intermediate_result, dest, dest_stride, width,
                            height, taps);
      }
    } else if (vertical_taps == 6) {
      SetupTaps<6, /*is_2d_vertical=*/true>(&v_filter, taps);
      if (width == 2) {
        Filter2DVertical2xH<6>(intermediate_result, dest, dest_stride, height,
                               taps);
      } else if (width == 4) {
        Filter2DVertical4xH<6>(intermediate_result, dest, dest_stride, height,
                               taps);
      } else {
        Filter2DVertical<6>(intermediate_result, dest, dest_stride, width,
                            height, taps);
      }
    } else if (vertical_taps == 4) {
      SetupTaps<4, /*is_2d_vertical=*/true>(&v_filter, taps);
      if (width == 2) {
        Filter2DVertical2xH<4>(intermediate_result, dest, dest_stride, height,
                               taps);
      } else if (width == 4) {
        Filter2DVertical4xH<4>(intermediate_result, dest, dest_stride, height,
                               taps);
      } else {
        Filter2DVertical<4>(intermediate_result, dest, dest_stride, width,
                            height, taps);
      }
    } else {  // |vertical_taps| == 2
      SetupTaps<2, /*is_2d_vertical=*/true>(&v_filter, taps);
      if (width == 2) {
        Filter2DVertical2xH<2>(intermediate_result, dest, dest_stride, height,
                               taps);
      } else if (width == 4) {
        Filter2DVertical4xH<2>(intermediate_result, dest, dest_stride, height,
                               taps);
      } else {
        Filter2DVertical<2>(intermediate_result, dest, dest_stride, width,
                            height, taps);
      }
    }
  }
}

// The 1D compound shift is always |kInterRoundBitsHorizontal|, even for 1D
// Vertical calculations.
__m128i Compound1DShift(const __m128i sum) {
  return RightShiftWithRounding_S16(sum, kInterRoundBitsHorizontal - 1);
}

__m256i Compound1DShift(const __m256i sum) {
  return RightShiftWithRounding_S16(sum, kInterRoundBitsHorizontal - 1);
}

template <int filter_index>
__m128i SumVerticalTaps(const __m128i* const srcs, const __m128i* const v_tap) {
  __m128i v_src[4];

  if (filter_index < 2) {
    // 6 taps.
    v_src[0] = _mm_unpacklo_epi8(srcs[0], srcs[1]);
    v_src[1] = _mm_unpacklo_epi8(srcs[2], srcs[3]);
    v_src[2] = _mm_unpacklo_epi8(srcs[4], srcs[5]);
  } else if (filter_index == 2) {
    // 8 taps.
    v_src[0] = _mm_unpacklo_epi8(srcs[0], srcs[1]);
    v_src[1] = _mm_unpacklo_epi8(srcs[2], srcs[3]);
    v_src[2] = _mm_unpacklo_epi8(srcs[4], srcs[5]);
    v_src[3] = _mm_unpacklo_epi8(srcs[6], srcs[7]);
  } else if (filter_index == 3) {
    // 2 taps.
    v_src[0] = _mm_unpacklo_epi8(srcs[0], srcs[1]);
  } else if (filter_index > 3) {
    // 4 taps.
    v_src[0] = _mm_unpacklo_epi8(srcs[0], srcs[1]);
    v_src[1] = _mm_unpacklo_epi8(srcs[2], srcs[3]);
  }
  const __m128i sum = SumOnePassTaps<filter_index>(v_src, v_tap);
  return sum;
}

template <int filter_index, bool unpack_high = false>
__m256i SumVerticalTaps(const __m256i* const srcs, const __m256i* const v_tap) {
  __m256i v_src[4];

  if (!unpack_high) {
    if (filter_index < 2) {
      // 6 taps.
      v_src[0] = _mm256_unpacklo_epi8(srcs[0], srcs[1]);
      v_src[1] = _mm256_unpacklo_epi8(srcs[2], srcs[3]);
      v_src[2] = _mm256_unpacklo_epi8(srcs[4], srcs[5]);
    } else if (filter_index == 2) {
      // 8 taps.
      v_src[0] = _mm256_unpacklo_epi8(srcs[0], srcs[1]);
      v_src[1] = _mm256_unpacklo_epi8(srcs[2], srcs[3]);
      v_src[2] = _mm256_unpacklo_epi8(srcs[4], srcs[5]);
      v_src[3] = _mm256_unpacklo_epi8(srcs[6], srcs[7]);
    } else if (filter_index == 3) {
      // 2 taps.
      v_src[0] = _mm256_unpacklo_epi8(srcs[0], srcs[1]);
    } else if (filter_index > 3) {
      // 4 taps.
      v_src[0] = _mm256_unpacklo_epi8(srcs[0], srcs[1]);
      v_src[1] = _mm256_unpacklo_epi8(srcs[2], srcs[3]);
    }
  } else {
    if (filter_index < 2) {
      // 6 taps.
      v_src[0] = _mm256_unpackhi_epi8(srcs[0], srcs[1]);
      v_src[1] = _mm256_unpackhi_epi8(srcs[2], srcs[3]);
      v_src[2] = _mm256_unpackhi_epi8(srcs[4], srcs[5]);
    } else if (filter_index == 2) {
      // 8 taps.
      v_src[0] = _mm256_unpackhi_epi8(srcs[0], srcs[1]);
      v_src[1] = _mm256_unpackhi_epi8(srcs[2], srcs[3]);
      v_src[2] = _mm256_unpackhi_epi8(srcs[4], srcs[5]);
      v_src[3] = _mm256_unpackhi_epi8(srcs[6], srcs[7]);
    } else if (filter_index == 3) {
      // 2 taps.
      v_src[0] = _mm256_unpackhi_epi8(srcs[0], srcs[1]);
    } else if (filter_index > 3) {
      // 4 taps.
      v_src[0] = _mm256_unpackhi_epi8(srcs[0], srcs[1]);
      v_src[1] = _mm256_unpackhi_epi8(srcs[2], srcs[3]);
    }
  }
  return SumOnePassTaps<filter_index>(v_src, v_tap);
}

template <int filter_index, bool is_compound = false>
void FilterVertical32xH(const uint8_t* src, const ptrdiff_t src_stride,
                        void* const dst, const ptrdiff_t dst_stride,
                        const int width, const int height,
                        const __m256i* const v_tap) {
  const int num_taps = GetNumTapsInFilter(filter_index);
  const int next_row = num_taps - 1;
  auto* dst8 = static_cast<uint8_t*>(dst);
  auto* dst16 = static_cast<uint16_t*>(dst);
  assert(width >= 32);
  int x = 0;
  do {
    const uint8_t* src_x = src + x;
    __m256i srcs[8];
    srcs[0] = LoadUnaligned32(src_x);
    src_x += src_stride;
    if (num_taps >= 4) {
      srcs[1] = LoadUnaligned32(src_x);
      src_x += src_stride;
      srcs[2] = LoadUnaligned32(src_x);
      src_x += src_stride;
      if (num_taps >= 6) {
        srcs[3] = LoadUnaligned32(src_x);
        src_x += src_stride;
        srcs[4] = LoadUnaligned32(src_x);
        src_x += src_stride;
        if (num_taps == 8) {
          srcs[5] = LoadUnaligned32(src_x);
          src_x += src_stride;
          srcs[6] = LoadUnaligned32(src_x);
          src_x += src_stride;
        }
      }
    }

    auto* dst8_x = dst8 + x;
    auto* dst16_x = dst16 + x;
    int y = height;
    do {
      srcs[next_row] = LoadUnaligned32(src_x);
      src_x += src_stride;

      const __m256i sums = SumVerticalTaps<filter_index>(srcs, v_tap);
      const __m256i sums_hi =
          SumVerticalTaps<filter_index, /*unpack_high=*/true>(srcs, v_tap);
      if (is_compound) {
        const __m256i results =
            Compound1DShift(_mm256_permute2x128_si256(sums, sums_hi, 0x20));
        const __m256i results_hi =
            Compound1DShift(_mm256_permute2x128_si256(sums, sums_hi, 0x31));
        StoreUnaligned32(dst16_x, results);
        StoreUnaligned32(dst16_x + 16, results_hi);
        dst16_x += dst_stride;
      } else {
        const __m256i results =
            RightShiftWithRounding_S16(sums, kFilterBits - 1);
        const __m256i results_hi =
            RightShiftWithRounding_S16(sums_hi, kFilterBits - 1);
        const __m256i packed_results = _mm256_packus_epi16(results, results_hi);

        StoreUnaligned32(dst8_x, packed_results);
        dst8_x += dst_stride;
      }

      srcs[0] = srcs[1];
      if (num_taps >= 4) {
        srcs[1] = srcs[2];
        srcs[2] = srcs[3];
        if (num_taps >= 6) {
          srcs[3] = srcs[4];
          srcs[4] = srcs[5];
          if (num_taps == 8) {
            srcs[5] = srcs[6];
            srcs[6] = srcs[7];
          }
        }
      }
    } while (--y != 0);
    x += 32;
  } while (x < width);
}

template <int filter_index, bool is_compound = false>
void FilterVertical16xH(const uint8_t* src, const ptrdiff_t src_stride,
                        void* const dst, const ptrdiff_t dst_stride,
                        const int /*width*/, const int height,
                        const __m256i* const v_tap) {
  const int num_taps = GetNumTapsInFilter(filter_index);
  const int next_row = num_taps;
  auto* dst8 = static_cast<uint8_t*>(dst);
  auto* dst16 = static_cast<uint16_t*>(dst);

  const uint8_t* src_x = src;
  __m256i srcs[8 + 1];
  // The upper 128 bits hold the filter data for the next row.
  srcs[0] = _mm256_castsi128_si256(LoadUnaligned16(src_x));
  src_x += src_stride;
  if (num_taps >= 4) {
    srcs[1] = _mm256_castsi128_si256(LoadUnaligned16(src_x));
    src_x += src_stride;
    srcs[0] =
        _mm256_inserti128_si256(srcs[0], _mm256_castsi256_si128(srcs[1]), 1);
    srcs[2] = _mm256_castsi128_si256(LoadUnaligned16(src_x));
    src_x += src_stride;
    srcs[1] =
        _mm256_inserti128_si256(srcs[1], _mm256_castsi256_si128(srcs[2]), 1);
    if (num_taps >= 6) {
      srcs[3] = _mm256_castsi128_si256(LoadUnaligned16(src_x));
      src_x += src_stride;
      srcs[2] =
          _mm256_inserti128_si256(srcs[2], _mm256_castsi256_si128(srcs[3]), 1);
      srcs[4] = _mm256_castsi128_si256(LoadUnaligned16(src_x));
      src_x += src_stride;
      srcs[3] =
          _mm256_inserti128_si256(srcs[3], _mm256_castsi256_si128(srcs[4]), 1);
      if (num_taps == 8) {
        srcs[5] = _mm256_castsi128_si256(LoadUnaligned16(src_x));
        src_x += src_stride;
        srcs[4] = _mm256_inserti128_si256(srcs[4],
                                          _mm256_castsi256_si128(srcs[5]), 1);
        srcs[6] = _mm256_castsi128_si256(LoadUnaligned16(src_x));
        src_x += src_stride;
        srcs[5] = _mm256_inserti128_si256(srcs[5],
                                          _mm256_castsi256_si128(srcs[6]), 1);
      }
    }
  }

  int y = height;
  do {
    srcs[next_row - 1] = _mm256_castsi128_si256(LoadUnaligned16(src_x));
    src_x += src_stride;

    srcs[next_row - 2] = _mm256_inserti128_si256(
        srcs[next_row - 2], _mm256_castsi256_si128(srcs[next_row - 1]), 1);

    srcs[next_row] = _mm256_castsi128_si256(LoadUnaligned16(src_x));
    src_x += src_stride;

    srcs[next_row - 1] = _mm256_inserti128_si256(
        srcs[next_row - 1], _mm256_castsi256_si128(srcs[next_row]), 1);

    const __m256i sums = SumVerticalTaps<filter_index>(srcs, v_tap);
    const __m256i sums_hi =
        SumVerticalTaps<filter_index, /*unpack_high=*/true>(srcs, v_tap);
    if (is_compound) {
      const __m256i results =
          Compound1DShift(_mm256_permute2x128_si256(sums, sums_hi, 0x20));
      const __m256i results_hi =
          Compound1DShift(_mm256_permute2x128_si256(sums, sums_hi, 0x31));

      StoreUnaligned32(dst16, results);
      StoreUnaligned32(dst16 + dst_stride, results_hi);
      dst16 += dst_stride << 1;
    } else {
      const __m256i results = RightShiftWithRounding_S16(sums, kFilterBits - 1);
      const __m256i results_hi =
          RightShiftWithRounding_S16(sums_hi, kFilterBits - 1);
      const __m256i packed_results = _mm256_packus_epi16(results, results_hi);
      const __m128i this_dst = _mm256_castsi256_si128(packed_results);
      const auto next_dst = _mm256_extracti128_si256(packed_results, 1);

      StoreUnaligned16(dst8, this_dst);
      StoreUnaligned16(dst8 + dst_stride, next_dst);
      dst8 += dst_stride << 1;
    }

    srcs[0] = srcs[2];
    if (num_taps >= 4) {
      srcs[1] = srcs[3];
      srcs[2] = srcs[4];
      if (num_taps >= 6) {
        srcs[3] = srcs[5];
        srcs[4] = srcs[6];
        if (num_taps == 8) {
          srcs[5] = srcs[7];
          srcs[6] = srcs[8];
        }
      }
    }
    y -= 2;
  } while (y != 0);
}

template <int filter_index, bool is_compound = false>
void FilterVertical8xH(const uint8_t* src, const ptrdiff_t src_stride,
                       void* const dst, const ptrdiff_t dst_stride,
                       const int /*width*/, const int height,
                       const __m128i* const v_tap) {
  const int num_taps = GetNumTapsInFilter(filter_index);
  const int next_row = num_taps - 1;
  auto* dst8 = static_cast<uint8_t*>(dst);
  auto* dst16 = static_cast<uint16_t*>(dst);

  // TODO(slavarnway): Write 256 bit version, processing two rows per iteration.
  const uint8_t* src_x = src;
  __m128i srcs[8];
  srcs[0] = LoadLo8(src_x);
  src_x += src_stride;
  if (num_taps >= 4) {
    srcs[1] = LoadLo8(src_x);
    src_x += src_stride;
    srcs[2] = LoadLo8(src_x);
    src_x += src_stride;
    if (num_taps >= 6) {
      srcs[3] = LoadLo8(src_x);
      src_x += src_stride;
      srcs[4] = LoadLo8(src_x);
      src_x += src_stride;
      if (num_taps == 8) {
        srcs[5] = LoadLo8(src_x);
        src_x += src_stride;
        srcs[6] = LoadLo8(src_x);
        src_x += src_stride;
      }
    }
  }

  int y = height;
  do {
    srcs[next_row] = LoadLo8(src_x);
    src_x += src_stride;

    const __m128i sums = SumVerticalTaps<filter_index>(srcs, v_tap);
    if (is_compound) {
      const __m128i results = Compound1DShift(sums);
      StoreUnaligned16(dst16, results);
      dst16 += dst_stride;
    } else {
      const __m128i results = RightShiftWithRounding_S16(sums, kFilterBits - 1);
      StoreLo8(dst8, _mm_packus_epi16(results, results));
      dst8 += dst_stride;
    }

    srcs[0] = srcs[1];
    if (num_taps >= 4) {
      srcs[1] = srcs[2];
      srcs[2] = srcs[3];
      if (num_taps >= 6) {
        srcs[3] = srcs[4];
        srcs[4] = srcs[5];
        if (num_taps == 8) {
          srcs[5] = srcs[6];
          srcs[6] = srcs[7];
        }
      }
    }
  } while (--y != 0);
}

template <int filter_index, bool is_compound = false>
void FilterVertical4xH(const uint8_t* src, const ptrdiff_t src_stride,
                       void* const dst, const ptrdiff_t dst_stride,
                       const int height, const __m128i* const v_tap) {
  const int num_taps = GetNumTapsInFilter(filter_index);
  auto* dst8 = static_cast<uint8_t*>(dst);
  auto* dst16 = static_cast<uint16_t*>(dst);

  __m128i srcs[9];

  if (num_taps == 2) {
    srcs[2] = _mm_setzero_si128();
    // 00 01 02 03
    srcs[0] = Load4(src);
    src += src_stride;

    int y = 0;
    do {
      // 10 11 12 13
      const __m128i a = Load4(src);
      // 00 01 02 03 10 11 12 13
      srcs[0] = _mm_unpacklo_epi32(srcs[0], a);
      src += src_stride;
      // 20 21 22 23
      srcs[2] = Load4(src);
      src += src_stride;
      // 10 11 12 13 20 21 22 23
      srcs[1] = _mm_unpacklo_epi32(a, srcs[2]);

      const __m128i sums = SumVerticalTaps<filter_index>(srcs, v_tap);
      if (is_compound) {
        const __m128i results = Compound1DShift(sums);
        StoreUnaligned16(dst16, results);
        dst16 += 4 << 1;
      } else {
        const __m128i results_16 =
            RightShiftWithRounding_S16(sums, kFilterBits - 1);
        const __m128i results = _mm_packus_epi16(results_16, results_16);
        Store4(dst8, results);
        dst8 += dst_stride;
        Store4(dst8, _mm_srli_si128(results, 4));
        dst8 += dst_stride;
      }

      srcs[0] = srcs[2];
      y += 2;
    } while (y < height);
  } else if (num_taps == 4) {
    srcs[4] = _mm_setzero_si128();
    // 00 01 02 03
    srcs[0] = Load4(src);
    src += src_stride;
    // 10 11 12 13
    const __m128i a = Load4(src);
    // 00 01 02 03 10 11 12 13
    srcs[0] = _mm_unpacklo_epi32(srcs[0], a);
    src += src_stride;
    // 20 21 22 23
    srcs[2] = Load4(src);
    src += src_stride;
    // 10 11 12 13 20 21 22 23
    srcs[1] = _mm_unpacklo_epi32(a, srcs[2]);

    int y = 0;
    do {
      // 30 31 32 33
      const __m128i b = Load4(src);
      // 20 21 22 23 30 31 32 33
      srcs[2] = _mm_unpacklo_epi32(srcs[2], b);
      src += src_stride;
      // 40 41 42 43
      srcs[4] = Load4(src);
      src += src_stride;
      // 30 31 32 33 40 41 42 43
      srcs[3] = _mm_unpacklo_epi32(b, srcs[4]);

      const __m128i sums = SumVerticalTaps<filter_index>(srcs, v_tap);
      if (is_compound) {
        const __m128i results = Compound1DShift(sums);
        StoreUnaligned16(dst16, results);
        dst16 += 4 << 1;
      } else {
        const __m128i results_16 =
            RightShiftWithRounding_S16(sums, kFilterBits - 1);
        const __m128i results = _mm_packus_epi16(results_16, results_16);
        Store4(dst8, results);
        dst8 += dst_stride;
        Store4(dst8, _mm_srli_si128(results, 4));
        dst8 += dst_stride;
      }

      srcs[0] = srcs[2];
      srcs[1] = srcs[3];
      srcs[2] = srcs[4];
      y += 2;
    } while (y < height);
  } else if (num_taps == 6) {
    srcs[6] = _mm_setzero_si128();
    // 00 01 02 03
    srcs[0] = Load4(src);
    src += src_stride;
    // 10 11 12 13
    const __m128i a = Load4(src);
    // 00 01 02 03 10 11 12 13
    srcs[0] = _mm_unpacklo_epi32(srcs[0], a);
    src += src_stride;
    // 20 21 22 23
    srcs[2] = Load4(src);
    src += src_stride;
    // 10 11 12 13 20 21 22 23
    srcs[1] = _mm_unpacklo_epi32(a, srcs[2]);
    // 30 31 32 33
    const __m128i b = Load4(src);
    // 20 21 22 23 30 31 32 33
    srcs[2] = _mm_unpacklo_epi32(srcs[2], b);
    src += src_stride;
    // 40 41 42 43
    srcs[4] = Load4(src);
    src += src_stride;
    // 30 31 32 33 40 41 42 43
    srcs[3] = _mm_unpacklo_epi32(b, srcs[4]);

    int y = 0;
    do {
      // 50 51 52 53
      const __m128i c = Load4(src);
      // 40 41 42 43 50 51 52 53
      srcs[4] = _mm_unpacklo_epi32(srcs[4], c);
      src += src_stride;
      // 60 61 62 63
      srcs[6] = Load4(src);
      src += src_stride;
      // 50 51 52 53 60 61 62 63
      srcs[5] = _mm_unpacklo_epi32(c, srcs[6]);

      const __m128i sums = SumVerticalTaps<filter_index>(srcs, v_tap);
      if (is_compound) {
        const __m128i results = Compound1DShift(sums);
        StoreUnaligned16(dst16, results);
        dst16 += 4 << 1;
      } else {
        const __m128i results_16 =
            RightShiftWithRounding_S16(sums, kFilterBits - 1);
        const __m128i results = _mm_packus_epi16(results_16, results_16);
        Store4(dst8, results);
        dst8 += dst_stride;
        Store4(dst8, _mm_srli_si128(results, 4));
        dst8 += dst_stride;
      }

      srcs[0] = srcs[2];
      srcs[1] = srcs[3];
      srcs[2] = srcs[4];
      srcs[3] = srcs[5];
      srcs[4] = srcs[6];
      y += 2;
    } while (y < height);
  } else if (num_taps == 8) {
    srcs[8] = _mm_setzero_si128();
    // 00 01 02 03
    srcs[0] = Load4(src);
    src += src_stride;
    // 10 11 12 13
    const __m128i a = Load4(src);
    // 00 01 02 03 10 11 12 13
    srcs[0] = _mm_unpacklo_epi32(srcs[0], a);
    src += src_stride;
    // 20 21 22 23
    srcs[2] = Load4(src);
    src += src_stride;
    // 10 11 12 13 20 21 22 23
    srcs[1] = _mm_unpacklo_epi32(a, srcs[2]);
    // 30 31 32 33
    const __m128i b = Load4(src);
    // 20 21 22 23 30 31 32 33
    srcs[2] = _mm_unpacklo_epi32(srcs[2], b);
    src += src_stride;
    // 40 41 42 43
    srcs[4] = Load4(src);
    src += src_stride;
    // 30 31 32 33 40 41 42 43
    srcs[3] = _mm_unpacklo_epi32(b, srcs[4]);
    // 50 51 52 53
    const __m128i c = Load4(src);
    // 40 41 42 43 50 51 52 53
    srcs[4] = _mm_unpacklo_epi32(srcs[4], c);
    src += src_stride;
    // 60 61 62 63
    srcs[6] = Load4(src);
    src += src_stride;
    // 50 51 52 53 60 61 62 63
    srcs[5] = _mm_unpacklo_epi32(c, srcs[6]);

    int y = 0;
    do {
      // 70 71 72 73
      const __m128i d = Load4(src);
      // 60 61 62 63 70 71 72 73
      srcs[6] = _mm_unpacklo_epi32(srcs[6], d);
      src += src_stride;
      // 80 81 82 83
      srcs[8] = Load4(src);
      src += src_stride;
      // 70 71 72 73 80 81 82 83
      srcs[7] = _mm_unpacklo_epi32(d, srcs[8]);

      const __m128i sums = SumVerticalTaps<filter_index>(srcs, v_tap);
      if (is_compound) {
        const __m128i results = Compound1DShift(sums);
        StoreUnaligned16(dst16, results);
        dst16 += 4 << 1;
      } else {
        const __m128i results_16 =
            RightShiftWithRounding_S16(sums, kFilterBits - 1);
        const __m128i results = _mm_packus_epi16(results_16, results_16);
        Store4(dst8, results);
        dst8 += dst_stride;
        Store4(dst8, _mm_srli_si128(results, 4));
        dst8 += dst_stride;
      }

      srcs[0] = srcs[2];
      srcs[1] = srcs[3];
      srcs[2] = srcs[4];
      srcs[3] = srcs[5];
      srcs[4] = srcs[6];
      srcs[5] = srcs[7];
      srcs[6] = srcs[8];
      y += 2;
    } while (y < height);
  }
}

template <int filter_index, bool negative_outside_taps = false>
void FilterVertical2xH(const uint8_t* src, const ptrdiff_t src_stride,
                       void* const dst, const ptrdiff_t dst_stride,
                       const int height, const __m128i* const v_tap) {
  const int num_taps = GetNumTapsInFilter(filter_index);
  auto* dst8 = static_cast<uint8_t*>(dst);

  __m128i srcs[9];

  if (num_taps == 2) {
    srcs[2] = _mm_setzero_si128();
    // 00 01
    srcs[0] = Load2(src);
    src += src_stride;

    int y = 0;
    do {
      // 00 01 10 11
      srcs[0] = Load2<1>(src, srcs[0]);
      src += src_stride;
      // 00 01 10 11 20 21
      srcs[0] = Load2<2>(src, srcs[0]);
      src += src_stride;
      // 00 01 10 11 20 21 30 31
      srcs[0] = Load2<3>(src, srcs[0]);
      src += src_stride;
      // 40 41
      srcs[2] = Load2<0>(src, srcs[2]);
      src += src_stride;
      // 00 01 10 11 20 21 30 31 40 41
      const __m128i srcs_0_2 = _mm_unpacklo_epi64(srcs[0], srcs[2]);
      // 10 11 20 21 30 31 40 41
      srcs[1] = _mm_srli_si128(srcs_0_2, 2);
      // This uses srcs[0]..srcs[1].
      const __m128i sums = SumVerticalTaps<filter_index>(srcs, v_tap);
      const __m128i results_16 =
          RightShiftWithRounding_S16(sums, kFilterBits - 1);
      const __m128i results = _mm_packus_epi16(results_16, results_16);

      Store2(dst8, results);
      dst8 += dst_stride;
      Store2(dst8, _mm_srli_si128(results, 2));
      if (height == 2) return;
      dst8 += dst_stride;
      Store2(dst8, _mm_srli_si128(results, 4));
      dst8 += dst_stride;
      Store2(dst8, _mm_srli_si128(results, 6));
      dst8 += dst_stride;

      srcs[0] = srcs[2];
      y += 4;
    } while (y < height);
  } else if (num_taps == 4) {
    srcs[4] = _mm_setzero_si128();

    // 00 01
    srcs[0] = Load2(src);
    src += src_stride;
    // 00 01 10 11
    srcs[0] = Load2<1>(src, srcs[0]);
    src += src_stride;
    // 00 01 10 11 20 21
    srcs[0] = Load2<2>(src, srcs[0]);
    src += src_stride;

    int y = 0;
    do {
      // 00 01 10 11 20 21 30 31
      srcs[0] = Load2<3>(src, srcs[0]);
      src += src_stride;
      // 40 41
      srcs[4] = Load2<0>(src, srcs[4]);
      src += src_stride;
      // 40 41 50 51
      srcs[4] = Load2<1>(src, srcs[4]);
      src += src_stride;
      // 40 41 50 51 60 61
      srcs[4] = Load2<2>(src, srcs[4]);
      src += src_stride;
      // 00 01 10 11 20 21 30 31 40 41 50 51 60 61
      const __m128i srcs_0_4 = _mm_unpacklo_epi64(srcs[0], srcs[4]);
      // 10 11 20 21 30 31 40 41
      srcs[1] = _mm_srli_si128(srcs_0_4, 2);
      // 20 21 30 31 40 41 50 51
      srcs[2] = _mm_srli_si128(srcs_0_4, 4);
      // 30 31 40 41 50 51 60 61
      srcs[3] = _mm_srli_si128(srcs_0_4, 6);

      // This uses srcs[0]..srcs[3].
      const __m128i sums = SumVerticalTaps<filter_index>(srcs, v_tap);
      const __m128i results_16 =
          RightShiftWithRounding_S16(sums, kFilterBits - 1);
      const __m128i results = _mm_packus_epi16(results_16, results_16);

      Store2(dst8, results);
      dst8 += dst_stride;
      Store2(dst8, _mm_srli_si128(results, 2));
      if (height == 2) return;
      dst8 += dst_stride;
      Store2(dst8, _mm_srli_si128(results, 4));
      dst8 += dst_stride;
      Store2(dst8, _mm_srli_si128(results, 6));
      dst8 += dst_stride;

      srcs[0] = srcs[4];
      y += 4;
    } while (y < height);
  } else if (num_taps == 6) {
    // During the vertical pass the number of taps is restricted when
    // |height| <= 4.
    assert(height > 4);
    srcs[8] = _mm_setzero_si128();

    // 00 01
    srcs[0] = Load2(src);
    src += src_stride;
    // 00 01 10 11
    srcs[0] = Load2<1>(src, srcs[0]);
    src += src_stride;
    // 00 01 10 11 20 21
    srcs[0] = Load2<2>(src, srcs[0]);
    src += src_stride;
    // 00 01 10 11 20 21 30 31
    srcs[0] = Load2<3>(src, srcs[0]);
    src += src_stride;
    // 40 41
    srcs[4] = Load2(src);
    src += src_stride;
    // 00 01 10 11 20 21 30 31 40 41 50 51 60 61
    const __m128i srcs_0_4x = _mm_unpacklo_epi64(srcs[0], srcs[4]);
    // 10 11 20 21 30 31 40 41
    srcs[1] = _mm_srli_si128(srcs_0_4x, 2);

    int y = 0;
    do {
      // 40 41 50 51
      srcs[4] = Load2<1>(src, srcs[4]);
      src += src_stride;
      // 40 41 50 51 60 61
      srcs[4] = Load2<2>(src, srcs[4]);
      src += src_stride;
      // 40 41 50 51 60 61 70 71
      srcs[4] = Load2<3>(src, srcs[4]);
      src += src_stride;
      // 80 81
      srcs[8] = Load2<0>(src, srcs[8]);
      src += src_stride;
      // 00 01 10 11 20 21 30 31 40 41 50 51 60 61
      const __m128i srcs_0_4 = _mm_unpacklo_epi64(srcs[0], srcs[4]);
      // 20 21 30 31 40 41 50 51
      srcs[2] = _mm_srli_si128(srcs_0_4, 4);
      // 30 31 40 41 50 51 60 61
      srcs[3] = _mm_srli_si128(srcs_0_4, 6);
      const __m128i srcs_4_8 = _mm_unpacklo_epi64(srcs[4], srcs[8]);
      // 50 51 60 61 70 71 80 81
      srcs[5] = _mm_srli_si128(srcs_4_8, 2);

      // This uses srcs[0]..srcs[5].
      const __m128i sums = SumVerticalTaps<filter_index>(srcs, v_tap);
      const __m128i results_16 =
          RightShiftWithRounding_S16(sums, kFilterBits - 1);
      const __m128i results = _mm_packus_epi16(results_16, results_16);

      Store2(dst8, results);
      dst8 += dst_stride;
      Store2(dst8, _mm_srli_si128(results, 2));
      dst8 += dst_stride;
      Store2(dst8, _mm_srli_si128(results, 4));
      dst8 += dst_stride;
      Store2(dst8, _mm_srli_si128(results, 6));
      dst8 += dst_stride;

      srcs[0] = srcs[4];
      srcs[1] = srcs[5];
      srcs[4] = srcs[8];
      y += 4;
    } while (y < height);
  } else if (num_taps == 8) {
    // During the vertical pass the number of taps is restricted when
    // |height| <= 4.
    assert(height > 4);
    srcs[8] = _mm_setzero_si128();
    // 00 01
    srcs[0] = Load2(src);
    src += src_stride;
    // 00 01 10 11
    srcs[0] = Load2<1>(src, srcs[0]);
    src += src_stride;
    // 00 01 10 11 20 21
    srcs[0] = Load2<2>(src, srcs[0]);
    src += src_stride;
    // 00 01 10 11 20 21 30 31
    srcs[0] = Load2<3>(src, srcs[0]);
    src += src_stride;
    // 40 41
    srcs[4] = Load2(src);
    src += src_stride;
    // 40 41 50 51
    srcs[4] = Load2<1>(src, srcs[4]);
    src += src_stride;
    // 40 41 50 51 60 61
    srcs[4] = Load2<2>(src, srcs[4]);
    src += src_stride;

    // 00 01 10 11 20 21 30 31 40 41 50 51 60 61
    const __m128i srcs_0_4 = _mm_unpacklo_epi64(srcs[0], srcs[4]);
    // 10 11 20 21 30 31 40 41
    srcs[1] = _mm_srli_si128(srcs_0_4, 2);
    // 20 21 30 31 40 41 50 51
    srcs[2] = _mm_srli_si128(srcs_0_4, 4);
    // 30 31 40 41 50 51 60 61
    srcs[3] = _mm_srli_si128(srcs_0_4, 6);

    int y = 0;
    do {
      // 40 41 50 51 60 61 70 71
      srcs[4] = Load2<3>(src, srcs[4]);
      src += src_stride;
      // 80 81
      srcs[8] = Load2<0>(src, srcs[8]);
      src += src_stride;
      // 80 81 90 91
      srcs[8] = Load2<1>(src, srcs[8]);
      src += src_stride;
      // 80 81 90 91 a0 a1
      srcs[8] = Load2<2>(src, srcs[8]);
      src += src_stride;

      // 40 41 50 51 60 61 70 71 80 81 90 91 a0 a1
      const __m128i srcs_4_8 = _mm_unpacklo_epi64(srcs[4], srcs[8]);
      // 50 51 60 61 70 71 80 81
      srcs[5] = _mm_srli_si128(srcs_4_8, 2);
      // 60 61 70 71 80 81 90 91
      srcs[6] = _mm_srli_si128(srcs_4_8, 4);
      // 70 71 80 81 90 91 a0 a1
      srcs[7] = _mm_srli_si128(srcs_4_8, 6);

      // This uses srcs[0]..srcs[7].
      const __m128i sums = SumVerticalTaps<filter_index>(srcs, v_tap);
      const __m128i results_16 =
          RightShiftWithRounding_S16(sums, kFilterBits - 1);
      const __m128i results = _mm_packus_epi16(results_16, results_16);

      Store2(dst8, results);
      dst8 += dst_stride;
      Store2(dst8, _mm_srli_si128(results, 2));
      dst8 += dst_stride;
      Store2(dst8, _mm_srli_si128(results, 4));
      dst8 += dst_stride;
      Store2(dst8, _mm_srli_si128(results, 6));
      dst8 += dst_stride;

      srcs[0] = srcs[4];
      srcs[1] = srcs[5];
      srcs[2] = srcs[6];
      srcs[3] = srcs[7];
      srcs[4] = srcs[8];
      y += 4;
    } while (y < height);
  }
}

void ConvolveVertical_AVX2(const void* const reference,
                           const ptrdiff_t reference_stride,
                           const int /*horizontal_filter_index*/,
                           const int vertical_filter_index,
                           const int /*horizontal_filter_id*/,
                           const int vertical_filter_id, const int width,
                           const int height, void* prediction,
                           const ptrdiff_t pred_stride) {
  const int filter_index = GetFilterIndex(vertical_filter_index, height);
  const int vertical_taps = GetNumTapsInFilter(filter_index);
  const ptrdiff_t src_stride = reference_stride;
  const auto* src = static_cast<const uint8_t*>(reference) -
                    (vertical_taps / 2 - 1) * src_stride;
  auto* dest = static_cast<uint8_t*>(prediction);
  const ptrdiff_t dest_stride = pred_stride;
  assert(vertical_filter_id != 0);

  const __m128i v_filter =
      LoadLo8(kHalfSubPixelFilters[filter_index][vertical_filter_id]);

  // Use 256 bits for width > 8.
  if (width > 8) {
    __m256i taps_256[4];
    if (filter_index < 2) {  // 6 tap.
      SetupTaps<6>(&v_filter, taps_256);
      if (width == 16) {
        FilterVertical16xH<0>(src, src_stride, dest, dest_stride, width, height,
                              taps_256);
      } else {
        FilterVertical32xH<0>(src, src_stride, dest, dest_stride, width, height,
                              taps_256);
      }
    } else if (filter_index == 2) {  // 8 tap.
      SetupTaps<8>(&v_filter, taps_256);
      if (width == 16) {
        FilterVertical16xH<2>(src, src_stride, dest, dest_stride, width, height,
                              taps_256);
      } else {
        FilterVertical32xH<2>(src, src_stride, dest, dest_stride, width, height,
                              taps_256);
      }
    } else if (filter_index == 3) {  // 2 tap.
      SetupTaps<2>(&v_filter, taps_256);
      if (width == 16) {
        FilterVertical16xH<3>(src, src_stride, dest, dest_stride, width, height,
                              taps_256);
      } else {
        FilterVertical32xH<3>(src, src_stride, dest, dest_stride, width, height,
                              taps_256);
      }
    } else if (filter_index == 4) {  // 4 tap.
      SetupTaps<4>(&v_filter, taps_256);
      if (width == 16) {
        FilterVertical16xH<4>(src, src_stride, dest, dest_stride, width, height,
                              taps_256);
      } else {
        FilterVertical32xH<4>(src, src_stride, dest, dest_stride, width, height,
                              taps_256);
      }
    } else {
      SetupTaps<4>(&v_filter, taps_256);
      if (width == 16) {
        FilterVertical16xH<5>(src, src_stride, dest, dest_stride, width, height,
                              taps_256);
      } else {
        FilterVertical32xH<5>(src, src_stride, dest, dest_stride, width, height,
                              taps_256);
      }
    }
  } else {  // width <= 8
    // Use 128 bit code.  Copied from convolve_sse4.cc.
    // TODO(slavarnway): Move sse4 code to convolve_sse4.inc.
    __m128i taps[4];

    if (filter_index < 2) {  // 6 tap.
      SetupTaps<6>(&v_filter, taps);
      if (width == 2) {
        FilterVertical2xH<0>(src, src_stride, dest, dest_stride, height, taps);
      } else if (width == 4) {
        FilterVertical4xH<0>(src, src_stride, dest, dest_stride, height, taps);
      } else {
        FilterVertical8xH<0>(src, src_stride, dest, dest_stride, width, height,
                             taps);
      }
    } else if (filter_index == 2) {  // 8 tap.
      SetupTaps<8>(&v_filter, taps);
      if (width == 2) {
        FilterVertical2xH<2>(src, src_stride, dest, dest_stride, height, taps);
      } else if (width == 4) {
        FilterVertical4xH<2>(src, src_stride, dest, dest_stride, height, taps);
      } else {
        FilterVertical8xH<2>(src, src_stride, dest, dest_stride, width, height,
                             taps);
      }
    } else if (filter_index == 3) {  // 2 tap.
      SetupTaps<2>(&v_filter, taps);
      if (width == 2) {
        FilterVertical2xH<3>(src, src_stride, dest, dest_stride, height, taps);
      } else if (width == 4) {
        FilterVertical4xH<3>(src, src_stride, dest, dest_stride, height, taps);
      } else {
        FilterVertical8xH<3>(src, src_stride, dest, dest_stride, width, height,
                             taps);
      }
    } else if (filter_index == 4) {  // 4 tap.
      SetupTaps<4>(&v_filter, taps);
      if (width == 2) {
        FilterVertical2xH<4>(src, src_stride, dest, dest_stride, height, taps);
      } else if (width == 4) {
        FilterVertical4xH<4>(src, src_stride, dest, dest_stride, height, taps);
      } else {
        FilterVertical8xH<4>(src, src_stride, dest, dest_stride, width, height,
                             taps);
      }
    } else {
      SetupTaps<4>(&v_filter, taps);
      if (width == 2) {
        FilterVertical2xH<5>(src, src_stride, dest, dest_stride, height, taps);
      } else if (width == 4) {
        FilterVertical4xH<5>(src, src_stride, dest, dest_stride, height, taps);
      } else {
        FilterVertical8xH<5>(src, src_stride, dest, dest_stride, width, height,
                             taps);
      }
    }
  }
}

void ConvolveCompoundVertical_AVX2(
    const void* const reference, const ptrdiff_t reference_stride,
    const int /*horizontal_filter_index*/, const int vertical_filter_index,
    const int /*horizontal_filter_id*/, const int vertical_filter_id,
    const int width, const int height, void* prediction,
    const ptrdiff_t /*pred_stride*/) {
  const int filter_index = GetFilterIndex(vertical_filter_index, height);
  const int vertical_taps = GetNumTapsInFilter(filter_index);
  const ptrdiff_t src_stride = reference_stride;
  const auto* src = static_cast<const uint8_t*>(reference) -
                    (vertical_taps / 2 - 1) * src_stride;
  auto* dest = static_cast<uint8_t*>(prediction);
  const ptrdiff_t dest_stride = width;
  assert(vertical_filter_id != 0);

  const __m128i v_filter =
      LoadLo8(kHalfSubPixelFilters[filter_index][vertical_filter_id]);

  // Use 256 bits for width > 8.
  if (width > 8) {
    __m256i taps_256[4];
    if (filter_index < 2) {  // 6 tap.
      SetupTaps<6>(&v_filter, taps_256);
      if (width == 16) {
        FilterVertical16xH<0, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps_256);
      } else {
        FilterVertical32xH<0, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps_256);
      }
    } else if (filter_index == 2) {  // 8 tap.
      SetupTaps<8>(&v_filter, taps_256);
      if (width == 16) {
        FilterVertical16xH<2, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps_256);
      } else {
        FilterVertical32xH<2, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps_256);
      }
    } else if (filter_index == 3) {  // 2 tap.
      SetupTaps<2>(&v_filter, taps_256);
      if (width == 16) {
        FilterVertical16xH<3, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps_256);
      } else {
        FilterVertical32xH<3, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps_256);
      }
    } else if (filter_index == 4) {  // 4 tap.
      SetupTaps<4>(&v_filter, taps_256);
      if (width == 16) {
        FilterVertical16xH<4, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps_256);
      } else {
        FilterVertical32xH<4, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps_256);
      }
    } else {
      SetupTaps<4>(&v_filter, taps_256);
      if (width == 16) {
        FilterVertical16xH<5, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps_256);
      } else {
        FilterVertical32xH<5, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps_256);
      }
    }
  } else {  // width <= 8
    // Use 128 bit code.  Copied from convolve_sse4.cc.
    // TODO(slavarnway): Move sse4 code to convolve_sse4.inc.
    __m128i taps[4];

    if (filter_index < 2) {  // 6 tap.
      SetupTaps<6>(&v_filter, taps);
      if (width == 4) {
        FilterVertical4xH<0, /*is_compound=*/true>(src, src_stride, dest,
                                                   dest_stride, height, taps);
      } else {
        FilterVertical8xH<0, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps);
      }
    } else if (filter_index == 2) {  // 8 tap.
      SetupTaps<8>(&v_filter, taps);
      if (width == 4) {
        FilterVertical4xH<2, /*is_compound=*/true>(src, src_stride, dest,
                                                   dest_stride, height, taps);
      } else {
        FilterVertical8xH<2, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps);
      }
    } else if (filter_index == 3) {  // 2 tap.
      SetupTaps<2>(&v_filter, taps);
      if (width == 4) {
        FilterVertical4xH<3, /*is_compound=*/true>(src, src_stride, dest,
                                                   dest_stride, height, taps);
      } else {
        FilterVertical8xH<3, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps);
      }
    } else if (filter_index == 4) {  // 4 tap.
      SetupTaps<4>(&v_filter, taps);
      if (width == 4) {
        FilterVertical4xH<4, /*is_compound=*/true>(src, src_stride, dest,
                                                   dest_stride, height, taps);
      } else {
        FilterVertical8xH<4, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps);
      }
    } else {
      SetupTaps<4>(&v_filter, taps);
      if (width == 4) {
        FilterVertical4xH<5, /*is_compound=*/true>(src, src_stride, dest,
                                                   dest_stride, height, taps);
      } else {
        FilterVertical8xH<5, /*is_compound=*/true>(
            src, src_stride, dest, dest_stride, width, height, taps);
      }
    }
  }
}

void ConvolveHorizontal_AVX2(const void* const reference,
                             const ptrdiff_t reference_stride,
                             const int horizontal_filter_index,
                             const int /*vertical_filter_index*/,
                             const int horizontal_filter_id,
                             const int /*vertical_filter_id*/, const int width,
                             const int height, void* prediction,
                             const ptrdiff_t pred_stride) {
  const int filter_index = GetFilterIndex(horizontal_filter_index, width);
  // Set |src| to the outermost tap.
  const auto* src = static_cast<const uint8_t*>(reference) - kHorizontalOffset;
  auto* dest = static_cast<uint8_t*>(prediction);

  if (width > 2) {
    DoHorizontalPass(src, reference_stride, dest, pred_stride, width, height,
                     horizontal_filter_id, filter_index);
  } else {
    // Use non avx2 version for smaller widths.
    DoHorizontalPass2xH(src, reference_stride, dest, pred_stride, width, height,
                        horizontal_filter_id, filter_index);
  }
}

void ConvolveCompoundHorizontal_AVX2(
    const void* const reference, const ptrdiff_t reference_stride,
    const int horizontal_filter_index, const int /*vertical_filter_index*/,
    const int horizontal_filter_id, const int /*vertical_filter_id*/,
    const int width, const int height, void* prediction,
    const ptrdiff_t pred_stride) {
  const int filter_index = GetFilterIndex(horizontal_filter_index, width);
  // Set |src| to the outermost tap.
  const auto* src = static_cast<const uint8_t*>(reference) - kHorizontalOffset;
  auto* dest = static_cast<uint8_t*>(prediction);
  // All compound functions output to the predictor buffer with |pred_stride|
  // equal to |width|.
  assert(pred_stride == width);
  // Compound functions start at 4x4.
  assert(width >= 4 && height >= 4);

#ifdef NDEBUG
  // Quiet compiler error.
  (void)pred_stride;
#endif

  DoHorizontalPass</*is_2d=*/false, /*is_compound=*/true>(
      src, reference_stride, dest, width, width, height, horizontal_filter_id,
      filter_index);
}

void Init8bpp() {
  Dsp* const dsp = dsp_internal::GetWritableDspTable(kBitdepth8);
  assert(dsp != nullptr);
  dsp->convolve[0][0][0][1] = ConvolveHorizontal_AVX2;
  dsp->convolve[0][0][1][0] = ConvolveVertical_AVX2;
  dsp->convolve[0][0][1][1] = Convolve2D_AVX2;

  dsp->convolve[0][1][0][1] = ConvolveCompoundHorizontal_AVX2;
  dsp->convolve[0][1][1][0] = ConvolveCompoundVertical_AVX2;
}

}  // namespace
}  // namespace low_bitdepth

void ConvolveInit_AVX2() { low_bitdepth::Init8bpp(); }

}  // namespace dsp
}  // namespace libgav1

#else  // !LIBGAV1_TARGETING_AVX2
namespace libgav1 {
namespace dsp {

void ConvolveInit_AVX2() {}

}  // namespace dsp
}  // namespace libgav1
#endif  // LIBGAV1_TARGETING_AVX2
