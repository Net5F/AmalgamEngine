#include "Morton.h"
#include <cstdint>

namespace AM
{
/** A lookup table of morton code values, for up to a 16x16 value space. 
    Note: If you're trying to understand the ordering, look at the wiki page.
          This initializer visually has the X and Y axis flipped. */
static constexpr std::array<std::array<Uint8, 16>, 16> zValues{
    {{0, 2, 8, 10, 32, 34, 40, 42, 128, 130, 136, 138, 160, 162, 168, 170},
     {1, 3, 9, 11, 33, 35, 41, 43, 129, 131, 137, 139, 161, 163, 169, 171},
     {4, 6, 12, 14, 36, 38, 44, 46, 132, 134, 140, 142, 164, 166, 172, 174},
     {5, 7, 13, 15, 37, 39, 45, 47, 133, 135, 141, 143, 165, 167, 173, 175},
     {16, 18, 24, 26, 48, 50, 56, 58, 144, 146, 152, 154, 176, 178, 184, 186},
     {17, 19, 25, 27, 49, 51, 57, 59, 145, 147, 153, 155, 177, 179, 185, 187},
     {20, 22, 28, 30, 52, 54, 60, 62, 148, 150, 156, 158, 180, 182, 188, 190},
     {21, 23, 29, 31, 53, 55, 61, 63, 149, 151, 157, 159, 181, 183, 189, 191},
     {64, 66, 72, 74, 96, 98, 104, 106, 192, 194, 200, 202, 224, 226, 232, 234},
     {65, 67, 73, 75, 97, 99, 105, 107, 193, 195, 201, 203, 225, 227, 233, 235},
     {68, 70, 76, 78, 100, 102, 108, 110, 196, 198, 204, 206, 228, 230, 236,
      238},
     {69, 71, 77, 79, 101, 103, 109, 111, 197, 199, 205, 207, 229, 231, 237,
      239},
     {80, 82, 88, 90, 112, 114, 120, 122, 208, 210, 216, 218, 240, 242, 248,
      250},
     {81, 83, 89, 91, 113, 115, 121, 123, 209, 211, 217, 219, 241, 243, 249,
      251},
     {84, 86, 92, 94, 116, 118, 124, 126, 212, 214, 220, 222, 244, 246, 252,
      254},
     {85, 87, 93, 95, 117, 119, 125, 127, 213, 215, 221, 223, 245, 247, 253,
      255}}};

static constexpr Uint64 magicbit2D_masks64[6]
    = {0x00000000FFFFFFFF, 0x0000FFFF0000FFFF, 0x00FF00FF00FF00FF,
       0x0F0F0F0F0F0F0F0F, 0x3333333333333333, 0x5555555555555555};

Uint16 Morton::m2D_lookup_16x16(Uint8 x, Uint8 y)
{
    return static_cast<Uint16>(zValues[x][y]);
}

Uint32 Morton::m2D_e_magicbits_combined(Uint16 x, Uint16 y)
{
    Uint64 m = x
               | (static_cast<Uint64>(y)
                  << 32); // put Y in upper 32 bits, X in lower 32 bits
    m = (m | (m << 8)) & magicbit2D_masks64[2];
    m = (m | (m << 4)) & magicbit2D_masks64[3];
    m = (m | (m << 2)) & magicbit2D_masks64[4];
    m = (m | (m << 1)) & magicbit2D_masks64[5];
    m = m | (m >> 31); // merge X and Y back together
    // Note: This is a hard cut off at 32 bits, to drop the split Y-version in 
    //       the upper 32 bits.
    return static_cast<Uint32>(m);
}

} // End namespace AM
