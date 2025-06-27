#pragma once

#include <iostream> // For std::cout and std::cerr
#include <map>
#include <memory>
#include <string>
#include "literal.hpp"
#include "token.hpp"
#include "RuntimeError.hpp"

class Environment : public std::enable_shared_from_this<Environment> {
public:
    explicit Environment(std::shared_ptr<Environment> enclosing = nullptr)
        : enclosing(enclosing) {}

    void define(const std::string& name, const lox_literal& value = lox_literal()) {
        values[name] = value;
    }

    lox_literal getAt(int distance, const std::string& name) const {
        auto env = ancestor(distance);
        if (!env) {
            throw RuntimeError(Token(TokenType::IDENTIFIER, name, std::monostate{}, 0), 
                              "Environment chain broken at distance " + std::to_string(distance));
        }
        auto it = env->values.find(name);
        if (it != env->values.end()) {
            return it->second;
        }
        throw RuntimeError(Token(TokenType::IDENTIFIER, name, std::monostate{}, 0), 
                          "Undefined variable '" + name + "' at distance " + std::to_string(distance));
    }

    void assignAt(int distance, const Token& name, const lox_literal& value) {
        ancestor(distance)->values[name.getLexeme()] = value;
    }

    lox_literal getValue(const Token& name) const {
        const std::string& key = name.getLexeme();
        if (values.count(key)) return values.at(key);
        if (enclosing) return enclosing->getValue(name);
        throw RuntimeError(name, "Undefined variable '" + key + "'.");
    }

    void assign(const Token& name, const lox_literal& value) {
        const std::string& key = name.getLexeme();
        if (values.count(key)) {
            values[key] = value;
            return;
        }
        if (enclosing) {
            enclosing->assign(name, value);
            return;
        }
        throw RuntimeError(name, "Undefined variable '" + key + "'.");
    }

    std::shared_ptr<Environment> ancestor(int distance) const {
        std::shared_ptr<Environment> env = const_cast<Environment*>(this)->shared_from_this();
        for (int i = 0; i < distance; ++i) {
            env = env->enclosing;
        }
        return env;
    }

    std::shared_ptr<Environment> getEnclosing() const {
        return enclosing;
    }

    void setEnclosing(std::shared_ptr<Environment> enclosing) {
        this->enclosing = enclosing;
    }

    bool has(const std::string& name) const {
        return values.find(name) != values.end();
    }

private:
    std::map<std::string, lox_literal> values;
    std::shared_ptr<Environment> enclosing;
};
