#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

namespace calc {
class Function {
public:
   virtual std::string_view name() const = 0;
   virtual int arity() const = 0;
   /// @brief If true, this function should always be parsed, even if it is
   /// directly adjacent to integers or any other function names
   virtual bool super_precedence() const {
      return false;
   }
   virtual std::vector<std::int64_t> execute(std::vector<std::int64_t> input) = 0;
};

class SimpleBinaryArithmeticFunction : public Function {
public:
   using FunctionType = std::int64_t (*)(std::int64_t, std::int64_t);

   SimpleBinaryArithmeticFunction(char const* name, FunctionType fn) : m_name(name), m_fn(fn) {}

   std::string_view name() const override {
      return m_name;
   }

   int arity() const override {
      return 2;
   }

   bool super_precedence() const {
      return true;
   }

   std::vector<std::int64_t> execute(std::vector<std::int64_t> input) override {
      return std::vector<std::int64_t>(m_fn(input[0], input[1]));
   }

private:
   char const* m_name;
   FunctionType m_fn;
};

} // namespace calc
