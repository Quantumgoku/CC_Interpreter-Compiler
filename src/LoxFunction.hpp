#pragma once
#include "token.hpp"
#include "LoxCallable.hpp"
#include "interpreter.hpp"
#include "Environment.hpp"

class LoxFunction : public LoxCallable {
    std::shared_ptr<Function> declaration;
    std::shared_ptr<Environment> closure;
public:
    LoxFunction(std::shared_ptr<Function> declaration, std::shared_ptr<Environment> closure)
        : declaration(std::move(declaration)), closure(std::move(closure)) {}

    literal call(const Interpreter& interpreter, const std::vector<literal>& arguments) const override {
        auto environment = std::make_shared<Environment>(closure);
        for (size_t i = 0; i < declaration->params.size(); ++i) {
            environment->define(declaration->params[i].getLexeme(), arguments[i]);
        }
        try {
            // declaration->body is a shared_ptr<Stmt>, but executeBlock expects a vector
            std::vector<std::shared_ptr<Stmt>> bodyVec = {declaration->body};
            interpreter.executeBlock(bodyVec, environment);
        } catch (const ReturnException& returnValue) {
            return returnValue.getValue(); // Return the value from the return exception
        }
        return literal(std::monostate{});
    }

    std::string toString() const override {
        return declaration->name.getLexeme() + " (function)";
    }

    size_t arity() const override {
        return declaration->params.size();
    }
};