#pragma once

#include "calc/bit_register.hpp"
#include "calc/calc.hpp"
#include "raylib.h"
#include "view/style.hpp"
#include <optional>
#include <vector>

class EnumeratedMode {
public:
   virtual char const* DisplayString() const = 0;
   virtual bool ShowKeybindString() const {
      return true;
   }
   // TODO auto dislpay this
   virtual char const* KeybindString() const = 0;
   virtual void Rotate() = 0;
   virtual Color BgColor() const {
      return kDefaultStyle.dark_bg;
   }
   virtual Color TextColor() const {
      return kDefaultStyle.dark_text;
   }
   virtual bool ShowKeybind() const {
      return true;
   }

protected:
   template <typename T> void EnumRotate(T& value, T max) {
      T value_v = value;
      auto new_value = static_cast<int>(value_v) + 1;
      if(new_value > static_cast<int>(max)) {
         new_value = 0;
      }
      value = static_cast<T>(new_value);
   }
};

struct EditorMode : public EnumeratedMode {
   enum class Mode { kInsert, kNormal };
   Mode mode = Mode::kInsert;
   char const* DisplayString() const override {
      switch(mode) {
      case Mode::kInsert:
         return "INS";
      case Mode::kNormal:
         return "NOR";
      }
      return "";
   }
   char const* KeybindString() const override {
      return "";
   }
   void Rotate() override {
      EnumRotate(mode, Mode::kNormal);
   }

   bool insert() const {
      return mode == Mode::kInsert;
   }

   Color BgColor() const override {
      return insert() ? kDefaultStyle.light_bg : kDefaultStyle.dark_bg;
   }
   Color TextColor() const override {
      return insert() ? DARKBLUE : kDefaultStyle.dark_text;
   }
   bool ShowKeybind() const override {
      return false;
   }
};

struct SeparatorMode : public EnumeratedMode {
   enum class Mode { kNone, kThree, kFour, kEight };
   Mode mode = Mode::kNone;
   char const* DisplayString() const override {
      switch(mode) {
      case Mode::kNone:
         return "no sep";
         break;
      case Mode::kThree:
         return "sep3";
         break;
      case Mode::kFour:
         return "sep4";
         break;
      case Mode::kEight:
         return "sep8";
         break;
      }
      return "";
   }
   char const* KeybindString() const override {
      return "s";
   }
   void Rotate() override {
      EnumRotate(mode, Mode::kEight);
   }

   int ToNumDigits() {
      switch(mode) {
      case Mode::kNone:
         return 0;
         break;
      case Mode::kThree:
         return 3;
         break;
      case Mode::kFour:
         return 4;
         break;
      case Mode::kEight:
         return 8;
         break;
      }
      return 0;
   }
};

struct NumericDisplayMode : public EnumeratedMode {
   NumericDisplayMode(char const* keybind_string) : m_keybind_string(keybind_string) {}
   using Mode = intbase::IntBase;
   Mode mode = Mode::kDec;
   char const* m_keybind_string;
   char const* DisplayString() const override {
      return intbase::as_string(mode);
   }
   char const* KeybindString() const override {
      return m_keybind_string;
   }
   void Rotate() override {
      EnumRotate(mode, Mode::kBin);
   }

   Color TextColor() const override {
      return to_dark_text_color(mode);
   }
};

struct IntWidthMode : public EnumeratedMode {
   enum class Mode { k8, k16, k32, k64 };
   Mode mode = Mode::k64;
   char const* DisplayString() const override {
      switch(mode) {
      case Mode::k8:
         return "w=8";
         break;
      case Mode::k16:
         return "w=16";
         break;
      case Mode::k32:
         return "w=32";
         break;
      case Mode::k64:
         return "w=64";
         break;
      }
      return "";
   }
   char const* KeybindString() const override {
      return "w";
   }
   void Rotate() override {
      EnumRotate(mode, Mode::k64);
   }
   int ToBits() {
      switch(mode) {
      case Mode::k8:
         return 8;
      case Mode::k16:
         return 16;
      case Mode::k32:
         return 32;
      case Mode::k64:
         return 64;
      }
      return 0;
   }
};

struct FixMode : public EnumeratedMode {
   enum class Mode { kInfix, kPostfix };
   Mode mode = Mode::kPostfix;
   char const* DisplayString() const override {
      switch(mode) {
      case Mode::kInfix:
         return "infix";
         break;
      case Mode::kPostfix:
         return "rpn";
         break;
      }
      return "";
   }
   char const* KeybindString() const override {
      return "r";
   }
   void Rotate() override {
      EnumRotate(mode, Mode::kPostfix);
   }
};

// temp for test
extern RegisterDisplay test_register;

class ViewModel {
public:
   std::string current_input;
   std::vector<parse::Token> parsed;
   int highlighted_index = 0;

   calc::State state;
   std::vector<std::string> history;

   int history_highlighted_index = 0;

   EditorMode editor_mode;
   NumericDisplayMode input_display{"z"};
   NumericDisplayMode output_display{"x"};
   SeparatorMode sep_mode;
   IntWidthMode int_width;
   FixMode fix_mode;

   RegisterDisplay const& currentRegister = test_register;

   void OnCharPressed(int chr);
   void OnKeyPressed(KeyboardKey k);

   std::string GetStackDisplayString(int index);

private:
   void OnInputChanged(bool reset_history_highlight);
   void OnCommit();
   void OnHistoryHighlightChanged();
};