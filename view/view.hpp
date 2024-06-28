#pragma once

#include "viewmodel.hpp"

class View {
public:
   View(ViewModel& vm) : m_vm(vm) {}
   void render();

private:
   ViewModel& m_vm;

   void render_main_input();
   void render_state_infobar();
   void render_stack();
   void render_history();

   void draw_bits(int y, int bitwidth, int64_t value);
};
