#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "calc/function.hpp"
#include "calc/parse.hpp"
#include "calc/value.hpp"

namespace calc {

struct Stack {
   std::vector<Value> data;

   Value pop() {
      auto ret = data.back();
      data.pop_back();
      return ret;
   }

   void push(Value n) {
      data.push_back(n);
   }
};

class State {
public:
   State();
   Stack committed_stack;
   Stack speculative_stack;
   bool speculate_poisoned = false;

   std::vector<std::unique_ptr<Function>> functions;

   void Execute(std::vector<parse::Token>& tokens, bool is_speculative);
   void Commit();

private:
   void ExecuteToken(parse::Token& token, bool is_speculative);
   void PoisionSpeculation();
   bool CheckSpecStackSize(std::size_t size);
};

} // namespace calc
