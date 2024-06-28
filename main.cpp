#include "raylib.h"

#include "view/view.hpp"
#include "viewmodel.hpp"

#include <iostream>
#include <string_view>
using namespace std::literals;

int main(void) {
   ViewModel viewmodel;
   View view(viewmodel);

   const int screenWidth = 800;
   const int screenHeight = 450;
   InitWindow(
      screenWidth,
      screenHeight,
      "raylib [core] example - basic window"
   );
   SetTargetFPS(60);
   SetExitKey(0);
   while(!WindowShouldClose()) {
      viewmodel.OnCharPressed(GetCharPressed());

      auto key = static_cast<KeyboardKey>(GetKeyPressed());
      viewmodel.OnKeyPressed(key);

      BeginDrawing();
      ClearBackground(WHITE);
      view.render();
      EndDrawing();
   }
   CloseWindow();
   return 0;
}