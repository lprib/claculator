#pragma once

namespace intbase {
enum class IntBase { kDec, kHex, kBin };
inline int as_int(IntBase base) {
   switch(base) {
   case IntBase::kDec:
      return 10;
   case IntBase::kHex:
      return 16;
   case IntBase::kBin:
      return 2;
   default:
      return 10;
   }
}

inline char const* as_string(IntBase base) {
   switch(base) {
   case IntBase::kDec:
      return "dec";
   case IntBase::kHex:
      return "hex";
   case IntBase::kBin:
      return "bin";
   default:
      return "";
   }
}
} // namespace intbase
