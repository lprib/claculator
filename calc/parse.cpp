#include "calc/parse.hpp"
#include "calc/math_util.hpp"

#include <cassert>
#include <iostream>
#include <limits>
#include <optional>

using namespace std::literals;

namespace parse {

static char to_lower(char upper) {
   if(upper >= 'A' && upper <= 'Z') {
      return upper - 'A' + 'a';
   } else {
      return upper;
   }
}

struct Parser {
   std::string_view input;
   size_t current_index;
   ParserSettings const& settings;

   Parser(std::string_view _input, ParserSettings const& _settings) :
      input(_input),
      current_index(0),
      settings(_settings) {}

   std::string_view remaining() {
      return input.substr(current_index);
   }

   char next() {
      return input[current_index];
   }

   bool prefix(std::string_view pre) {
      if(remaining().substr(0, pre.size()) == pre) {
         current_index += pre.size();
         return true;
      } else {
         return false;
      }
   }

   static constexpr std::string_view kWhitespaceChars = " \t\r\n";
   bool skip_whitespace() {
      bool is_ws = false;
      while((current_index < input.size()) &&
            kWhitespaceChars.find(next()) != std::string_view::npos) {
         ++current_index;
         is_ws = true;
      }
      return is_ws;
   }

   std::optional<Token> word() {
      int n_chars = 0;
      while(((current_index + n_chars) < input.size()) &&
            (kWhitespaceChars.find(remaining()[n_chars]) == std::string_view::npos)) {
         // stop early for super precedence
         if(super_precedence_word(true, n_chars).has_value()) {
            break;
         }
         ++n_chars;
      }
      if(n_chars == 0) {
         return std::nullopt;
      }

      auto tok = Token::make_error(current_index, current_index + n_chars, "undefined word");

      for(size_t i = 0; i < settings.functions.size(); ++i) {
         if(settings.functions[i]->name() == remaining().substr(0, n_chars)) {
            tok = Token::make_word(
               current_index,
               current_index + n_chars,
               i,
               remaining().substr(0, n_chars)
            );
            break;
         }
      }

      current_index += n_chars;
      return tok;
   }

   static constexpr std::string_view kNumericChars = "0123456789abcdef";
   std::optional<Token> number(intbase::IntBase base) {
      size_t int_base = static_cast<size_t>(intbase::as_int(base));
      if(int_base > kNumericChars.size()) {
         return std::nullopt;
      }

      std::string_view base_chars = kNumericChars.substr(0, int_base);

      int n_chars = 0;
      while(((current_index + n_chars) < input.size()) &&
            base_chars.find(to_lower(remaining()[n_chars])) != std::string_view::npos) {
         ++n_chars;
      }
      if(n_chars == 0) {
         return std::nullopt;
      }

      int64_t number = 0;
      int64_t digit_mul = 1;
      for(int i = n_chars - 1; i >= 0; --i) {
         int64_t digit = base_chars.find(to_lower(remaining()[i]));
         if(does_mul_overflow(digit, digit_mul)) {
            auto tok = Token::make_error(current_index, current_index + n_chars, "overflow");
            current_index += n_chars;
            return tok;
         }
         if(does_add_overflow(number, digit * digit_mul)) {
            auto tok = Token::make_error(current_index, current_index + n_chars, "overflow");
            current_index += n_chars;
            return tok;
         }
         number += digit * digit_mul;

         if(does_mul_overflow(digit_mul, int_base)) {
            auto tok = Token::make_error(current_index, current_index + n_chars, "overflow");
            current_index += n_chars;
            return tok;
         }
         digit_mul *= int_base;
      }

      auto tok = Token::make_integer(current_index, current_index + n_chars, base, number);
      current_index += n_chars;
      return tok;
   }

   std::optional<Token> default_number() {
      return number(settings.default_numeric_base);
   }

   std::optional<Token> generic_prefixed_number(std::string_view pre, intbase::IntBase base) {
      if(prefix(pre)) {
         return number(base);
      } else {
         return std::nullopt;
      }
   }

   std::optional<Token> prefixed_hex_number() {
      return generic_prefixed_number("0x"sv, intbase::IntBase::kHex);
   }

   std::optional<Token> prefixed_bin_number() {
      return generic_prefixed_number("0b"sv, intbase::IntBase::kBin);
   }

   std::optional<Token> prefixed_dec_number() {
      return generic_prefixed_number("0i"sv, intbase::IntBase::kDec);
   }

   std::optional<Token> test_for_super_precedence(std::string_view c, size_t start) {
      for(size_t i = 0; i < settings.functions.size(); ++i) {
         if(settings.functions[i]->super_precedence()) {
            auto const& name = settings.functions[i]->name();
            if(c.rfind(name, 0) == 0) {
               return Token::make_word(start, start + name.size(), i, c.substr(0, name.size()));
            }
         }
      }
      return std::nullopt;
   }

   std::optional<Token> super_precedence_word(bool lookahead, int offset = 0) {
      auto index_offset = current_index + offset;
      auto tok = test_for_super_precedence(input.substr(current_index + offset), index_offset);
      if(lookahead) {
         return tok;
      } else {
         if(tok.has_value()) {
            current_index += tok->length();
         }
         return tok;
      }
   }

   std::optional<Token> next_token() {
      (void)skip_whitespace();

      auto maybe = super_precedence_word(false);
      if(maybe.has_value()) {
         return *maybe;
      }
      maybe = prefixed_hex_number();
      if(maybe.has_value()) {
         return *maybe;
      }
      maybe = prefixed_bin_number();
      if(maybe.has_value()) {
         return *maybe;
      }
      maybe = prefixed_dec_number();
      if(maybe.has_value()) {
         return *maybe;
      }
      maybe = default_number();
      if(maybe.has_value()) {
         return *maybe;
      }

      return word();
   }

   std::vector<Token> parse() {
      auto vec = std::vector<Token>();
      auto tok = next_token();
      while(tok.has_value()) {
         vec.push_back(*tok);
         tok = next_token();
      }
      return vec;
   }
};

std::vector<Token> parse(ParserSettings const& settings, std::string_view input) {
   return Parser(input, settings).parse();
}

void unit_test() {
   std::cout << Parser("69420", ParserSettings(intbase::IntBase::kDec, {}))
                   .number(intbase::IntBase::kDec)
                   ->number
             << "\n";
   assert(
      Parser("69420", ParserSettings(intbase::IntBase::kDec, {}))
         .number(intbase::IntBase::kDec)
         ->number == 69420
   );
   assert(
      Parser("12AB34CD56EF", ParserSettings(intbase::IntBase::kDec, {}))
         .number(intbase::IntBase::kHex)
         ->number == 0x12AB34CD56EFLL
   );
   assert(
      Parser("100101001110111010111011", ParserSettings(intbase::IntBase::kDec, {}))
         .number(intbase::IntBase::kBin)
         ->number == 0b100101001110111010111011LL
   );

   auto settings = ParserSettings(intbase::IntBase::kDec, {});
   auto result = parse(settings, "123 0xff 0b1000 word*    3 4* 5 6<<>> abc//abc");
   for(auto token : result) {
      std::cout << token << " ";
   }
   std::cout << "\n";
}

} // namespace parse