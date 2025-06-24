#pragma once
#include "token.hpp"
#include "LoxCallable.hpp"
#include "Environment.hpp"
#include "ReturnException.hpp"
#include "literal.hpp"
#include "Stmt.hpp"

class Interpreter; // Forward declaration

class LoxFunction : public LoxCallable {
    std::shared_ptr<Function> declaration;
    std::shared_ptr<Environment> closure;
    bool isInitializer = false;
public:
    LoxFunction(std::shared_ptr<Function> declaration, std::shared_ptr<Environment> closure, bool isInitializer)
        : declaration(std::move(declaration)), closure(std::move(closure)), isInitializer(isInitializer) {}

    lox_literal call(Interpreter& interpreter, const std::vector<lox_literal>& arguments) override;

    std::string toString() const override {
        return "<fn " + declaration->name.getLexeme() + ">";
    }

    size_t arity() const override {
        return declaration->params.size();
    }

    std::shared_ptr<LoxFunction> bind(const std::shared_ptr<LoxInstance>& instance) const {
        auto environment = std::make_shared<Environment>(closure);
        environment->define("this", instance);
        return std::make_shared<LoxFunction>(declaration, environment, isInitializer);
    }

    std::shared_ptr<Environment> getClosure() const { return closure; }

    ~LoxFunction() override {}
};