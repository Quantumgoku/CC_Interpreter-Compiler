#pragma once
#include "token.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "literal.hpp"
#include<unordered_map>
#include "LoxCallable.hpp"
#include "LoxInstance.hpp" // Ensure this header defines the LoxInstance class

class LoxInstance; // Forward declaration

class LoxClass : public LoxCallable, public std::enable_shared_from_this<LoxClass> {
public:
    LoxClass(const std::string& name) : name(name) {}
    std::string toString() const override { return name; }
    size_t arity() const override { return 0; }
    lox_literal call(const Interpreter&, const std::vector<lox_literal>&) override;
    std::string getName() const { return name; }
private:
    std::string name;
};