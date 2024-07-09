#pragma once
#include <cstddef>
#include <string_view>

struct TextSpan {
   size_t start;
   size_t end;
   TextSpan(size_t _start, size_t _end) : start(_start), end(_end) {}

   std::string_view view(std::string_view parent) const {
      return parent.substr(start, end - start);
   }

   bool contains(size_t index) const {
      return (index >= start) && (index < end);
   }
};

namespace {
static constexpr std::string_view kWhitespaceChars = " \t\r\n";
}

inline bool IsWhitespace(char c) {
   return kWhitespaceChars.find(c) != std::string_view::npos;
}