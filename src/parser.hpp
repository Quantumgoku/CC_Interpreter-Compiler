#include "token.hpp"
#include "tokenizer.hpp"
#include <iostream>

struct Expr{
    Expr *left;
    Expr *right;
    Token op;
};