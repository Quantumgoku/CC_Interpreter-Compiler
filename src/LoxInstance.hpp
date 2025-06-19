#pragma once
#include "token.hpp"
#include "literal.hpp"
#include "Environment.hpp"
#include "LoxCallable.hpp"
#include<unordered_map>

class LoxClass; // Forward declaration

class LoxInstance : public LoxCallable {
    std::shared_ptr<LoxClass> klass;
    std::unordered_map<std::string, lox_literal> fields;
public:
    LoxInstance(std::shared_ptr<LoxClass> klass) : klass(std::move(klass)) {}
    std::string toString() const override;
    size_t arity() const override { return 0; }
    lox_literal call(const Interpreter&, const std::vector<lox_literal>&) override {
        throw RuntimeError(Token(TokenType::IDENTIFIER, "", std::monostate{}, 0), "Only classes and functions are callable.");
    }

    lox_literal get(const Token& name) {
        auto it = fields.find(name.getLexeme());
        if (it != fields.end()) {
            return it->second;
        }
        // if (klass->hasMethod(name.getLexeme())) {
        //     return klass->getMethod(name.getLexeme())->bind(shared_from_this());
        // }
        throw RuntimeError(name, "Undefined property '" + name.getLexeme() + "'.");
    }
    void set(const Token& name, const lox_literal& value); // Declaration only, implementation in .cpp
};