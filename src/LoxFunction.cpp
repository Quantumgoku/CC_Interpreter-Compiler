#include "LoxFunction.hpp"
#include "interpreter.hpp"

lox_literal LoxFunction::call(Interpreter& interpreter, const std::vector<lox_literal>& arguments) {
    // If 'this' is already defined in the closure, use closure directly for parameters (for methods)
    std::shared_ptr<Environment> environment;
    if (closure->has("this")) {
        environment = closure;
    } else {
        environment = std::make_shared<Environment>(closure);
    }
    for (size_t i = 0; i < declaration->params.size(); ++i) {
        environment->define(declaration->params[i].getLexeme(), arguments[i]);
    }
    // Save and set closureForNestedFunctions to the closure for the duration of the call
    auto prevClosure = interpreter.closureForNestedFunctions;
    interpreter.closureForNestedFunctions = closure;
    try {
        // If the function body is a Block, execute its statements directly in the new environment
        if (auto block = std::dynamic_pointer_cast<Block>(declaration->body)) {
            interpreter.executeBlock(block->statements, environment);
        } else {
            std::vector<std::shared_ptr<Stmt>> bodyVec = {declaration->body};
            interpreter.executeBlock(bodyVec, environment);
        }
    } catch (const ReturnException& returnValue) {
        interpreter.closureForNestedFunctions = prevClosure;
        if (isInitializer) {
            const auto& retVal = returnValue.getValue();
            if (!std::holds_alternative<std::monostate>(retVal)) {
                throw RuntimeError(Token(TokenType::IDENTIFIER, "return", std::monostate{}, 0), "Can't return a value from an initializer.");
            }
            return closure->getAt(0, "this");
        }
        return returnValue.getValue();
    }
    interpreter.closureForNestedFunctions = prevClosure;
    if(isInitializer) {
        // If this function is an initializer, return the instance
        return closure->getAt(0, "this");
    }
    return lox_literal(std::monostate{});
}
