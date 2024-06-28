#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace parse {

enum class TokenType {
   kDecimalNumber,
   kHexNumber,
   kBinaryNumber,
   kBuiltin,
   kWord,
   kError,
};

enum class BuiltinOperation { kAdd, kSub, kMul, kDiv };
static constexpr BuiltinOperation kDefaultBuiltinOperation = BuiltinOperation::kAdd;

class Token {
public:
   static Token make_integer(size_t start, size_t end, int base, int64_t n) {
      TokenType type = TokenType::kDecimalNumber;
      switch(base) {
      case 2:
         type = TokenType::kBinaryNumber;
         break;
      case 10:
         type = TokenType::kDecimalNumber;
         break;
      case 16:
         type = TokenType::kHexNumber;
         break;
      default:
         break;
      }
      return Token(start, end, type, kDefaultBuiltinOperation, n);
   }
   static Token make_builtin(size_t start, size_t end, BuiltinOperation builtin) {
      return Token(start, end, TokenType::kBuiltin, builtin, 0);
   }
   static Token make_word(size_t start, size_t end, std::string_view word) {
      return Token(start, end, TokenType::kWord, kDefaultBuiltinOperation, 0, std::string(word));
   }
   static Token make_error(size_t start, size_t end, std::string text) {
      return Token(start, end, TokenType::kError, kDefaultBuiltinOperation, 0, std::move(text));
   }

   size_t start;
   size_t end;
   TokenType type;

   BuiltinOperation builtin;
   int64_t number;
   std::string text;

   size_t length() {
      return end - start;
   }

   friend std::ostream& operator<<(std::ostream& o, Token const& tok) {
      switch(tok.type) {
      case TokenType::kDecimalNumber:
         o << "dec:" << tok.number;
         break;
      case TokenType::kHexNumber:
         o << "hex:" << tok.number;
         break;
      case TokenType::kBinaryNumber:
         o << "bin:" << tok.number;
         break;
      case TokenType::kBuiltin:
         o << "builtin:";
         switch(tok.builtin) {
         case BuiltinOperation::kAdd:
            o << "+";
            break;
         case BuiltinOperation::kSub:
            o << "-";
            break;
         case BuiltinOperation::kMul:
            o << "*";
            break;
         case BuiltinOperation::kDiv:
            o << "/";
            break;
         }
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
      size_t _start, size_t _end, TokenType _type, BuiltinOperation _builtin, int64_t _number,
      std::string _text = ""
   ) :
      start(_start),
      end(_end),
      type(_type),
      builtin(_builtin),
      number(_number),
      text(_text) {}
};

enum class DefaultNumericBase { kDec, kHex, kBin };

struct ParserSettings {
   ParserSettings(DefaultNumericBase _default_numeric_base) :
      default_numeric_base(_default_numeric_base) {}

   DefaultNumericBase default_numeric_base;
};

std::vector<Token> parse(ParserSettings const& settings, std::string_view input);

void unit_test();

} // namespace parse