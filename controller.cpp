#include "controller.hpp"
#include "calc/parse.hpp"
#include "text.hpp"

#include <array>
#include <charconv>
#include <format>
#include <iostream>
#include <memory>

class FieldFunction : public calc::BuiltinNormalFunction {
public:
   FieldFunction(Controller& controller) :
      calc::BuiltinNormalFunction(3, "field"),
      m_controller(controller) {}
   bool allow_speculative_execution() const override {
      return false;
   }
   ExecutionResult execute(std::vector<calc::Value> input) override {
      if((input[0].type() != calc::Value::Type::kInt) ||
         (input[1].type() != calc::Value::Type::kInt) ||
         (input[2].type() != calc::Value::Type::kString)) {
         return ExecutionResult::make_error("require (int int string)");
      }

      for(auto const& field : m_controller.current_register.fields) {
         if(field.name == input[2].as_string()) {
            // an identical field already exists
            return ExecutionResult::make_success();
         }
      }

      m_controller.current_register.fields.push_back(
         Field(input[0].as_int(), input[1].as_int(), input[2].as_string(), FieldDisplay::kNumeric)
      );

      for(auto const& field : m_controller.current_register.fields) {
         std::cout << field.name << "\n";
      }

      return ExecutionResult::make_success();
   }

private:
   Controller& m_controller;
};

class ClearFieldsFunction : public calc::BuiltinNormalFunction {
public:
   ClearFieldsFunction(Controller& controller) :
      calc::BuiltinNormalFunction(0, "clearfields"),
      m_controller(controller) {}
   bool allow_speculative_execution() const override {
      return false;
   }
   ExecutionResult execute(std::vector<calc::Value> input) override {
      m_controller.current_register.fields.clear();
      return ExecutionResult::make_success();
   }

private:
   Controller& m_controller;
};

Controller::Controller() {
   state.functions.push_back(std::make_unique<FieldFunction>(*this));
   state.functions.push_back(std::make_unique<ClearFieldsFunction>(*this));
}

void Controller::OnCharPressed(int chr) {
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
         SpeculativelyExecuteInput(true, true);
      }
      break;
   default:
      break;
   }
}

void Controller::OnHistoryHighlightChanged() {
   if(history_highlighted_index < history.size()) {
      current_input = history[history_highlighted_index];
      highlighted_index = current_input.size();
      SpeculativelyExecuteInput(false, false);
   }
}

template <typename T> T RotateEnum(T value, T max) {
   auto new_value = static_cast<int>(value) + 1;
   if(new_value > static_cast<int>(max)) {
      new_value = 0;
   }

   return static_cast<T>(new_value);
}

void Controller::OnKeyPressed(KeyboardKey k) {
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
         SpeculativelyExecuteInput(true, false);
         break;
      case KEY_Z:
         input_display.Rotate();
         SpeculativelyExecuteInput(true, false);
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
      case KEY_F:
         fast_entry_mode.Rotate();
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
               SpeculativelyExecuteInput(true, false);
            } else {
               DeleteOneChar();
               SpeculativelyExecuteInput(true, false);
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

void Controller::DeleteOneChar() {
   auto to_delete = static_cast<int>(highlighted_index) - 1;
   if(to_delete >= 0) {
      current_input.erase(to_delete, 1);
      --highlighted_index;
   }
}

std::string Controller::GetStackDisplayString(int index) {
   calc::Value const& item = state.speculative_stack.data[index];
   switch(item.type()) {
   case calc::Value::Type::kInt: {
      std::array<char, 33> buf{};
      int base = intbase::as_int(output_display.mode);
      std::to_chars(
         &*buf.begin(),
         (&*buf.begin()) + buf.size(),
         state.speculative_stack.data[index].as_int(),
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
      return std::format("{}", item.as_double());
   case calc::Value::Type::kString:
      return std::format("\"{}\"", item.as_string());
   default:
      return "";
   }
}

void Controller::ParseInput() {
   parsed = parse::parse(parse::ParserSettings(input_display.mode, state.functions), current_input);
}

void Controller::SpeculativelyExecuteInput(bool reset_history_highlight, bool allow_fast_entry) {
   ParseInput();

   // If we are not selecting history:
   // If fast entry mode is enabled and the input is all ok (speculative etc):
   //    If the last token is a word and function->allow_speculative_execution():
   //       Commit the entry immediately and execute for real
   // Otherwise: speculatively execute (if applicable), or just parse and show
   // annotations
   if(allow_fast_entry && (fast_entry_mode.mode == FastEntryMode::Mode::kOn) && (!parsed.empty()) &&
      (parsed.back().type == parse::TokenType::kWord)) {
      bool ok_to_fast_commit = true;
      for(auto const& token : parsed) {
         if(token.type == parse::TokenType::kError) {
            ok_to_fast_commit = false;
            break;
         }
         if((token.type == parse::TokenType::kWord)) {
            auto& function = state.functions[parsed.back().function_index];
            if(!function->allow_speculative_execution()) {
               ok_to_fast_commit = false;
               break;
            }
         }
      }
      if(ok_to_fast_commit) {
         OnCommit();
         return;
      }
   }

   state.Execute(parsed, true);
   if(reset_history_highlight) {
      history_highlighted_index = history.size();
   }
}

void Controller::OnCommit() {
   if(current_input.empty()) {
      return;
   }
   state.Execute(parsed, false);
   state.Commit();
   if(history.empty() || (history.back() != current_input)) {
      history.push_back(current_input);
   }
   current_input.clear();
   highlighted_index = 0;
   history_highlighted_index = history.size();
}