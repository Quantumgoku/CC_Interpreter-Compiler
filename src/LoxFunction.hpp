#pragma once
#include "token.hpp"
#include "LoxCallable.hpp"
#include "Environment.hpp"
#include "ReturnException.hpp"
#include "literal.hpp"
#include "Stmt.hpp"
#include <memory>

class Interpreter; // Forward declaration

class LoxFunction : public LoxCallable, public std::enable_shared_from_this<LoxFunction> {
    std::shared_ptr<Function> declaration;
    std::shared_ptr<Environment> closure;
    bool isInitializer = false;
    std::shared_ptr<LoxFunction> original; // Points to the original function if this is a bound method
public:
    LoxFunction(std::shared_ptr<Function> declaration, std::shared_ptr<Environment> closure, bool isInitializer)
        : declaration(std::move(declaration)), closure(std::move(closure)), isInitializer(isInitializer), original(nullptr) {}
    LoxFunction(std::shared_ptr<Function> declaration, std::shared_ptr<Environment> closure, bool isInitializer, std::shared_ptr<LoxFunction> original)
        : declaration(std::move(declaration)), closure(std::move(closure)), isInitializer(isInitializer), original(std::move(original)) {}

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
        // Always bind the original function, not a previously bound one
        auto orig = getUnbound();
        return std::make_shared<LoxFunction>(orig->declaration, environment, orig->isInitializer, orig);
    }
    std::shared_ptr<LoxFunction> getUnbound() const {
        if (original) return original->getUnbound();
        return std::const_pointer_cast<LoxFunction>(shared_from_this());
    }

    std::shared_ptr<Environment> getClosure() const { return closure; }

    ~LoxFunction() override {}
};