
#include "calc/calc.hpp"

#include <variant>

namespace calc {
// void State::Speculate(std::vector<parse::Token> const& tokens) {
//    speculate_poisoned = false;
//    speculative_stack = committed_stack;
//    for(auto const& token : tokens) {
//       SpeculateToken(token);
//    }
// }
void State::Commit() {
   committed_stack = speculative_stack;
}

bool State::CheckSpecStackSize(std::size_t size) {
   if(speculative_stack.data.size() < size) {
      speculate_poisoned = true;
      return false;
   } else {
      return true;
   }
}

// void State::SpeculateToken(parse::Token const& token) {
//    if(speculate_poisoned)
//       return;
//    std::visit(
//       [&](auto&& arg) {
//          using T = std::decay_t<decltype(arg)>;
//          if constexpr(std::is_same_v<T, std::int64_t>) {
//             speculative_stack.push(arg);
//          } else if constexpr(std::is_same_v<T, parse::BuiltinToken>) {
//             switch(arg) {
//             case parse::BuiltinToken::kAdd: {
//                if(!CheckSpecStackSize(2))
//                   return;
//                auto r = speculative_stack.pop();
//                auto l = speculative_stack.pop();
//                speculative_stack.push(l + r);
//             } break;
//             case parse::BuiltinToken::kSub: {
//                if(!CheckSpecStackSize(2))
//                   return;
//                auto r = speculative_stack.pop();
//                auto l = speculative_stack.pop();
//                speculative_stack.push(l - r);
//             } break;
//             case parse::BuiltinToken::kMul: {
//                if(!CheckSpecStackSize(2))
//                   return;
//                auto r = speculative_stack.pop();
//                auto l = speculative_stack.pop();
//                speculative_stack.push(l * r);
//             } break;
//             case parse::BuiltinToken::kDiv: {
//                if(!CheckSpecStackSize(2))
//                   return;
//                auto r = speculative_stack.pop();
//                auto l = speculative_stack.pop();
//                speculative_stack.push(l / r);
//             } break;
//             default:
//                break;
//             }
//          } else if constexpr(std::is_same_v<T, parse::TokenParseError>) {
//             speculate_poisoned = true;
//          } else
//             static_assert(false, "non-exhaustive visitor!");
//       },
//       token.value
//    );
// }
} // namespace calc
