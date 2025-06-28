#pragma once
#include "token.hpp"
#include "LoxCallable.hpp"
#include "Environment.hpp"
#include "ReturnException.hpp"
#include "literal.hpp"
#include "Stmt.hpp"
#include <memory>

class Interpreter; // Forward declaration
class LoxInstance; // Forward declaration

class LoxFunction : public LoxCallable, public std::enable_shared_from_this<LoxFunction> {
    std::shared_ptr<Function> declaration;
    std::shared_ptr<Environment> closure;
    bool isInitializer = false;
    std::shared_ptr<LoxFunction> original; // Points to the original function if this is a bound method
    std::shared_ptr<LoxInstance> boundInstance; // The instance this method is bound to
public:
    LoxFunction(std::shared_ptr<Function> declaration, std::shared_ptr<Environment> closure, bool isInitializer)
        : declaration(std::move(declaration)), closure(std::move(closure)), isInitializer(isInitializer), original(nullptr), boundInstance(nullptr) {}
    LoxFunction(std::shared_ptr<Function> declaration, std::shared_ptr<Environment> closure, bool isInitializer, std::shared_ptr<LoxFunction> original)
        : declaration(std::move(declaration)), closure(std::move(closure)), isInitializer(isInitializer), original(std::move(original)), boundInstance(nullptr) {}

    lox_literal call(Interpreter& interpreter, const std::vector<lox_literal>& arguments) override;

    std::string toString() const override {
        return "<fn " + declaration->name.getLexeme() + ">";
    }

    size_t arity() const override {
        return declaration->params.size();
    }

    std::shared_ptr<LoxFunction> bind(const std::shared_ptr<LoxInstance>& instance) const;

    std::shared_ptr<LoxFunction> getUnbound() const;

    std::shared_ptr<Environment> getClosure() const { return closure; }

    std::shared_ptr<LoxInstance> getBoundInstance() const { return boundInstance; }

    ~LoxFunction() override {}
};