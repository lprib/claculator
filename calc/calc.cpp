
#include "calc/calc.hpp"
#include "calc/value.hpp"

#include <format>
#include <iostream>
#include <iterator>
#include <memory>
#include <variant>

namespace calc {

class DivideFunction : public BinaryArithmeticFunction {
public:
   DivideFunction() : BinaryArithmeticFunction("/") {}
   Function::ExecutionResult execute(std::vector<Value> input) override {
      // TODO support doubles
      if(input[1].as_int() == 0) {
         return ExecutionResult::make_error("div by zero");
      }
      return ExecutionResult::make_success(
         std::vector<Value>{Value(int64_t{input[0].as_int() / input[1].as_int()})}
      );
   }
};

class DropFunction : public BuiltinNormalFunction {
public:
   DropFunction() : BuiltinNormalFunction(1, "drop") {}
   Function::ExecutionResult execute(std::vector<Value> input) override {
      return ExecutionResult::make_success();
   }
};
class DupFunction : public BuiltinNormalFunction {
public:
   DupFunction() : BuiltinNormalFunction(1, "dup") {}
   Function::ExecutionResult execute(std::vector<Value> input) override {
      return ExecutionResult::make_success(std::vector<Value>{input[0], input[0]});
   }
};

class Dup2Function : public BuiltinNormalFunction {
public:
   Dup2Function() : BuiltinNormalFunction(2, "dup2") {}
   Function::ExecutionResult execute(std::vector<Value> input) override {
      return ExecutionResult::make_success(
         std::vector<Value>{input[0], input[1], input[0], input[1]}
      );
   }
};

class SwapFunction : public BuiltinNormalFunction {
public:
   SwapFunction() : BuiltinNormalFunction(2, "swap") {}
   Function::ExecutionResult execute(std::vector<Value> input) override {
      return ExecutionResult::make_success(std::vector<Value>{input[1], input[0]});
   }
};

#define SIMPLE_BIN_OP(op) \
   fns.push_back(std::make_unique<calc::SimpleBinaryArithmeticFunction>(#op, [](auto a, auto b) { \
      return a op b; \
   }));

static std::vector<std::unique_ptr<calc::Function>> MakeBuiltinFunctions() {
   std::vector<std::unique_ptr<calc::Function>> fns;
   SIMPLE_BIN_OP(+);
   SIMPLE_BIN_OP(-);
   SIMPLE_BIN_OP(*);
   SIMPLE_BIN_OP(%);
   fns.push_back(std::make_unique<DivideFunction>());
   fns.push_back(std::make_unique<DropFunction>());
   fns.push_back(std::make_unique<Dup2Function>());
   fns.push_back(std::make_unique<DupFunction>());
   fns.push_back(std::make_unique<SwapFunction>());
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

void State::Execute(std::vector<parse::Token>& tokens, bool is_speculative) {
   speculate_poisoned = false;
   speculative_stack = committed_stack;
   for(auto& token : tokens) {
      ExecuteToken(token, is_speculative);
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

void State::ExecuteToken(parse::Token& token, bool is_speculative) {
   if(speculate_poisoned) {
      return;
   }

   switch(token.type) {
   case parse::TokenType::kDecimalNumber:
   case parse::TokenType::kHexNumber:
   case parse::TokenType::kBinaryNumber:
   case parse::TokenType::kDouble:
   case parse::TokenType::kString:
      speculative_stack.push(token.push_value);
      break;
   case parse::TokenType::kWord: {
      auto& fn = functions[token.function_index];
      if(is_speculative && !fn->allow_speculative_execution()) {
         token.additional_popup_text = " [enter to execute] ";
         PoisionSpeculation();
         return;
      }

      if(speculative_stack.data.size() < fn->arity()) {
         token.into_error(std::format(
            "stack underflow: require {}, got {}",
            fn->arity(),
            speculative_stack.data.size()
         ));
         PoisionSpeculation();
         return;
      }

      std::vector<Value> input;
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
