#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace calc {
class Function {
public:
   virtual std::string_view name() const = 0;
   virtual size_t arity() const = 0;
   /// @brief If true, this function should always be parsed, even if it is
   /// directly adjacent to integers or any other function names
   virtual bool super_precedence() const {
      return false;
   }

   struct ExecutionResult {
      bool is_error;
      std::vector<std::int64_t> returns;
      std::string error;

      static ExecutionResult make_success(std::vector<std::int64_t> returns) {
         return ExecutionResult(false, std::move(returns), "");
      }
      static ExecutionResult make_error(std::string error) {
         return ExecutionResult(true, std::vector<std::int64_t>(), std::move(error));
      }
   };

   virtual ExecutionResult execute(std::vector<std::int64_t> input) = 0;
};

class BinaryArithmeticFunction : public Function {
public:
   BinaryArithmeticFunction(char const* name) : m_name(name) {}

   std::string_view name() const override {
      return m_name;
   }

   size_t arity() const override {
      return 2;
   }

   bool super_precedence() const {
      return true;
   }

private:
   char const* m_name;
};

class SimpleBinaryArithmeticFunction : public BinaryArithmeticFunction {
public:
   using FunctionType = std::int64_t (*)(std::int64_t, std::int64_t);

   SimpleBinaryArithmeticFunction(char const* name, FunctionType fn) :
      BinaryArithmeticFunction(name),
      m_fn(fn) {}

   ExecutionResult execute(std::vector<std::int64_t> input) override {
      return ExecutionResult::make_success(std::vector<std::int64_t>{m_fn(input[0], input[1])});
   }

private:
   FunctionType m_fn;
};

} // namespace calc
