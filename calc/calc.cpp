
#include "calc/calc.hpp"

#include <variant>

namespace calc {
State::State() :
   functions{std::unique_ptr<SimpleBinaryArithmeticFunction>{
      new SimpleBinaryArithmeticFunction("+", [](auto a, auto b) { return a + b; })
   }} {}

void State::Speculate(std::vector<parse::Token>& tokens) {
   speculate_poisoned = false;
   speculative_stack = committed_stack;
   for(auto& token : tokens) {
      SpeculateToken(token);
   }
}
void State::Commit() {
   committed_stack = speculative_stack;
}

bool State::CheckSpecStackSize(std::size_t size) {
   if(speculative_stack.data.size() < size) {
      speculate_poisoned = true;
      return false;
   } else {
      return true;
   }
}

void State::SpeculateToken(parse::Token& token) {
   if(speculate_poisoned) {
      return;
   }

   switch(token.type) {
   case parse::TokenType::kDecimalNumber:
   case parse::TokenType::kHexNumber:
   case parse::TokenType::kBinaryNumber:
      speculative_stack.push(token.number);
      break;
   case parse::TokenType::kBuiltin:
      switch(token.builtin) {

#define BINOP_CASE(_token_enum, _op) \
   case parse::BuiltinOperation::_token_enum: { \
      if(!CheckSpecStackSize(2)) { \
         token.into_error("stack underflow"); \
         return; \
      } \
      auto r = speculative_stack.pop(); \
      auto l = speculative_stack.pop(); \
      speculative_stack.push(l _op r); \
   } break

         BINOP_CASE(kAdd, +);
         BINOP_CASE(kSub, -);
         BINOP_CASE(kMul, *);
         BINOP_CASE(kDiv, /); // todo
         BINOP_CASE(kIntDiv, /);
         BINOP_CASE(kMod, %);
         BINOP_CASE(kAnd, &);
         BINOP_CASE(kOr, |);
         BINOP_CASE(kXor, ^);
         BINOP_CASE(kShr, >>);
         BINOP_CASE(kShl, <<);

      case parse::BuiltinOperation::kInv: {
         if(!CheckSpecStackSize(1)) {
            token.into_error("stack underflow");
            speculate_poisoned = true;
            return;
         }
         speculative_stack.push(~speculative_stack.pop());
      }
      }

      break;
   case parse::TokenType::kWord:
      speculate_poisoned = true; // todo
      break;
   case parse::TokenType::kError:
      speculate_poisoned = true;
      break;
   }
}
} // namespace calc
