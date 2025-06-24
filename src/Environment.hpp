#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <memory>
#include "token.hpp"
#include "literal.hpp"
#include "RuntimeError.hpp"

class Environment : public std::enable_shared_from_this<Environment> {
private:
    std::weak_ptr<Environment> enclosing;
    std::map<std::string, lox_literal> values;
public:

    Environment() {
    }
    Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {
    }

    void define(const std::string& name, const lox_literal& value = lox_literal()){
        values[name] = value;
    }

    lox_literal getAt(int distance, const std::string& name) const {
        auto env = ancestor(distance);
        auto it = env->values.find(name);
        if (it != env->values.end()) {
            // Defensive: always return a copy, never a reference
            return lox_literal(it->second);
        }
        throw RuntimeError(Token(TokenType::IDENTIFIER, name, std::monostate{}, 0), "Undefined variable '" + name + "' at resolved depth.");
    }

    lox_literal getValue(const Token& name) const {
        const std::string& key = name.getLexeme();
        auto it = values.find(key);
        if(it != values.end()){
            // Defensive: always return a copy, never a reference
            return lox_literal(it->second);
        }
        else if(auto enc = enclosing.lock()){
            return enc->getValue(name);
        }
        throw RuntimeError(name, "Undefined variable '" + key + "'.");
    }

    void assign(const Token& name, const lox_literal& value) {
        const std::string& key = name.getLexeme();
        auto it = values.find(key);
        if(it != values.end()){
            it->second = value;
            return;
        }
        else if(auto enc = enclosing.lock()){
            enc->assign(name, value);
            return;
        }
        throw RuntimeError(name, "Undefined variable '" + key + "'.");
    }

    void assignAt(int distance, const Token& name, const lox_literal& value) {
        auto env = ancestor(distance);
        auto it = env->values.find(name.getLexeme());
        if (it != env->values.end()) {
            it->second = value;
        } else {
            throw RuntimeError(name, "Undefined variable '" + name.getLexeme() + "' at resolved depth.");
        }
    }

    std::shared_ptr<Environment> ancestor(int distance) const {
        std::shared_ptr<Environment> environment = std::const_pointer_cast<Environment>(shared_from_this());
        for (int i = 0; i < distance; ++i) {
            environment = environment->enclosing.lock();
        }
        return environment;
    }
};