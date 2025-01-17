#include "raylib.h"

#include "view/view.hpp"
#include "controller.hpp"

#include <iostream>
#include <string_view>
using namespace std::literals;

int main(void) {
   Controller viewmodel;
   View view(viewmodel);

   const int screenWidth = 850;
   const int screenHeight = 450;
   SetConfigFlags(FLAG_WINDOW_RESIZABLE);
   InitWindow(
      screenWidth,
      screenHeight,
      "claculator"
   );
   SetWindowMinSize(screenWidth, screenHeight);
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