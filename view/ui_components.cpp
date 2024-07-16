#include "ui_components.hpp"

#include <cmath>
#include <optional>

static constexpr float kBlinkPeriod = 0.5;

static void textbox_background(int x, int y, int w, int font_size, Color outline, Color fill) {
   DrawRectangleLines(x, y, w, font_size + 4, outline);
   DrawRectangle(x + 1, y + 1, w - 2, font_size + 4 - 2, fill);
}

static std::optional<Color> get_color(
   size_t char_index, std::vector<SpanDescription> const& spans
) {
   for(auto const& span_desc : spans) {
      if(span_desc.span.contains(char_index)) {
         return span_desc.color;
      }
   }
   return std::nullopt;
}

static std::optional<std::string> get_popup(
   size_t char_index, std::vector<SpanDescription> const& spans
) {
   for(auto const& span_desc : spans) {
      if((span_desc.span.start == char_index) && (span_desc.popup != "")) {
         return span_desc.popup;
      }
   }
   return std::nullopt;
}

void single_line_textbox(
   int x, int y, int w, std::string const& str, int font_size, Color outline, Color fill, Color text
) {
   textbox_background(x, y, w, font_size, outline, fill);
   DrawText(str.c_str(), x + 2 + font_size / 8, y + 2, font_size, text);
}

void rich_text_box(
   int x, int y, int w, std::string const& str, int font_size, Color outline, Color fill,
   Color text_default, Color highlight, int highlighted_index,
   std::vector<SpanDescription> const& spans
) {
   textbox_background(x, y, w, font_size, outline, fill);

   int xoffset = x + 2 + font_size / 8;
   bool first_popup = true;

   for(size_t char_i = 0; char_i < str.size(); ++char_i) {
      char cstr[2] = {str[char_i], 0};
      DrawText(cstr, xoffset, y + 2, font_size, get_color(char_i, spans).value_or(text_default));
      auto popup = get_popup(char_i, spans);
      if(popup.has_value()) {
         static constexpr int kPopupVertPad = 10;

         if(first_popup) {
            auto popup_c_str = popup->c_str();
            single_line_textbox(
               xoffset,
               y - kDefaultStyle.small_font - 4 - kPopupVertPad,
               MeasureText(popup_c_str, kDefaultStyle.small_font) + 6,
               popup_c_str,
               kDefaultStyle.small_font,
               kDefaultStyle.dark_text,
               kDefaultStyle.dark_bg,
               kDefaultStyle.dark_text
            );
         }

         static constexpr int kTriangleSize = 10;
         auto a = Vector2(xoffset + kTriangleSize / 2, y - kPopupVertPad + kTriangleSize / 1.414);
         auto b = Vector2(xoffset + kTriangleSize, y - kPopupVertPad);
         auto c = Vector2(xoffset, y - kPopupVertPad);
         DrawTriangle(a, b, c, kDefaultStyle.dark_text);

         first_popup = false;
      }
      xoffset += MeasureText(cstr, font_size) + font_size / 10;
   }

   if(highlighted_index != -1) {
      auto pre_str = str.substr(0, highlighted_index);
      auto port_str = str.substr(0, highlighted_index + 1);
      auto start = MeasureText(pre_str.c_str(), font_size);
      auto end = static_cast<size_t>(highlighted_index) >= str.size()
                    ? start + 10 // default cursor width
                    : MeasureText(port_str.c_str(), font_size);

      Color lerpHighlight = highlight;
      float lerp = (std::sin(GetTime() * 6.28 / kBlinkPeriod) + 1.0) / 2.0;
      lerpHighlight.a = (int)(lerp * (float)lerpHighlight.a);

      DrawRectangle(
         x + start + 2 + font_size / 8 + letter_spacing() / 2,
         y + 2,
         end - start,
         font_size,
         lerpHighlight
      );
   }
}
