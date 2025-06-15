#pragma once
#include <variant>
#include <string>
#include <memory>

class LoxCallable;
using literal = std::variant<std::monostate, std::string, double, bool, std::shared_ptr<LoxCallable>>;
