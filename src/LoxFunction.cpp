#include "LoxFunction.hpp"
#include "interpreter.hpp"
#include <iostream>

lox_literal LoxFunction::call(Interpreter& interpreter, const std::vector<lox_literal>& arguments) {
    // Always create a new environment for parameters, with closure as parent
    auto environment = std::make_shared<Environment>(closure);
    // If this is a method (closure has 'this'), propagate 'this' to the new environment
    if (closure->has("this")) {
        environment->define("this", closure->getAt(0, "this"));
    }
    for (size_t i = 0; i < declaration->params.size(); ++i) {
        environment->define(declaration->params[i].getLexeme(), arguments[i]);
    }
    auto prevClosure = interpreter.closureForNestedFunctions;
    interpreter.closureForNestedFunctions = closure;
    try {
        if (auto block = std::dynamic_pointer_cast<Block>(declaration->body)) {
            interpreter.executeBlock(block->statements, environment);
        } else {
            std::vector<std::shared_ptr<Stmt>> bodyVec = {declaration->body};
            interpreter.executeBlock(bodyVec, environment);
        }
    } catch (const ReturnException& returnValue) {
        interpreter.closureForNestedFunctions = prevClosure;
        if (isInitializer) {
            // Always return 'this' from the environment
            return environment->getAt(0, "this");
        }
        return returnValue.getValue();
    }
    interpreter.closureForNestedFunctions = prevClosure;
    if(isInitializer) {
        return environment->getAt(0, "this");
    }
    return lox_literal(std::monostate{});
}

std::shared_ptr<LoxFunction> LoxFunction::bind(const std::shared_ptr<LoxInstance>& instance) const {
    auto environment = std::make_shared<Environment>(closure);
    environment->define("this", instance);
    auto orig = getUnbound();
    return std::make_shared<LoxFunction>(orig->declaration, environment, orig->isInitializer, orig);
}

std::shared_ptr<LoxFunction> LoxFunction::getUnbound() const {
    if (original) {
        return original->getUnbound();
    }
    return std::const_pointer_cast<LoxFunction>(shared_from_this());
}
