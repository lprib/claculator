#pragma once

#include "text.hpp"
#include "raylib.h"
#include "view/style.hpp"
#include <vector>

static constexpr int bigfont_textbox_height() {
   return kDefaultStyle.big_font + 4;
}

static constexpr int smallfont_textbox_height() {
   return kDefaultStyle.small_font + 4;
}

static constexpr int letter_spacing() {
   return kDefaultStyle.big_font / 5;
}

struct SpanDescription {
   TextSpan span;
   Color color;
   std::string popup;
   SpanDescription(TextSpan _span, Color _color, std::string _popup = "") :
      span(_span),
      color(_color),
      popup(_popup) {}
};

void single_line_textbox(
   int x, int y, int w, std::string const& str, int font_size, Color outline, Color fill, Color text
);

void rich_text_box(
   int x, int y, int w, std::string const& str, int font_size, Color outline, Color fill,
   Color text_default, Color highlight, int highlighted_index,
   std::vector<SpanDescription> const& spans
);