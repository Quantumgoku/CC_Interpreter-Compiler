#pragma once
#include <string>
#include <memory>
#include <vector>
#include "token.hpp"
#include "Environment.hpp"
#include "literal.hpp"

class Interpreter; // Forward declaration

class LoxCallable {
public:
    virtual ~LoxCallable() = default;

    virtual std::string toString() const = 0;

    virtual size_t arity() const = 0;

    virtual literal call(const Interpreter& interpreter, const std::vector<literal>& arguments) const = 0;

    //virtual std::shared_ptr<LoxCallable> bind(const std::shared_ptr<Environment>& environment) const = 0;
};