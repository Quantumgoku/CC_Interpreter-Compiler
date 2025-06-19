#pragma once
#include "token.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "literal.hpp"
#include<unordered_map>
#include "LoxCallable.hpp"

class LoxClass : public LoxCallable {
public:

    LoxClass(const std::string& name) : name(name) {}

    std::string getName() const {
        return name;
    }

    std::string toString() const override { return name; }
    size_t arity() const override { return 0; }
    lox_literal call(const Interpreter&, const std::vector<lox_literal>&) const override { return std::monostate{}; }

private:
    std::string name;
};