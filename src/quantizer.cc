// Copyright 2019 The libgav1 Authors
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

#include "src/quantizer.h"

#include <cassert>
#include <cstdint>

#include "src/utils/common.h"
#include "src/utils/constants.h"

#if LIBGAV1_MAX_BITDEPTH != 8 && LIBGAV1_MAX_BITDEPTH != 10 && \
    LIBGAV1_MAX_BITDEPTH != 12
#error LIBGAV1_MAX_BITDEPTH must be 8, 10 or 12
#endif

namespace libgav1 {
namespace {

// Import all the constants in the anonymous namespace.
#include "src/quantizer_tables.inc"

// Format the kDcLookup and kAcLookup arrays manually for easier comparison
// with the Dc_Qlookup and Ac_Qlookup arrays in Section 7.12.2.

// clang-format off
constexpr int16_t kDcLookup[][256] = {
  // Lookup table for 8 bit.
  {
    4, 8, 8, 9, 10, 11, 12, 12, 13, 14, 15, 16,
    17, 18, 19, 19, 20, 21, 22, 23, 24, 25, 26, 26,
    27, 28, 29, 30, 31, 32, 32, 33, 34, 35, 36, 37,
    38, 38, 39, 40, 41, 42, 43, 43, 44, 45, 46, 47,
    48, 48, 49, 50, 51, 52, 53, 53, 54, 55, 56, 57,
    57, 58, 59, 60, 61, 62, 62, 63, 64, 65, 66, 66,
    67, 68, 69, 70, 70, 71, 72, 73, 74, 74, 75, 76,
    77, 78, 78, 79, 80, 81, 81, 82, 83, 84, 85, 85,
    87, 88, 90, 92, 93, 95, 96, 98, 99, 101, 102, 104,
    105, 107, 108, 110, 111, 113, 114, 116, 117, 118, 120, 121,
    123, 125, 127, 129, 131, 134, 136, 138, 140, 142, 144, 146,
    148, 150, 152, 154, 156, 158, 161, 164, 166, 169, 172, 174,
    177, 180, 182, 185, 187, 190, 192, 195, 199, 202, 205, 208,
    211, 214, 217, 220, 223, 226, 230, 233, 237, 240, 243, 247,
    250, 253, 257, 261, 265, 269, 272, 276, 280, 284, 288, 292,
    296, 300, 304, 309, 313, 317, 322, 326, 330, 335, 340, 344,
    349, 354, 359, 364, 369, 374, 379, 384, 389, 395, 400, 406,
    411, 417, 423, 429, 435, 441, 447, 454, 461, 467, 475, 482,
    489, 497, 505, 513, 522, 530, 539, 549, 559, 569, 579, 590,
    602, 614, 626, 640, 654, 668, 684, 700, 717, 736, 755, 775,
    796, 819, 843, 869, 896, 925, 955, 988, 1022, 1058, 1098, 1139,
    1184, 1232, 1282, 1336
  },
#if LIBGAV1_MAX_BITDEPTH >= 10
  // Lookup table for 10 bit.
  {
    4, 9, 10, 13, 15, 17, 20, 22, 25, 28, 31, 34,
    37, 40, 43, 47, 50, 53, 57, 60, 64, 68, 71, 75,
    78, 82, 86, 90, 93, 97, 101, 105, 109, 113, 116, 120,
    124, 128, 132, 136, 140, 143, 147, 151, 155, 159, 163, 166,
    170, 174, 178, 182, 185, 189, 193, 197, 200, 204, 208, 212,
    215, 219, 223, 226, 230, 233, 237, 241, 244, 248, 251, 255,
    259, 262, 266, 269, 273, 276, 280, 283, 287, 290, 293, 297,
    300, 304, 307, 310, 314, 317, 321, 324, 327, 331, 334, 337,
    343, 350, 356, 362, 369, 375, 381, 387, 394, 400, 406, 412,
    418, 424, 430, 436, 442, 448, 454, 460, 466, 472, 478, 484,
    490, 499, 507, 516, 525, 533, 542, 550, 559, 567, 576, 584,
    592, 601, 609, 617, 625, 634, 644, 655, 666, 676, 687, 698,
    708, 718, 729, 739, 749, 759, 770, 782, 795, 807, 819, 831,
    844, 856, 868, 880, 891, 906, 920, 933, 947, 961, 975, 988,
    1001, 1015, 1030, 1045, 1061, 1076, 1090, 1105, 1120, 1137, 1153, 1170,
    1186, 1202, 1218, 1236, 1253, 1271, 1288, 1306, 1323, 1342, 1361, 1379,
    1398, 1416, 1436, 1456, 1476, 1496, 1516, 1537, 1559, 1580, 1601, 1624,
    1647, 1670, 1692, 1717, 1741, 1766, 1791, 1817, 1844, 1871, 1900, 1929,
    1958, 1990, 2021, 2054, 2088, 2123, 2159, 2197, 2236, 2276, 2319, 2363,
    2410, 2458, 2508, 2561, 2616, 2675, 2737, 2802, 2871, 2944, 3020, 3102,
    3188, 3280, 3375, 3478, 3586, 3702, 3823, 3953, 4089, 4236, 4394, 4559,
    4737, 4929, 5130, 5347
  },
#endif  // LIBGAV1_MAX_BITDEPTH >= 10
};

constexpr int16_t kAcLookup[][256] = {
  // Lookup table for 8 bit.
  {
    4, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66,
    67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
    79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,
    91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
    104, 106, 108, 110, 112, 114, 116, 118, 120, 122, 124, 126,
    128, 130, 132, 134, 136, 138, 140, 142, 144, 146, 148, 150,
    152, 155, 158, 161, 164, 167, 170, 173, 176, 179, 182, 185,
    188, 191, 194, 197, 200, 203, 207, 211, 215, 219, 223, 227,
    231, 235, 239, 243, 247, 251, 255, 260, 265, 270, 275, 280,
    285, 290, 295, 300, 305, 311, 317, 323, 329, 335, 341, 347,
    353, 359, 366, 373, 380, 387, 394, 401, 408, 416, 424, 432,
    440, 448, 456, 465, 474, 483, 492, 501, 510, 520, 530, 540,
    550, 560, 571, 582, 593, 604, 615, 627, 639, 651, 663, 676,
    689, 702, 715, 729, 743, 757, 771, 786, 801, 816, 832, 848,
    864, 881, 898, 915, 933, 951, 969, 988, 1007, 1026, 1046, 1066,
    1087, 1108, 1129, 1151, 1173, 1196, 1219, 1243, 1267, 1292, 1317, 1343,
    1369, 1396, 1423, 1451, 1479, 1508, 1537, 1567, 1597, 1628, 1660, 1692,
    1725, 1759, 1793, 1828
  },
#if LIBGAV1_MAX_BITDEPTH >= 10
  // Lookup table for 10 bit.
  {
    4, 9, 11, 13, 16, 18, 21, 24, 27, 30, 33, 37,
    40, 44, 48, 51, 55, 59, 63, 67, 71, 75, 79, 83,
    88, 92, 96, 100, 105, 109, 114, 118, 122, 127, 131, 136,
    140, 145, 149, 154, 158, 163, 168, 172, 177, 181, 186, 190,
    195, 199, 204, 208, 213, 217, 222, 226, 231, 235, 240, 244,
    249, 253, 258, 262, 267, 271, 275, 280, 284, 289, 293, 297,
    302, 306, 311, 315, 319, 324, 328, 332, 337, 341, 345, 349,
    354, 358, 362, 367, 371, 375, 379, 384, 388, 392, 396, 401,
    409, 417, 425, 433, 441, 449, 458, 466, 474, 482, 490, 498,
    506, 514, 523, 531, 539, 547, 555, 563, 571, 579, 588, 596,
    604, 616, 628, 640, 652, 664, 676, 688, 700, 713, 725, 737,
    749, 761, 773, 785, 797, 809, 825, 841, 857, 873, 889, 905,
    922, 938, 954, 970, 986, 1002, 1018, 1038, 1058, 1078, 1098, 1118,
    1138, 1158, 1178, 1198, 1218, 1242, 1266, 1290, 1314, 1338, 1362, 1386,
    1411, 1435, 1463, 1491, 1519, 1547, 1575, 1603, 1631, 1663, 1695, 1727,
    1759, 1791, 1823, 1859, 1895, 1931, 1967, 2003, 2039, 2079, 2119, 2159,
    2199, 2239, 2283, 2327, 2371, 2415, 2459, 2507, 2555, 2603, 2651, 2703,
    2755, 2807, 2859, 2915, 2971, 3027, 3083, 3143, 3203, 3263, 3327, 3391,
    3455, 3523, 3591, 3659, 3731, 3803, 3876, 3952, 4028, 4104, 4184, 4264,
    4348, 4432, 4516, 4604, 4692, 4784, 4876, 4972, 5068, 5168, 5268, 5372,
    5476, 5584, 5692, 5804, 5916, 6032, 6148, 6268, 6388, 6512, 6640, 6768,
    6900, 7036, 7172, 7312
  },
#endif  // LIBGAV1_MAX_BITDEPTH >= 10
};
// clang-format on

void Transpose(uint8_t* const dst, const uint8_t* const src, int src_width,
               int src_height) {
  const int dst_width = src_height;
  const int dst_height = src_width;
  Array2DView<const uint8_t> source(src_height, src_width, src);
  Array2DView<uint8_t> dest(dst_height, dst_width, dst);
  for (int y = 0; y < dst_height; ++y) {
    for (int x = 0; x < dst_width; ++x) {
      dest[y][x] = source[x][y];
    }
  }
}

// Copies the lower triangle and fills the upper triangle of |dst| using |src|
// as the source.
void FillUpperTriangle(uint8_t* dst, const uint8_t* src, int size) {
  Array2DView<uint8_t> dest(size, size, dst);
  int k = 0;
  for (int y = 0; y < size; ++y) {
    for (int x = 0; x <= y; ++x) {
      dest[y][x] = dest[x][y] = src[k++];
    }
  }
}

}  // namespace

bool InitializeQuantizerMatrix(QuantizerMatrix* quantizer_matrix_ptr) {
  for (int level = 0; level < kNumQuantizerLevelsForQuantizerMatrix; ++level) {
    for (int plane_type = kPlaneTypeY; plane_type < kNumPlaneTypes;
         ++plane_type) {
      auto& quantizer_matrix = (*quantizer_matrix_ptr)[level][plane_type];
      // Notes about how these matrices are populated:
      // * For square transforms, we store only the lower left triangle (it is
      // symmetric about the main diagonal. So when populating the matrix, we
      // will have to fill in the upper right triangle.
      // * For rectangular transforms, the matrices are transposes when the
      // width and height are reversed. So when populating we populate it with
      // memcpy when w < h and populate it by transposing when w > h.
      // * There is a special case for 16x16 where the matrix is the same as
      // 32x32 with some offsets.
      // * We use the "adjusted transform size" when using these matrices, so we
      // won't have to populate them for transform sizes with one of the
      // dimensions equal to 64.
      for (int tx_size = 0; tx_size < kNumTransformSizes; ++tx_size) {
        if (kTransformWidth[tx_size] == 64 || kTransformHeight[tx_size] == 64) {
          continue;
        }
        const int size = kTransformWidth[tx_size] * kTransformHeight[tx_size];
        if (!quantizer_matrix[tx_size].Resize(size)) {
          return false;
        }
      }
#define QUANTIZER_MEMCPY(W, H)                            \
  memcpy(quantizer_matrix[kTransformSize##W##x##H].get(), \
         kQuantizerMatrix##W##x##H[level][plane_type], (W) * (H))
#define QUANTIZER_TRANSPOSE(W, H)                            \
  Transpose(quantizer_matrix[kTransformSize##W##x##H].get(), \
            kQuantizerMatrix##H##x##W[level][plane_type], H, W)
#define QUANTIZER_FILL_UPPER_TRIANGLE(SIZE)                                \
  FillUpperTriangle(quantizer_matrix[kTransformSize##SIZE##x##SIZE].get(), \
                    kQuantizerMatrix##SIZE##x##SIZE[level][plane_type], SIZE)
      QUANTIZER_FILL_UPPER_TRIANGLE(4);   // 4x4
      QUANTIZER_MEMCPY(4, 8);             // 4x8
      QUANTIZER_MEMCPY(4, 16);            // 4x16
      QUANTIZER_TRANSPOSE(8, 4);          // 8x4
      QUANTIZER_FILL_UPPER_TRIANGLE(8);   // 8x8
      QUANTIZER_MEMCPY(8, 16);            // 8x16
      QUANTIZER_MEMCPY(8, 32);            // 8x32
      QUANTIZER_TRANSPOSE(16, 4);         // 16x4
      QUANTIZER_TRANSPOSE(16, 8);         // 16x8
      QUANTIZER_MEMCPY(16, 32);           // 16x32
      QUANTIZER_TRANSPOSE(32, 8);         // 32x8
      QUANTIZER_TRANSPOSE(32, 16);        // 32x16
      QUANTIZER_FILL_UPPER_TRIANGLE(32);  // 32x32
      // 16x16.
      Array2DView<uint8_t> dst16x16(
          16, 16, quantizer_matrix[kTransformSize16x16].get());
      Array2DView<const uint8_t> src32x32(
          32, 32, quantizer_matrix[kTransformSize32x32].get());
      for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
          dst16x16[y][x] = src32x32[MultiplyBy2(y)][MultiplyBy2(x)];
        }
      }
#undef QUANTIZER_FILL_UPPER_TRIANGLE
#undef QUANTIZER_TRANSPOSE
#undef QUANTIZER_MEMCPY
    }
  }
  return true;
}

int GetQIndex(const Segmentation& segmentation, int index, int base_qindex) {
  if (segmentation.FeatureActive(index, kSegmentFeatureQuantizer)) {
    const int segment_qindex =
        base_qindex +
        segmentation.feature_data[index][kSegmentFeatureQuantizer];
    return Clip3(segment_qindex, kMinQuantizer, kMaxQuantizer);
  }
  return base_qindex;
}

Quantizer::Quantizer(int bitdepth, const QuantizerParameters* params)
    : params_(*params) {
  assert(bitdepth >= 8 && bitdepth <= LIBGAV1_MAX_BITDEPTH);
  const int index = BitdepthToArrayIndex(bitdepth);
  dc_lookup_ = kDcLookup[index];
  ac_lookup_ = kAcLookup[index];
}

int Quantizer::GetDcValue(Plane plane, int qindex) const {
  return dc_lookup_[Clip3(qindex + params_.delta_dc[plane], kMinQuantizer,
                          kMaxQuantizer)];
}

int Quantizer::GetAcValue(Plane plane, int qindex) const {
  return ac_lookup_[Clip3(qindex + params_.delta_ac[plane], kMinQuantizer,
                          kMaxQuantizer)];
}

}  // namespace libgav1
