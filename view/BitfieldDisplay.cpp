#include "view/BitfieldDisplay.hpp"
#include "raylib.h"
#include "view/style.hpp"
#include <array>
#include <charconv>
#include <iostream>

static constexpr std::array<Color, 10> kFieldColors = {
   YELLOW, ORANGE, PINK, RED, GREEN, LIME, SKYBLUE, BLUE, PURPLE, VIOLET
};

struct BitfieldDisplayInfo {
   int bitbox_size = kDefaultStyle.tiny_font + 3;
   int spacing = 2;
   int big_spacing = 10;
};

static int x_offset_of(int bit_index, BitfieldDisplayInfo const& info) {
   auto index_from_left = 31 - bit_index;
   return (info.bitbox_size + info.spacing) * index_from_left +
          info.big_spacing * (index_from_left / 8);
}

static void render_one_line(
   int x, int y, RegisterDisplay const& display, int64_t value, int bitoffset,
   int bitcount, BitfieldDisplayInfo const& info
) {
   for(int i = 0; i < bitcount; ++i) {
      int bit_index = (bitcount - i - 1) + bitoffset;
      bool bit_set = value & (1ull << bit_index);
      Color bg = bit_set ? kDefaultStyle.light_bg : kDefaultStyle.dark_bg;
      Color fg = bit_set ? kDefaultStyle.light_text : kDefaultStyle.dark_text;

      int x_offset = x_offset_of(bit_index - bitoffset, info);

      DrawRectangle(x + x_offset, y, info.bitbox_size, info.bitbox_size, bg);

      std::array<char, 10> buf{};
      std::to_chars(&*buf.begin(), (&*buf.begin()) + buf.size(), bit_index, 10);
      DrawText(
         &*buf.begin(),
         x + x_offset + info.bitbox_size / 2 - kDefaultStyle.tiny_font / 2,
         y + info.bitbox_size / 2 - kDefaultStyle.tiny_font / 2,
         kDefaultStyle.tiny_font,
         fg
      );
   }

   for(int i = 0; i < display.fields.size(); ++i) {
      auto const& field = display.fields[i];

      auto first_oob = (field.firstbit >= (bitoffset + bitcount)) ||
                       (field.firstbit < bitoffset);
      auto last_oob = (field.lastbit >= (bitoffset + bitcount)) ||
                      (field.lastbit < bitoffset);
      if(first_oob && last_oob) {
         continue;
      }

      int first_clipped = first_oob ? bitoffset : field.firstbit;
      int last_clipped = last_oob ? (bitoffset + bitcount - 1) : field.lastbit;
      int first_offset = x_offset_of(first_clipped - bitoffset, info);
      int last_offset = x_offset_of(last_clipped - bitoffset, info);
      int width = first_offset - last_offset + info.bitbox_size + 2;

      auto color = kFieldColors[i % kFieldColors.size()];
      DrawRectangleLines(
         x + last_offset - 1,
         y,
         width,
         info.bitbox_size + 2,
         color
      );

      int text_row = last_clipped % 4;
      auto string = field.name + "=" + field.GetDisplay(value);
      DrawText(
         string.c_str(),
         last_offset,
         y + text_row * kDefaultStyle.small_font + info.bitbox_size + 2,
         kDefaultStyle.small_font,
         color
      );
   }
}

void BitfieldDisplay::render(
   int x, int y, RegisterDisplay const& display, int64_t value
) {
   auto info = BitfieldDisplayInfo{
      .bitbox_size = kDefaultStyle.tiny_font + 11,
      .spacing = 2,
      .big_spacing = 10,
   };

   render_one_line(x + 1, y, display, value, 32, 32, info);
   render_one_line(x + 1, y + 110, display, value, 0, 32, info);
}