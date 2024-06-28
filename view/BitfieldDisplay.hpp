#pragma once

#include "calc/bit_register.hpp"

class BitfieldDisplay {
public:
   static void render(
      int x, int y, RegisterDisplay const& display, int64_t value
   );
};