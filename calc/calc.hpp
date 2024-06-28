#pragma once

#include <cstdint>
#include <vector>

#include "calc/parse.hpp"

namespace calc {

class Cell {
public:
   Cell() : m_type(Type::kInt), m_data(int64_t{0}) {}

private:
   enum class Type { kInt, kDouble };
   union Data {
      int64_t i;
      double d;
   };

   Type m_type;
   Data m_data;
};

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

struct State {
   Stack committed_stack;
   Stack speculative_stack;
   bool speculate_poisoned = false;

   // void Speculate(std::vector<parse::Token> const& tokens);
   void Commit();

private:
   // void SpeculateToken(parse::Token const& token);
   bool CheckSpecStackSize(std::size_t size);
};

} // namespace calc
