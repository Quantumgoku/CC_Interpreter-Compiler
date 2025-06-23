#include "LoxFunction.hpp"
#include "interpreter.hpp"

lox_literal LoxFunction::call(const Interpreter& interpreter, const std::vector<lox_literal>& arguments) {
    auto environment = std::make_shared<Environment>(closure);
    for (size_t i = 0; i < declaration->params.size(); ++i) {
        environment->define(declaration->params[i].getLexeme(), arguments[i]);
    }
    try {
        // If the function body is a Block, execute its statements directly in the new environment
        if (auto block = std::dynamic_pointer_cast<Block>(declaration->body)) {
            interpreter.executeBlock(block->statements, environment);
        } else {
            std::vector<std::shared_ptr<Stmt>> bodyVec = {declaration->body};
            interpreter.executeBlock(bodyVec, environment);
        }
    } catch (const ReturnException& returnValue) {
        if (isInitializer) {
            const auto& retVal = returnValue.getValue();
            if (!std::holds_alternative<std::monostate>(retVal)) {
                throw RuntimeError(Token(TokenType::IDENTIFIER, "return", std::monostate{}, 0), "Can't return a value from an initializer.");
            }
            return closure->getAt(0, "this");
        }
        return returnValue.getValue();
    }
    if(isInitializer) {
        // If this function is an initializer, return the instance
        return closure->getAt(0, "this");
    }
    return lox_literal(std::monostate{});
}
