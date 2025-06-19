#pragma once
#include "literal.hpp"
#include "token.hpp"
#include <vector>
#include <stdexcept>

inline bool isTruthy(const lox_literal& value) {
    if (std::holds_alternative<std::monostate>(value)) return false;
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value);
    return true;
}

inline void checkNumberOperand(const Token& op, const std::vector<lox_literal>& operands) {
    for (const auto& operand : operands) {
        if (!std::holds_alternative<double>(operand)) {
            throw RuntimeError(op, "Operand must be a number.");
        }
    }
}
