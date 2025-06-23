#pragma once
#include "token.hpp"
#include "literal.hpp"
#include "Environment.hpp"
#include "LoxCallable.hpp"
#include "LoxFunction.hpp"
#include <unordered_map>

class LoxClass; // Forward declaration

class LoxInstance : public LoxCallable, public std::enable_shared_from_this<LoxInstance> {
public:
    static std::shared_ptr<LoxInstance> create(std::shared_ptr<LoxClass> klass) {
        return std::shared_ptr<LoxInstance>(new LoxInstance(std::move(klass)));
    }
    std::string toString() const override;
    size_t arity() const override { return 0; }
    lox_literal call(const Interpreter&, const std::vector<lox_literal>&) override;
    lox_literal get(const Token& name);
    void set(const Token& name, const lox_literal& value);
    ~LoxInstance() override {
        // Defensive logging for shutdown crash diagnosis
        // (You can replace this with a real logger or std::cerr if needed)
        std::cerr << "Destroying LoxInstance of class: " << (klass ? klass->getName() : "<null>") << std::endl;
    }
private:
    LoxInstance(std::shared_ptr<LoxClass> klass);
    std::shared_ptr<LoxClass> klass;
    std::unordered_map<std::string, lox_literal> fields;
};