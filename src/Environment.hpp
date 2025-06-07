#include<map>
#include<string>
#include<vector>
#include<variant>
#include<optional>
#include<stdexcept>
#include "token.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "RuntimeError.hpp"

class Environment{
private:
    std::shared_ptr<Environment> enclosing;
    std::map<std::string, literal> values;
public:

    Environment() = default;
    Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {}

    void define(const std::string& name, const literal& value){
        values[name] = value;
    }

    void assign(const Token& name, const literal& value) {
        auto it = values.find(name.getLexme());
        if(it != values.end()){
            it->second = value;
        }
        else if(enclosing != nullptr){
            enclosing->assign(name, value);
            return;
        }
        throw RuntimeError(name, "Undefined variable '" + name.getLexme() + "'.");
    }

    std::optional<literal> get(const std::string& name) const {
        auto it = values.find(name);
        if(it != values.end()){
            return it->second;
        }
        if(enclosing != nullptr){
            return enclosing->get(name);
        }
        throw RuntimeError(Token(TokenType::IDENTIFIER, name, std::monostate{}, 0), "Undefined variable '" + name + "'.");
    }
};