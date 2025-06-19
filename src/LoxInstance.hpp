#pragma once
#include "token.hpp"
#include "literal.hpp"
#include "Environment.hpp"
#include "LoxCallable.hpp"

class LoxClass; // Forward declaration

class LoxInstance : public LoxCallable {
    std::shared_ptr<LoxClass> klass;
public:
    LoxInstance(std::shared_ptr<LoxClass> klass) : klass(std::move(klass)) {}
    std::string toString() const override;
    size_t arity() const override { return 0; }
    lox_literal call(const Interpreter&, const std::vector<lox_literal>&) override {
        throw RuntimeError(Token(TokenType::IDENTIFIER, "", std::monostate{}, 0), "Only classes and functions are callable.");
    }
};