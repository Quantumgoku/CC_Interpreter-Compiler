#pragma once

#include <vector>
#include "token.hpp"
#include "literal.hpp"

class Expr;
class ExprVisitorPrint;
class ExprVisitorEval;

class Expr {
public:
    virtual ~Expr() = default;
    virtual void accept(const ExprVisitorPrint& visitor) const = 0;
    virtual lox_literal accept(ExprVisitorEval& visitor) const = 0;
};

// Forward declarations
class Assign;
class Binary;
class Grouping;
class Literal;
class Logical;
class Unary;
class Super;
class This;
class Call;
class Get;
class Set;
class Variable;

class ExprVisitorPrint {
public:
    virtual void visit(const Assign& expr) const = 0;
    virtual void visit(const Binary& expr) const = 0;
    virtual void visit(const Grouping& expr) const = 0;
    virtual void visit(const Literal& expr) const = 0;
    virtual void visit(const Logical& expr) const = 0;
    virtual void visit(const Unary& expr) const = 0;
    virtual void visit(const Super& expr) const = 0;
    virtual void visit(const This& expr) const = 0;
    virtual void visit(const Call& expr) const = 0;
    virtual void visit(const Get& expr) const = 0;
    virtual void visit(const Set& expr) const = 0;
    virtual void visit(const Variable& expr) const = 0;
    virtual ~ExprVisitorPrint() = default;
};

class ExprVisitorEval {
public:
    virtual lox_literal visit(const Assign& expr) = 0;
    virtual lox_literal visit(const Binary& expr) = 0;
    virtual lox_literal visit(const Grouping& expr) = 0;
    virtual lox_literal visit(const Literal& expr) = 0;
    virtual lox_literal visit(const Logical& expr) = 0;
    virtual lox_literal visit(const Unary& expr) = 0;
    virtual lox_literal visit(const Super& expr) = 0;
    virtual lox_literal visit(const This& expr) = 0;
    virtual lox_literal visit(const Call& expr) = 0;
    virtual lox_literal visit(const Get& expr) = 0;
    virtual lox_literal visit(const Set& expr) = 0;
    virtual lox_literal visit(const Variable& expr) = 0;
    virtual ~ExprVisitorEval() = default;
};

class Assign : public Expr {
public:
    Assign(Token name, std::shared_ptr<Expr> value) : name(name), value(value) {}
    void accept(const ExprVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(ExprVisitorEval& visitor) const override { return visitor.visit(*this); }
    Token name;
    std::shared_ptr<Expr> value;
};

class Binary : public Expr {
public:
    Binary(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right) : left(left), op(op), right(right) {}
    void accept(const ExprVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(ExprVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
};

class Grouping : public Expr {
public:
    Grouping(std::shared_ptr<Expr> expression) : expression(expression) {}
    void accept(const ExprVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(ExprVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::shared_ptr<Expr> expression;
};

class Literal : public Expr {
public:
    ~Literal() override = default;
    Literal(lox_literal value) : value(value) {}
    void accept(const ExprVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(ExprVisitorEval& visitor) const override { return visitor.visit(*this); }
    lox_literal value;
};

class Logical : public Expr {
public:
    Logical(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right) : left(left), op(op), right(right) {}
    void accept(const ExprVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(ExprVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
};

class Unary : public Expr {
public:
    Unary(Token op, std::shared_ptr<Expr> right) : op(op), right(right) {}
    void accept(const ExprVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(ExprVisitorEval& visitor) const override { return visitor.visit(*this); }
    Token op;
    std::shared_ptr<Expr> right;
};

class Super : public Expr {
public:
    Super(Token keyword, Token method) : keyword(keyword), method(method) {}
    void accept(const ExprVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(ExprVisitorEval& visitor) const override { return visitor.visit(*this); }
    Token keyword;
    Token method;
};

class This : public Expr {
public:
    This(Token keyword) : keyword(keyword) {}
    void accept(const ExprVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(ExprVisitorEval& visitor) const override { return visitor.visit(*this); }
    Token keyword;
};

class Call : public Expr {
public:
    Call(std::shared_ptr<Expr> callee, Token paren, std::vector<std::shared_ptr<Expr>> arguments) : callee(callee), paren(paren), arguments(arguments) {}
    void accept(const ExprVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(ExprVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::shared_ptr<Expr> callee;
    Token paren;
    std::vector<std::shared_ptr<Expr>> arguments;
};

class Get : public Expr {
public:
    Get(std::shared_ptr<Expr> object, Token name) : object(object), name(name) {}
    void accept(const ExprVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(ExprVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::shared_ptr<Expr> object;
    Token name;
};

class Set : public Expr {
public:
    Set(std::shared_ptr<Expr> object, Token name, std::shared_ptr<Expr> value) : object(object), name(name), value(value) {}
    void accept(const ExprVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(ExprVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::shared_ptr<Expr> object;
    Token name;
    std::shared_ptr<Expr> value;
};

class Variable : public Expr {
public:
    Variable(Token name) : name(name) {}
    void accept(const ExprVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(ExprVisitorEval& visitor) const override { return visitor.visit(*this); }
    Token name;
};

// All AST node fields referencing other nodes use std::shared_ptr<T>.
// All containers (vectors) and optionals use shared_ptr for AST nodes.

using ExprPtr = std::shared_ptr<Expr>;
using ExprList = std::vector<ExprPtr>;
