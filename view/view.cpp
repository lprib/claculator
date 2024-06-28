#include "raylib.h"

#include "view/BitfieldDisplay.hpp"
#include "view/style.hpp"
#include "view/view.hpp"
#include <array>
#include <charconv>
#include <cmath>
#include <iostream>
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

static void single_line_textbox(
   int x, int y, int w, std::string const& str, int font_size, Color outline,
   Color fill, Color text, Color highlight = MAGENTA, int highlighted_index = -1
) {
   DrawRectangleLines(x, y, w, font_size + 4, outline);
   DrawRectangle(x + 1, y + 1, w - 2, font_size + 4 - 2, fill);
   auto hpad = font_size / 8;
   DrawText(str.c_str(), x + 2 + hpad, y + 2, font_size, text);

   if(highlighted_index != -1) {
      auto pre_str = str.substr(0, highlighted_index);
      auto port_str = str.substr(0, highlighted_index + 1);
      auto start = MeasureText(pre_str.c_str(), font_size);
      auto end = highlighted_index >= str.size()
                    ? start + 10 // default cursor width
                    : MeasureText(port_str.c_str(), font_size);

      Color lerpHighlight = highlight;
      float lerp = (std::sin(GetTime() * 6.28 / kBlinkPeriod) + 1.0) / 2.0;
      lerpHighlight.a = (int)(lerp * (float)lerpHighlight.a);

      DrawRectangle(
         x + start + 2 + hpad + letter_spacing() / 2,
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
   for(std::size_t i = 0; i < m_vm.history.size(); ++i) {
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

void View::render_main_input() {
   auto highlight = Color{0xff, 0xff, 0xff, 0x80};
   auto is_infix = m_vm.fix_mode.mode == FixMode::Mode::kInfix;

   auto x = is_infix ? 20 : 0;
   auto w = GetScreenWidth() - (is_infix ? 40 : 0);

   auto y =
      GetScreenHeight() - bigfont_textbox_height() - smallfont_textbox_height();

   single_line_textbox(
      x,
      y,
      w,
      m_vm.current_input.c_str(),
      kDefaultStyle.big_font,
      is_infix ? kDefaultStyle.fg_mid : kDefaultStyle.bg1_dark,
      kDefaultStyle.fg_dark,
      kDefaultStyle.bg1_light,
      highlight,
      m_vm.highlighted_index
   );

   if(is_infix) {
      DrawText("(", 5, y + 2, kDefaultStyle.big_font, kDefaultStyle.fg_mid);
      DrawText(
         ")",
         GetScreenWidth() - 15,
         y + 2,
         kDefaultStyle.big_font,
         kDefaultStyle.fg_mid
      );
   }
}

struct ModeWidth {
   EnumeratedMode const* mode;
   int width;
};

void View::render_state_infobar() {
   char const* mode_text;
   Color outline, fill, text;
   switch(m_vm.editor_mode.mode) {
   case EditorMode::Mode::kInsert:
      outline = kDefaultStyle.bg3_mid;
      fill = kDefaultStyle.bg3_dark;
      text = kDefaultStyle.bg2_light;
      mode_text = "I";
      break;
   case EditorMode::Mode::kNormal:
      outline = kDefaultStyle.bg1_mid;
      fill = kDefaultStyle.bg1_dark;
      text = kDefaultStyle.bg1_light;
      mode_text = "N";
      break;
   }
   int xoffset = 0;

   single_line_textbox(
      xoffset,
      GetScreenHeight() - smallfont_textbox_height(),
      30,
      mode_text,
      kDefaultStyle.small_font,
      outline,
      fill,
      text
   );
   xoffset += 30;

   std::vector<ModeWidth> modes = {
      {&m_vm.input_display, 60},
      {&m_vm.output_display, 65},
      {&m_vm.sep_mode, 80},
      {&m_vm.int_width, 60},
      {&m_vm.fix_mode, 50}
   };

   for(auto const& modewidth : modes) {
      static constexpr int kKeybindSize = 20;
      single_line_textbox(
         xoffset,
         GetScreenHeight() - smallfont_textbox_height(),
         kKeybindSize,
         modewidth.mode->KeybindString(),
         kDefaultStyle.small_font,
         kDefaultStyle.bg2_mid,
         kDefaultStyle.bg2_dark,
         kDefaultStyle.bg2_light
      );
      xoffset += kKeybindSize;

      single_line_textbox(
         xoffset,
         GetScreenHeight() - smallfont_textbox_height(),
         modewidth.width,
         modewidth.mode->DisplayString(),
         kDefaultStyle.small_font,
         kDefaultStyle.bg1_mid,
         kDefaultStyle.bg1_dark,
         kDefaultStyle.bg1_light
      );
      xoffset += modewidth.width;
   }
}

void View::render() {
   ClearBackground(kDefaultStyle.neutral_bg);
   render_main_input();
   render_state_infobar();
   render_stack();
   render_history();

   BitfieldDisplay::render(
      0,
      GetScreenHeight() - 300,
      m_vm.currentRegister,
      m_vm.state.speculative_stack.data.empty()
         ? 0
         : m_vm.state.speculative_stack.data.back()
   );
}