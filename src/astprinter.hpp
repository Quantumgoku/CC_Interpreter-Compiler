#include <string>
#include <sstream>
#include "Expr.hpp"
#include <iostream>

class ASTPrinter : public ExprVisitorBase {
public:
    std::string print(const Expr& expr) {
        oss.str("");
        oss.clear();
        expr.accept(*this);
        return oss.str();
    }

    void visit(const Binary& expr) const override {
        parenthesize(expr.op.getLexme(), {expr.left, expr.right});
    }

    void visit(const Grouping& expr) const override {
        parenthesize("group", {expr.expression});
    }

    void visit(const Literal& expr) const override {
        if (!std::holds_alternative<std::monostate>(expr.value)) {
            oss << std::visit([](auto&& arg) -> std::string {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>)
                    return arg;
                else if constexpr (std::is_same_v<T, double>)
                    return std::to_string(arg);
                else if constexpr (std::is_same_v<T, bool>)
                    return arg ? "true" : "false";
                else
                    return "";
            }, expr.value);
        } else {
            oss << "nil";
        }
    }

    void visit(const Unary& expr) const override {
        parenthesize(expr.op.getLexme(), {expr.right});
    }

private:
    mutable std::ostringstream oss;

    // Helper for parenthesizing nodes
    void parenthesize(const std::string& name, const std::vector<ExprPtr>& exprs) const {
        oss << "(" << name;
        for (const auto& expr : exprs) {
            oss << " ";
            expr->accept(*this);
        }
        oss << ")";
    }
};