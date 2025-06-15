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
public:
    LoxFunction(std::shared_ptr<Function> declaration, std::shared_ptr<Environment> closure)
        : declaration(std::move(declaration)), closure(std::move(closure)) {}

    lox_literal call(const Interpreter& interpreter, const std::vector<lox_literal>& arguments) const override {
        auto environment = std::make_shared<Environment>(closure);
        for (size_t i = 0; i < declaration->params.size(); ++i) {
            environment->define(declaration->params[i].getLexeme(), arguments[i]);
        }
        try {
            std::vector<std::shared_ptr<Stmt>> bodyVec = {declaration->body};
            interpreter.executeBlock(bodyVec, environment);
        } catch (const ReturnException& returnValue) {
            return returnValue.getValue();
        }
        return lox_literal(std::monostate{});
    }

    std::string toString() const override {
        return "<fn " + declaration->name.getLexeme() + ">";
    }

    size_t arity() const override {
        return declaration->params.size();
    }
};