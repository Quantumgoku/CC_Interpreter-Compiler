#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "token.hpp"
#include "literal.hpp"
#include "RuntimeError.hpp"

class Environment{
private:
    std::shared_ptr<Environment> enclosing;
    std::map<std::string, lox_literal> values;
public:

    Environment() = default;
    Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {}

    void define(const std::string& name, const lox_literal& value = lox_literal()){
        values[name] = value;
    }

    lox_literal getValue(const Token& name) const {
        const std::string& key = name.getLexeme();
        auto it = values.find(key);
        if(it != values.end()){
            return it->second;
        }
        else if(enclosing != nullptr){
            return enclosing->getValue(name);
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
        else if(enclosing != nullptr){
            enclosing->assign(name, value);
            return;
        }
        throw RuntimeError(name, "Undefined variable '" + key + "'.");
    }
};