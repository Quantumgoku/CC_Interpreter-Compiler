#pragma once
#include "token.hpp"
#include "LoxCallable.hpp"
#include "Environment.hpp"
#include "ReturnException.hpp"
#include "literal.hpp"
#include "Stmt.hpp"
#include <memory>

class Interpreter;
class LoxInstance;

/**
 * Represents a Lox function or method. Handles closures, method binding,
 * and proper 'this' and 'super' resolution for inheritance.
 */
class LoxFunction : public LoxCallable, public std::enable_shared_from_this<LoxFunction> {
    std::shared_ptr<Function> declaration;      // The function's AST node
    std::shared_ptr<Environment> closure;       // Captured environment (lexical scope)
    bool isInitializer = false;                 // True if this is a class initializer
    std::shared_ptr<LoxFunction> original;      // Original unbound function (for bound methods)
    std::shared_ptr<LoxInstance> boundInstance; // Instance this method is bound to

public:
    /** Create an unbound function. */
    LoxFunction(std::shared_ptr<Function> declaration, std::shared_ptr<Environment> closure, bool isInitializer)
        : declaration(std::move(declaration)), closure(std::move(closure)), isInitializer(isInitializer), original(nullptr), boundInstance(nullptr) {}
    
    /** Create a bound function (used internally by bind()). */
    LoxFunction(std::shared_ptr<Function> declaration, std::shared_ptr<Environment> closure, bool isInitializer, std::shared_ptr<LoxFunction> original)
        : declaration(std::move(declaration)), closure(std::move(closure)), isInitializer(isInitializer), original(std::move(original)), boundInstance(nullptr) {}

    /** Execute the function with given arguments. */
    lox_literal call(Interpreter& interpreter, const std::vector<lox_literal>& arguments) override;

    /** String representation of the function. */
    std::string toString() const override {
        return "<fn " + declaration->name.getLexeme() + ">";
    }

    /** Number of parameters this function expects. */
    size_t arity() const override {
        return declaration->params.size();
    }

    /** Create a new function bound to a specific instance (for methods). */
    std::shared_ptr<LoxFunction> bind(const std::shared_ptr<LoxInstance>& instance) const;

    /** Get the original unbound function. */
    std::shared_ptr<LoxFunction> getUnbound() const;

    /** Get the captured closure environment. */
    std::shared_ptr<Environment> getClosure() const { return closure; }

    /** Get the instance this function is bound to (if any). */
    std::shared_ptr<LoxInstance> getBoundInstance() const { return boundInstance; }

    ~LoxFunction() override {}
};