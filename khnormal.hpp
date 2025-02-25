#pragma once
#include <string_view>

namespace khnormal {
  std::string khnormalize(std::string_view v);
  std::string normalize(std::string_view text);
}