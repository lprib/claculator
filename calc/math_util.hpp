#pragma once

#include <cstdint>
#include <limits.h>

// TODO seems to be broken?
inline bool does_mul_overflow(int64_t a, int64_t b) {
   uint64_t aa = static_cast<uint64_t>(a);
   uint64_t bb = static_cast<uint64_t>(b);

   uint64_t x = a * b;
   return (aa != 0) && (x / aa != bb);
}

// TODO seems to be broken?
inline bool does_add_overflow(int64_t a, int64_t b) {
   if(b > 0 && a > INT64_MAX - b)
      return true;
   if(b < 0 && a < INT64_MAX - b)
      return true;
   return false;
}