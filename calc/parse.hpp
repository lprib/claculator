#pragma once

#include "calc/function.hpp"
#include "calc/intbase.hpp"
#include "calc/value.hpp"
#include "text.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace parse {

enum class TokenType { kDecimalNumber, kHexNumber, kBinaryNumber, kDouble, kString, kWord, kError };

enum class BuiltinOperation {
   kAdd,
   kSub,
   kMul,
   kDiv,
   kIntDiv,
   kMod,
   kAnd,
   kOr,
   kXor,
   kInv,
   kShr,
   kShl
};

class Token {
public:
   static Token make_integer(size_t start, size_t end, intbase::IntBase base, int64_t n) {
      TokenType type = TokenType::kDecimalNumber;
      switch(base) {
      case intbase::IntBase::kBin:
         type = TokenType::kBinaryNumber;
         break;
      case intbase::IntBase::kDec:
         type = TokenType::kDecimalNumber;
         break;
      case intbase::IntBase::kHex:
         type = TokenType::kHexNumber;
         break;
      default:
         break;
      }
      return Token(start, end, type, n, 0, "");
   }
   static Token make_double(size_t start, size_t end, double n) {
      return Token(start, end, TokenType::kDouble, n, 0, "");
   }
   static Token make_string(size_t start, size_t end, std::string n) {
      return Token(start, end, TokenType::kString, n, 0, "");
   }
   static Token make_word(size_t start, size_t end, int index, std::string_view word) {
      return Token(start, end, TokenType::kWord, calc::Value(int64_t{0}), index, std::string(word));
   }
   static Token make_error(size_t start, size_t end, std::string text) {
      return Token(start, end, TokenType::kError, calc::Value(int64_t{0}), 0, std::move(text));
   }

   void into_error(std::string error_text) {
      *this = Token::make_error(span.start, span.end, std::move(error_text));
   }

   TextSpan span;
   TokenType type;

   /// @brief used for kDecimalNumber, kHexNumber, kBinaryNumber
   calc::Value push_value;
   /// @brief used for kWord
   size_t function_index;
   /// @brief used for kWord, kError
   std::string text;

   /// @brief May be set to show additional popup for this token in the UI
   std::string additional_popup_text;

   size_t length() {
      return span.end - span.start;
   }

   bool is_integer() {
      switch(type) {
      case TokenType::kDecimalNumber:
      case TokenType::kHexNumber:
      case TokenType::kBinaryNumber:
         return true;
      default:
         return false;
      }
   }

   friend std::ostream& operator<<(std::ostream& o, Token const& tok) {
      switch(tok.type) {
      case TokenType::kDecimalNumber:
         o << "dec:" << tok.push_value.as_int();
         break;
      case TokenType::kHexNumber:
         o << "hex:" << tok.push_value.as_int();
         break;
      case TokenType::kBinaryNumber:
         o << "bin:" << tok.push_value.as_int();
         break;
      case TokenType::kDouble:
         o << "double:" << tok.push_value.as_double();
         break;
      case TokenType::kString:
         o << "string:\"" << tok.push_value.as_string() << "\"";
         break;
      case TokenType::kWord:
         o << "word:" << tok.text;
         break;
      case TokenType::kError:
         o << "error:" << tok.text;
         break;
      }
      return o;
   }

private:
   Token(
      size_t _start, size_t _end, TokenType _type, calc::Value _push_value, size_t _function_index,
      std::string _text
   ) :
      span(_start, _end),
      type(_type),
      push_value(_push_value),
      function_index(_function_index),
      text(_text) {}
};

struct ParserSettings {
   ParserSettings(
      intbase::IntBase _default_numeric_base,
      std::vector<std::unique_ptr<calc::Function>> const& _functions
   ) :
      default_numeric_base(_default_numeric_base),
      functions(_functions) {}

   intbase::IntBase default_numeric_base;
   std::vector<std::unique_ptr<calc::Function>> const& functions;
};

std::vector<Token> parse(ParserSettings const& settings, std::string_view input);

void unit_test();

} // namespace parse