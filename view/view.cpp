#include "raylib.h"

#include "text.hpp"
#include "ui_components.hpp"
#include "view/BitfieldDisplay.hpp"
#include "view/style.hpp"
#include "view/view.hpp"
#include <array>
#include <charconv>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

void View::render_stack() {
   // TODO scroll view
   DrawText("Stack", 5, 5, kDefaultStyle.small_font, kDefaultStyle.dark_text);
   for(std::size_t i = 0; i < m_controller.state.speculative_stack.data.size(); ++i) {
      auto data = m_controller.GetStackDisplayString(i);
      single_line_textbox(
         1,
         i * bigfont_textbox_height() + 8 + kDefaultStyle.small_font,
         400,
         data.c_str(),
         kDefaultStyle.big_font,
         to_dark_text_color(m_controller.output_display.mode),
         kDefaultStyle.dark_bg,
         kDefaultStyle.dark_text
      );
   }
}

void View::render_history() {
   // TODO scroll view
   DrawText(
      "History",
      GetScreenWidth() - 400 + 4,
      5,
      kDefaultStyle.small_font,
      kDefaultStyle.dark_text
   );
   for(size_t i = 0; i < m_controller.history.size(); ++i) {
      auto data = m_controller.history[i].c_str();
      auto is_highlighted = m_controller.history_highlighted_index == i;
      single_line_textbox(
         GetScreenWidth() - 400 - 1,
         i * bigfont_textbox_height() + 8 + kDefaultStyle.small_font,
         400,
         data,
         kDefaultStyle.big_font,
         kDefaultStyle.highlight,
         kDefaultStyle.dark_bg,
         is_highlighted ? kDefaultStyle.dark_text_emphasis : kDefaultStyle.dark_text
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
         spans.push_back(
            SpanDescription(tok.span, kDefaultStyle.dark_text_emphasis, tok.additional_popup_text)
         );
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
      m_controller.current_input.c_str(),
      kDefaultStyle.big_font,
      SKYBLUE,
      kDefaultStyle.dark_bg,
      kDefaultStyle.dark_text,
      highlight,
      m_controller.highlighted_index,
      tokens_to_span_desc(m_controller.parsed)
   );
}

struct ModeWidth {
   EnumeratedMode const* mode;
   int width;
};

void View::render_state_infobar() {
   static constexpr int kKeybindSize = 20;
   static constexpr int kPadding = 5;

   std::vector<ModeWidth> modes = {
      {&m_controller.editor_mode, 70},
      {&m_controller.input_display, 50},
      {&m_controller.output_display, 50},
      {&m_controller.sep_mode, 80},
      {&m_controller.int_width, 60},
      {&m_controller.fix_mode, 50},
      {&m_controller.fast_entry_mode, 90},
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

void View::render_multi_base_displays() {
   static constexpr int kWidth = 250;
   static constexpr int kPadding = 5;
   single_line_textbox(
      kPadding + 0*(kWidth+kPadding),
      GetScreenHeight() - 120,
      kWidth,
      m_controller.GetStackDisplayStringRadix(0, NumericDisplayMode::Mode::kDec),
      kDefaultStyle.small_font,
      to_dark_text_color(intbase::IntBase::kDec),
      kDefaultStyle.dark_bg,
      kDefaultStyle.dark_text
   );
   single_line_textbox(
      kPadding + 1*(kWidth+kPadding),
      GetScreenHeight() - 120,
      kWidth,
      m_controller.GetStackDisplayStringRadix(0, NumericDisplayMode::Mode::kHex),
      kDefaultStyle.small_font,
      to_dark_text_color(intbase::IntBase::kHex),
      kDefaultStyle.dark_bg,
      kDefaultStyle.dark_text
   );
   single_line_textbox(
      kPadding + 2*(kWidth+kPadding),
      GetScreenHeight() - 120,
      kWidth,
      m_controller.GetStackDisplayStringRadix(0, NumericDisplayMode::Mode::kBin),
      kDefaultStyle.small_font,
      to_dark_text_color(intbase::IntBase::kBin),
      kDefaultStyle.dark_bg,
      kDefaultStyle.dark_text
   );
}

void View::render() {
   ClearBackground(kDefaultStyle.neutral_bg);
   render_main_input();
   render_state_infobar();
   render_stack();
   render_history();

   int top_of_stack = 0;
   if(!m_controller.state.speculative_stack.data.empty() &&
      m_controller.state.speculative_stack.data.back().type() == calc::Value::Type::kInt) {
      top_of_stack = m_controller.state.speculative_stack.data.back().as_int();
   }

   render_multi_base_displays();

   auto const& reg = m_controller.current_register;
   BitfieldDisplay::render(
      5,
      GetScreenHeight() - BitfieldDisplay::height(reg) - 125,
      reg,
      top_of_stack
   );
}