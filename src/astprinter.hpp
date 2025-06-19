#include <string>
#include <sstream>
#include "Expr.hpp"
#include <iostream>
#include <variant>

class ASTPrinter : public ExprVisitorPrint {
public:
    std::string print(const Expr& expr) {
        oss.str("");
        oss.clear();
        expr.accept(*this);
        return oss.str();
    }

    void visit(const Variable& expr) const override {
        oss << "(var " << expr.name.getLexeme() << ")";
    }

    void visit(const Binary& expr) const override {
        parenthesize(expr.op.getLexeme(), {expr.left, expr.right});
    }

    void visit(const Grouping& expr) const override {
        parenthesize("group", {expr.expression});
    }

    void visit(const Literal& expr) const override {
        if (!std::holds_alternative<std::monostate>(expr.value)) {
            auto* valuePtr = const_cast<lox_literal*>(&expr.value);
            ::std::visit([this](const auto& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>) {
                    oss << arg;
                } else if constexpr (std::is_same_v<T, double>) {
                    oss << this->normalizeNumberLiteral(std::to_string(arg));
                } else if constexpr (std::is_same_v<T, bool>) {
                    oss << (arg ? "true" : "false");
                } else if constexpr (std::is_same_v<T, std::shared_ptr<LoxCallable>>) {
                    oss << "<fn>";
                } else if constexpr (std::is_same_v<T, std::shared_ptr<LoxInstance>>) {
                    oss << "<instance>";
                } else if constexpr (std::is_same_v<T, std::monostate>) {
                    oss << "nil";
                } else {
                    oss << "<unknown>";
                }
            }, *valuePtr);
        } else {
            oss << "nil";
        }
    }

    void visit(const Unary& expr) const override {
        //print expr for debugging 
        oss << "(" << expr.op.getLexeme() << " ";
        expr.right->accept(*this);
        oss << ")";
    }

    void visit(const Assign& expr) const override {
        oss << "(assign " << expr.name.getLexeme() << " ";
        expr.value->accept(*this);
        oss << ")";
    }

    void visit(const Logical& expr) const override {
        parenthesize(expr.op.getLexeme(), {expr.left, expr.right});
    }

    void visit(const Call& expr) const override {
        oss << "(call ";
        expr.callee->accept(*this);
        for (const auto& arg : expr.arguments) {
            oss << " ";
            arg->accept(*this);
        }
        oss << ")";
    }

    void visit(const Get& expr) const override {
        oss << "(get ";
        expr.object->accept(*this);
        oss << "." << expr.name.getLexeme() << ")";
    }

    void visit(const Set& expr) const override {
        oss << "(set ";
        expr.object->accept(*this);
        oss << "." << expr.name.getLexeme() << " ";
        expr.value->accept(*this);
        oss << ")";
    }

private:
    mutable std::ostringstream oss;

    std::string normalizeNumberLiteral(const std::string& numStr) const {
        size_t dotPos = numStr.find('.');
        if (dotPos == std::string::npos) {
            return numStr + ".0";
        }
        size_t endPos = numStr.size() - 1;
        while (endPos > dotPos && numStr[endPos] == '0') {
            endPos--;
        }
        if (endPos == dotPos) {
            endPos++;
        }
        return numStr.substr(0, endPos + 1);
    }

    void parenthesize(const std::string& name, const std::vector<ExprPtr>& exprs) const {
        oss << "(" << name;
        for (const auto& expr : exprs) {
            oss << " ";
            expr->accept(*this);
        }
        oss << ")";
    }
};
