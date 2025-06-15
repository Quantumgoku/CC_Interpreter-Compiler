#pragma once
#include <stdexcept>
#include "token.hpp"
#include "literal.hpp"

class RuntimeError : public std::runtime_error {
public:
    Token token;
    RuntimeError(const Token& token, const std::string& message) : std::runtime_error(message), token(token){}
};