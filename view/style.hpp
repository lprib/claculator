#pragma once

#include "calc/intbase.hpp"
#include "raylib.h"

class Style {
public:
   int big_font;
   int small_font;
   int tiny_font;
   Color bg1_dark;
   Color bg1_mid;
   Color bg1_light;
   Color bg2_dark;
   Color bg2_mid;
   Color bg2_light;
   Color bg3_dark;
   Color bg3_mid;
   Color bg3_light;
   Color fg_dark;
   Color fg_mid;
   Color fg_light;
   Color all_bg;

   Color dark_bg;
   Color dark_text;
   Color neutral_bg;
   Color light_bg;
   Color light_text;
   Color highlight;
};

inline Color to_dark_text_color(intbase::IntBase base) {
   switch(base) {
   case intbase::IntBase::kDec:
      return SKYBLUE;
   case intbase::IntBase::kHex:
      return LIME;
   case intbase::IntBase::kBin:
      return YELLOW;
   default:
      return WHITE;
   }
}

// constexpr Style kDefaultStyle = {
//    30,
//    20,
//    Color{0x18, 0x02, 0x31, 0xff},
//    Color{0x2E, 0x0D, 0x52, 0xff},
//    Color{0x75, 0x5A, 0x94, 0xff},
//    Color{0x3C, 0x00, 0x1E, 0xff},
//    Color{0x64, 0x09, 0x37, 0xff},
//    Color{0xB4, 0x67, 0x8E, 0xff},
//    Color{0x03, 0x0F, 0x31, 0xff},
//    Color{0x10, 0x20, 0x52, 0xff},
//    Color{0x5C, 0x6A, 0x94, 0xff},
//    Color{0x49, 0x43, 0x00, 0xff},
//    Color{0x79, 0x71, 0x0B, 0xff},
//    Color{0xDB, 0xD4, 0x7E, 0xff},
//    Color{0x20, 0x20, 0x20, 0xff}
// };

constexpr Style kDefaultStyle = {
   30,
   20,
   10,
   DARKBLUE,
   BLUE,
   SKYBLUE,
   Color{60, 60, 60, 255},
   GRAY,
   LIGHTGRAY,
   DARKGREEN,
   GREEN,
   LIME,
   DARKGRAY,
   MAROON,
   RAYWHITE,
   Color{0x20, 0x20, 0x20, 0xff},

   Color{0x1e, 0x1e, 0x1e, 0xff},
   Color{0x75, 0x75, 0x75, 0xff},
   Color{0x28, 0x28, 0x28, 0xff},
   Color{0x79, 0x79, 0x79, 0xff},
   Color{0x14, 0x14, 0x14, 0xff},
   Color{0x1a, 0x3a, 0xac, 0xff},
};