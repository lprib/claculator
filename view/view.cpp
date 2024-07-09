#include "raylib.h"

#include "text.hpp"
#include "view/BitfieldDisplay.hpp"
#include "view/style.hpp"
#include "view/view.hpp"
#include <array>
#include <charconv>
#include <cmath>
#include <iostream>
#include <optional>
#include <string>
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

static constexpr float kBlinkPeriod = 0.5;

struct SpanDescription {
   TextSpan span;
   Color color;
   std::string popup;
   SpanDescription(TextSpan _span, Color _color, std::string _popup = "") :
      span(_span),
      color(_color),
      popup(_popup) {}
};

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

static void single_line_textbox(
   int x, int y, int w, std::string const& str, int font_size, Color outline, Color fill, Color text
) {
   textbox_background(x, y, w, font_size, outline, fill);
   DrawText(str.c_str(), x + 2 + font_size / 8, y + 2, font_size, text);
}

static void rich_text_box(
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

void View::render_stack() {
   for(std::size_t i = 0; i < m_vm.state.speculative_stack.data.size(); ++i) {
      auto data = m_vm.GetStackDisplayString(i);
      single_line_textbox(
         1,
         i * bigfont_textbox_height() + 1,
         400,
         data.c_str(),
         kDefaultStyle.big_font,
         kDefaultStyle.bg1_mid,
         kDefaultStyle.bg1_dark,
         kDefaultStyle.bg1_light
      );
   }
}

void View::render_history() {
   for(size_t i = 0; i < m_vm.history.size(); ++i) {
      auto data = m_vm.history[i].c_str();
      auto is_highlighted = m_vm.history_highlighted_index == i;
      single_line_textbox(
         GetScreenWidth() - 400 - 1,
         i * bigfont_textbox_height() + 1,
         400,
         data,
         kDefaultStyle.big_font,
         is_highlighted ? kDefaultStyle.bg3_dark : kDefaultStyle.bg3_light,
         is_highlighted ? kDefaultStyle.bg3_light : kDefaultStyle.bg3_dark,
         kDefaultStyle.bg3_mid
      );
   }
}

static std::vector<SpanDescription> tokens_to_span_desc(std::vector<parse::Token> const& tokens) {
   auto spans = std::vector<SpanDescription>();
   for(auto const& tok : tokens) {
      switch(tok.type) {
      case parse::TokenType::kDecimalNumber:
         spans.push_back(SpanDescription(tok.span, to_dark_text_color(intbase::IntBase::kDec)));
         break;
      case parse::TokenType::kHexNumber:
         spans.push_back(SpanDescription(tok.span, to_dark_text_color(intbase::IntBase::kHex)));
         break;
      case parse::TokenType::kBinaryNumber:
         spans.push_back(SpanDescription(tok.span, to_dark_text_color(intbase::IntBase::kBin)));
         break;
      case parse::TokenType::kDouble:
         spans.push_back(SpanDescription(tok.span, kDefaultStyle.syntax_double_color));
         break;
      case parse::TokenType::kString:
         spans.push_back(SpanDescription(tok.span, kDefaultStyle.syntax_string_color));
         break;
      case parse::TokenType::kWord:
         spans.push_back(SpanDescription(tok.span, kDefaultStyle.dark_text_emphasis));
         break;
      case parse::TokenType::kError:
         spans.push_back(SpanDescription(tok.span, RED, tok.text));
         break;
      }
   }

   return spans;
}

void View::render_main_input() {
   auto highlight = Color{0xff, 0xff, 0xff, 0x80};
   static constexpr int kPadding = 5;
   auto y = GetScreenHeight() - bigfont_textbox_height() - smallfont_textbox_height() - kPadding;

   rich_text_box(
      5,
      y,
      GetScreenWidth() - kPadding * 2,
      m_vm.current_input.c_str(),
      kDefaultStyle.big_font,
      SKYBLUE,
      kDefaultStyle.dark_bg,
      kDefaultStyle.dark_text,
      highlight,
      m_vm.highlighted_index,
      tokens_to_span_desc(m_vm.parsed)
   );
}

struct ModeWidth {
   EnumeratedMode const* mode;
   int width;
};

void View::render_state_infobar() {
   static constexpr int kKeybindSize = 20;
   static constexpr int kPadding = 10;

   std::vector<ModeWidth> modes = {
      {&m_vm.editor_mode, 70},
      {&m_vm.input_display, 60},
      {&m_vm.output_display, 65},
      {&m_vm.sep_mode, 80},
      {&m_vm.int_width, 60},
      {&m_vm.fix_mode, 50}
   };

   int xoffset = 0;

   for(auto const& modewidth : modes) {
      if(modewidth.mode->ShowKeybind()) {
         single_line_textbox(
            xoffset,
            GetScreenHeight() - smallfont_textbox_height(),
            kKeybindSize,
            modewidth.mode->KeybindString(),
            kDefaultStyle.small_font,
            kDefaultStyle.dark_text,
            kDefaultStyle.dark_bg,
            kDefaultStyle.dark_text
         );
         xoffset += kKeybindSize;
      }

      single_line_textbox(
         xoffset,
         GetScreenHeight() - smallfont_textbox_height(),
         modewidth.width,
         modewidth.mode->DisplayString(),
         kDefaultStyle.small_font,
         modewidth.mode->BgColor(),
         modewidth.mode->BgColor(),
         modewidth.mode->TextColor()
      );
      xoffset += modewidth.width + kPadding;
   }
}

void View::render() {
   ClearBackground(kDefaultStyle.neutral_bg);
   render_main_input();
   render_state_infobar();
   render_stack();
   render_history();

   int bitfield = 0;
   if(!m_vm.state.speculative_stack.data.empty() &&
      m_vm.state.speculative_stack.data.back().type() == calc::Value::Type::kInt) {
      bitfield = m_vm.state.speculative_stack.data.back().int_or_default();
   }

   BitfieldDisplay::render(0, GetScreenHeight() - 300, m_vm.currentRegister, bitfield);
}