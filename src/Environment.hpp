#include<map>
#include<string>
#include<vector>
#include<variant>
#include<optional>
#include<stdexcept>
#include "token.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "interpreter.hpp"

class Environment{
    std::map<std::string, literal> values;
public:
    void define(const std::string& name, const literal& value){
        values[name] = value;
    }

    std::optional<literal> get(const std::string& name) const {
        auto it = values.find(name);
        if(it != values.end()){
            return it->second;
        }
        throw Interpreter::RuntimeError(Token(TokenType::IDENTIFIER, name, std::monostate{}, 0), "Undefined variable '" + name + "'.");
    }
};