#pragma once
#include <exception>
#include "token.hpp"
#include "literal.hpp"

class ReturnException : public std::exception {
public:
    explicit ReturnException(const literal& value) : value(value) {    
    }

    const char* what() const noexcept override {
        return "ReturnException";
    }

    literal getValue() const {
        return value;
    }
private:
    literal value;
};