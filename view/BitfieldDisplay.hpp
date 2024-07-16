#pragma once

#include "calc/bit_register.hpp"

class BitfieldDisplay {
public:
   static int height(RegisterDisplay const& display) {
      return display.fields.empty() ? 45 : 250;
   }
   static void render(
      int x, int y, RegisterDisplay const& display, int64_t value
   );
};