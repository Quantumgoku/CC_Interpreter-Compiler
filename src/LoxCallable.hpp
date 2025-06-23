#pragma once
#include <vector>
#include "token.hpp"
#include "Environment.hpp"
#include "literal.hpp"

class Interpreter; // Forward declaration

class LoxCallable {
public:
    virtual ~LoxCallable() {} // Ensure proper cleanup of derived classes
    virtual size_t arity() const = 0;
    virtual lox_literal call(const Interpreter& interpreter, const std::vector<lox_literal>& arguments) = 0;
    virtual std::string toString() const = 0;
};