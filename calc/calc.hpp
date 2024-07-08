#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "calc/function.hpp"
#include "calc/parse.hpp"

namespace calc {

struct Stack {
   std::vector<std::int64_t> data;

   std::int64_t pop() {
      auto ret = data.back();
      data.pop_back();
      return ret;
   }

   void push(std::int64_t n) {
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

   void Speculate(std::vector<parse::Token>& tokens);
   void Commit();

private:
   void SpeculateToken(parse::Token& token);
   void PoisionSpeculation();
   bool CheckSpecStackSize(std::size_t size);
};

} // namespace calc
