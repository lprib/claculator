#pragma once

#include <iostream>
#include <string>
#include <vector>

enum class FieldDisplay { kNumeric, kEnum };

class Field {
public:
   constexpr Field(
      int _firstbit, int _lastbit, std::string _name, FieldDisplay _display
   ) :
      firstbit(_firstbit),
      lastbit(_lastbit),
      name(std::move(_name)),
      display(_display) {}

   int firstbit;
   int lastbit;
   std::string name;
   FieldDisplay display;

   std::string GetDisplay(int64_t value) const {
      int64_t mask =
         (1ul << ((int64_t)lastbit - (int64_t)firstbit + 1ul)) - 1ul;
      int64_t field_value = ((value >> (int64_t)firstbit) & mask);
      return std::to_string(field_value);
   }
};

class RegisterDisplay {
public:
   constexpr RegisterDisplay(std::vector<Field> _fields) :
      fields(std::move(_fields)) {}
   std::vector<Field> fields;
};