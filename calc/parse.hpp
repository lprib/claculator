#pragma once

#include "calc/intbase.hpp"

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
static constexpr BuiltinOperation kDefaultBuiltinOperation = BuiltinOperation::kAdd;

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
         case BuiltinOperation::kIntDiv:
            o << "//";
            break;
         case BuiltinOperation::kMod:
            o << "%";
            break;
         case BuiltinOperation::kAnd:
            o << "&";
            break;
         case BuiltinOperation::kOr:
            o << "|";
            break;
         case BuiltinOperation::kXor:
            o << "^";
            break;
         case BuiltinOperation::kInv:
            o << "~";
            break;
         case BuiltinOperation::kShr:
            o << ">>";
            break;
         case BuiltinOperation::kShl:
            o << "<<";
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

struct ParserSettings {
   ParserSettings(intbase::IntBase _default_numeric_base) :
      default_numeric_base(_default_numeric_base) {}

   intbase::IntBase default_numeric_base;
};

std::vector<Token> parse(ParserSettings const& settings, std::string_view input);

void unit_test();

} // namespace parse