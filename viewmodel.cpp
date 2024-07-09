#include "viewmodel.hpp"
#include "calc/parse.hpp"
#include "text.hpp"

#include <array>
#include <charconv>
#include <format>
#include <iostream>

// temp for test
// RegisterDisplay test_register(std::vector<Field>{Field(0, 1, "TEST", FieldDisplay::kNumeric)});
RegisterDisplay test_register(std::vector<Field>{});

void ViewModel::OnCharPressed(int chr) {
   switch(editor_mode.mode) {
   case EditorMode::Mode::kInsert:
      if((chr >= 32) && (chr <= 125)) {
         // current_input.push_back(static_cast<char>(chr));
         if(highlighted_index >= current_input.size()) {
            current_input.append(1, static_cast<char>(chr));
         } else {
            current_input.insert(highlighted_index, 1, static_cast<char>(chr));
         }
         ++highlighted_index;
         OnInputChanged(true);
      }
      break;
   default:
      break;
   }
}

void ViewModel::OnHistoryHighlightChanged() {
   if(history_highlighted_index < history.size()) {
      current_input = history[history_highlighted_index];
      highlighted_index = current_input.size();
      OnInputChanged(false);
   }
}

template <typename T> T RotateEnum(T value, T max) {
   auto new_value = static_cast<int>(value) + 1;
   if(new_value > static_cast<int>(max)) {
      new_value = 0;
   }

   return static_cast<T>(new_value);
}

void ViewModel::OnKeyPressed(KeyboardKey k) {
   if(k == KEY_ENTER) {
      OnCommit();
      return;
   }

   if((editor_mode.mode == EditorMode::Mode::kNormal) || IsKeyDown(KEY_LEFT_CONTROL) ||
      IsKeyDown(KEY_RIGHT_CONTROL)) {
      switch(k) {
      case KEY_H:
         if(highlighted_index > 0) {
            --highlighted_index;
         }
         break;
      case KEY_L:
         if(highlighted_index <= current_input.size()) {
            ++highlighted_index;
         }
         break;
      case KEY_I:
         editor_mode.mode = EditorMode::Mode::kInsert;
         break;
      case KEY_J:
         if(history_highlighted_index < history.size()) {
            ++history_highlighted_index;
         }
         OnHistoryHighlightChanged();
         break;
      case KEY_K:
         if(history_highlighted_index > 0) {
            --history_highlighted_index;
         }
         OnHistoryHighlightChanged();
         break;
      case KEY_A:
         if(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
            highlighted_index = current_input.size();
         } else {
            if(highlighted_index <= current_input.size()) {
               ++highlighted_index;
            }
         }
         editor_mode.mode = EditorMode::Mode::kInsert;
         break;
      case KEY_D:
         current_input.clear();
         highlighted_index = 0;
         OnInputChanged(true);
         break;
      case KEY_Z:
         input_display.Rotate();
         OnInputChanged(true);
         break;
      case KEY_X:
         output_display.Rotate();
         break;
      case KEY_S:
         sep_mode.Rotate();
         break;
      case KEY_W:
         int_width.Rotate();
         break;
      case KEY_R:
         fix_mode.Rotate();
         break;
      default:
         break;
      }
   }

   if(editor_mode.mode == EditorMode::Mode::kInsert) {
      switch(k) {
      case KEY_BACKSPACE:
         if(!current_input.empty()) {
            if(IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
               // delete non-whitespace
               while((highlighted_index > 0) && !IsWhitespace(current_input[highlighted_index - 1])
               ) {
                  DeleteOneChar();
               }
               // delete whitespace
               while((highlighted_index > 0) && IsWhitespace(current_input[highlighted_index - 1])
               ) {
                  DeleteOneChar();
               }
               OnInputChanged(true);
            } else {
               DeleteOneChar();
               OnInputChanged(true);
            }
         }
         break;
      case KEY_CAPS_LOCK:
         editor_mode.mode = EditorMode::Mode::kNormal;
         break;
      default:
         break;
      }
   }
}

void ViewModel::DeleteOneChar() {
   auto to_delete = static_cast<int>(highlighted_index) - 1;
   if(to_delete >= 0) {
      current_input.erase(to_delete, 1);
      --highlighted_index;
   }
}

std::string ViewModel::GetStackDisplayString(int index) {
   calc::Value const& item = state.speculative_stack.data[index];
   switch(item.type()) {
   case calc::Value::Type::kInt: {
      std::array<char, 33> buf{};
      int base = intbase::as_int(output_display.mode);
      std::to_chars(
         &*buf.begin(),
         (&*buf.begin()) + buf.size(),
         state.speculative_stack.data[index].int_or_default(),
         base
      );
      auto str = std::string(&*buf.begin());
      if(sep_mode.mode != SeparatorMode::Mode::kNone) {
         int skipped = 1;
         for(int i = str.size() - 1; i > 0; --i) {
            if(skipped == sep_mode.ToNumDigits()) {
               str.insert(i, 1, ',');
               skipped = 0;
            }
            ++skipped;
         }
      }
      return str;
   }; break;
   case calc::Value::Type::kDouble:
      return std::format("{}", item.double_or_default());
   case calc::Value::Type::kString:
      return std::format("\"{}\"", item.string_or_default());
   default:
      return "";
   }
}

void ViewModel::OnInputChanged(bool reset_history_highlight) {
   parsed = parse::parse(parse::ParserSettings(input_display.mode, state.functions), current_input);

   for(auto const& token : parsed) {
      std::cout << token << " ";
   }
   std::cout << "\n";

   state.Speculate(parsed);
   if(reset_history_highlight) {
      history_highlighted_index = history.size();
   }
}

void ViewModel::OnCommit() {
   state.Commit();
   history.push_back(current_input);
   current_input.clear();
   highlighted_index = 0;
   history_highlighted_index = history.size();
}