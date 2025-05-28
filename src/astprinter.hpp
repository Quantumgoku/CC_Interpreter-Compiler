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
            oss << std::visit([this](auto&& arg) -> std::string {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>)
                    return arg;
                else if constexpr (std::is_same_v<T, double>)
                    return this->normalizeNumberLiteral(std::to_string(arg));
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
        oss << "(" << expr.op.getLexme() << " ";
        if(auto lit = dynamic_cast<Literal*>(expr.right.get())) {
            this->visit(*lit);
        } else if(auto un = dynamic_cast<Unary*>(expr.right.get())) {
            this->visit(*un);
        } else if(auto bin = dynamic_cast<Binary*>(expr.right.get())) {
            this->visit(*bin);
        } else if(auto grp = dynamic_cast<Grouping*>(expr.right.get())) {
            this->visit(*grp);
        } else {
            expr.right->accept(*this);
        }
        oss << ")";
    }

private:
    mutable std::ostringstream oss;

    std::string normalizeNumberLiteral(const std::string& numStr) const {
        size_t dotPos = numStr.find('.');
        if (dotPos == std::string::npos) {
            // No decimal point, add ".0"
            return numStr + ".0";
        }
        // Start from the end and remove trailing zeros
        size_t endPos = numStr.size() - 1;
        while (endPos > dotPos && numStr[endPos] == '0') {
            endPos--;
        }
        // If the last char after trimming is '.', append '0'
        if (endPos == dotPos) {
            endPos++;
        }
        return numStr.substr(0, endPos + 1);
    }

    // Helper for parenthesizing nodes
    void parenthesize(const std::string& name, const std::vector<ExprPtr>& exprs) const {
        oss << "(" << name;
        for (const auto& expr : exprs) {
            oss << " ";
            if(auto lit = dynamic_cast<Literal*>(expr.get())){
                this->visit(*lit);
            }else if(auto un = dynamic_cast<Unary*>(expr.get())){
                this->visit(*un);
            }else if(auto bin = dynamic_cast<Binary*>(expr.get())){
                this->visit(*bin);
            }else if(auto grp = dynamic_cast<Grouping*>(expr.get())){
                this->visit(*grp);
            }else{
                expr->accept(*this);
            }
        }
        oss << ")";
    }
};