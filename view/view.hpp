#pragma once

#include "controller.hpp"

class View {
public:
   View(Controller& controller) : m_controller(controller) {}
   void render();

private:
   Controller& m_controller;

   void render_main_input();
   void render_state_infobar();
   void render_stack();
   void render_history();

   void draw_bits(int y, int bitwidth, int64_t value);
};
