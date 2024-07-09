#pragma once

#include "calc/value.hpp"

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
      std::vector<Value> returns;
      std::string error;

      static ExecutionResult make_success(std::vector<Value> returns) {
         return ExecutionResult(false, std::move(returns), "");
      }
      static ExecutionResult make_error(std::string error) {
         return ExecutionResult(true, std::vector<Value>(), std::move(error));
      }
   };

   virtual ExecutionResult execute(std::vector<Value> input) = 0;
};

class BuiltinNormalFunction : public Function {
public:
   BuiltinNormalFunction(size_t _arity, std::string_view _name) : m_arity(_arity), m_name(_name) {}
   std::string_view name() const override {
      return m_name;
   }

   size_t arity() const override {
      return m_arity;
   }

   bool super_precedence() const {
      return false;
   }

private:
   size_t m_arity;
   std::string_view m_name;
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

   ExecutionResult execute(std::vector<Value> input) override {
      // todo support floats
      if((input[0].type() != Value::Type::kInt) || (input[1].type() != Value::Type::kInt)) {
         return ExecutionResult::make_error("require two integer args");
      }
      return ExecutionResult::make_success(
         std::vector<Value>{Value(m_fn(input[0].int_or_default(), input[1].int_or_default()))}
      );
   }

private:
   FunctionType m_fn;
};

} // namespace calc
