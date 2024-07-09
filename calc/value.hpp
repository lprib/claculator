#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace calc {

class Value {
public:
   enum class Type { kInt, kDouble, kString };

   Value(int64_t x) : inner(x), typ(Type::kInt) {}
   Value(double x) : inner(x), typ(Type::kDouble) {}
   Value(std::string x) : inner(x), typ(Type::kString) {}

   Type type() const {
      return typ;
   }

   int64_t int_or_default() const {
      if(const int64_t* pval = std::get_if<int64_t>(&inner)) {
         return *pval;
      }
      return 0;
   }
   double double_or_default() const {
      if(const double* pval = std::get_if<double>(&inner)) {
         return *pval;
      }
      return 0.0;
   }
   std::string string_or_default() const {
      if(const std::string* pval = std::get_if<std::string>(&inner)) {
         return *pval;
      }
      return "";
   }

private:
   std::variant<int64_t, double, std::string> inner;
   Type typ;
};
} // namespace calc