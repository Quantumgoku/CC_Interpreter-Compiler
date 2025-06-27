#pragma once
#include <stdexcept>
#include "token.hpp"

class ResolveError : public std::runtime_error {
public:
    const Token token;

    ResolveError(const Token& token, const std::string& message)
        : std::runtime_error(message), token(token) {}
};
