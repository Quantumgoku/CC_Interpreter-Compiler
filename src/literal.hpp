#pragma once
#include <variant>
#include <string>
#include <memory>

class LoxCallable; // Forward declaration only
class LoxInstance; // Forward declaration for use in lox_literal
using lox_literal = std::variant<
    std::monostate, 
    double, 
    std::string, 
    bool, 
    std::shared_ptr<LoxCallable>,
    std::shared_ptr<LoxInstance>
>;
