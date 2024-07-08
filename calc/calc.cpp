
#include "calc/calc.hpp"

#include <format>
#include <iostream>
#include <iterator>
#include <memory>
#include <variant>

namespace calc {

static std::vector<std::unique_ptr<calc::Function>> MakeBuiltinFunctions() {
   std::vector<std::unique_ptr<calc::Function>> fns;
   fns.push_back(std::make_unique<calc::SimpleBinaryArithmeticFunction>("+", [](auto a, auto b) {
      return a + b;
   }));
   return fns;
}

State::State() {
   auto builtins = MakeBuiltinFunctions();
   functions.insert(
      functions.end(),
      std::make_move_iterator(builtins.begin()),
      std::make_move_iterator(builtins.end())
   );
}

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

void State::PoisionSpeculation() {
   speculate_poisoned = true;
}

bool State::CheckSpecStackSize(std::size_t size) {
   if(speculative_stack.data.size() < size) {
      PoisionSpeculation();
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
   case parse::TokenType::kWord: {
      auto& fn = functions[token.function_index];
      if(speculative_stack.data.size() < fn->arity()) {
         token.into_error(std::format(
            "stack underflow: require {}, got {}",
            speculative_stack.data.size(),
            fn->arity()
         ));
         PoisionSpeculation();
         return;
      }

      std::vector<std::int64_t> input;
      size_t index = speculative_stack.data.size() - fn->arity();
      for(size_t i = 0; i < fn->arity(); ++i) {
         input.push_back(std::move(speculative_stack.data[index]));
         speculative_stack.data.erase(speculative_stack.data.begin() + index);
      }
      auto results = fn->execute(input);
      if(results.is_error) {
         token.into_error(std::move(results.error));
         PoisionSpeculation();
      } else {
         for(auto& result : results.returns) {
            speculative_stack.push(std::move(result));
         }
      }
   } break;
   case parse::TokenType::kError:
      PoisionSpeculation();
      break;
   }
}
} // namespace calc
