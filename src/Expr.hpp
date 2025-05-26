#pragma once

#include <string>
#include <variant>
#include <vector>
#include <memory>
#include "token.hpp"

class Expr;
class ExprVisitorBase;

class Expr {
public:
    virtual ~Expr() = default;
    virtual void accept(const ExprVisitorBase& visitor) const = 0;
};

// Forward declarations
struct Binary;
struct Grouping;
struct Literal;
struct Unary;

class ExprVisitorBase {
public:
    virtual void visit(const Binary& expr) const = 0;
    virtual void visit(const Grouping& expr) const = 0;
    virtual void visit(const Literal& expr) const = 0;
    virtual void visit(const Unary& expr) const = 0;
    virtual ~ExprVisitorBase() = default;
};

class Binary : public Expr {
public:
    Binary(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right) : left(left), op(op), right(right) {}
    void accept(const ExprVisitorBase& visitor) const override { visitor.visit(*this); }
    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
};

class Grouping : public Expr {
public:
    Grouping(std::shared_ptr<Expr> expression) : expression(expression) {}
    void accept(const ExprVisitorBase& visitor) const override { visitor.visit(*this); }
    std::shared_ptr<Expr> expression;
};

class Literal : public Expr {
public:
    ~Literal() override = default;
    Literal(std::variant<std::monostate, std::string, double, bool> value) : value(value) {}
    void accept(const ExprVisitorBase& visitor) const override { visitor.visit(*this); }
    std::variant<std::monostate, std::string, double, bool> value;
};

class Unary : public Expr {
public:
    Unary(Token op, std::shared_ptr<Expr> right) : op(op), right(right) {}
    void accept(const ExprVisitorBase& visitor) const override { visitor.visit(*this); }
    Token op;
    std::shared_ptr<Expr> right;
};

using ExprPtr = std::shared_ptr<Expr>;
using ExprList = std::vector<ExprPtr>;
