#include "LoxFunction.hpp"
#include "interpreter.hpp"
#include <iostream>

lox_literal LoxFunction::call(Interpreter& interpreter, const std::vector<lox_literal>& arguments) {
    // Create a new environment for function execution, with closure as parent
    auto environment = std::make_shared<Environment>(closure);

    // For bound methods, add 'this' to the execution environment (distance 0)
    if (boundInstance) {
        environment->define("this", boundInstance);
    }

    // Add parameters to function environment
    for (size_t i = 0; i < declaration->params.size(); ++i) {
        environment->define(declaration->params[i].getLexeme(), arguments[i]);
    }

    try {
        if (auto block = std::dynamic_pointer_cast<Block>(declaration->body)) {
            // Function body blocks are resolved directly in the function scope
            // so execute them directly in the function environment (no additional environment)
            interpreter.executeBlock(block->statements, environment);
        } else {
            std::vector<std::shared_ptr<Stmt>> bodyVec = {declaration->body};
            interpreter.executeBlock(bodyVec, environment);
        }
    } catch (const ReturnException& returnValue) {
        if (isInitializer) {
            // Always return 'this' from the execution environment
            return environment->getAt(0, "this");
        }
        return returnValue.getValue();
    }
    
    if(isInitializer) {
        return environment->getAt(0, "this");
    }
    return lox_literal(std::monostate{});
}

std::shared_ptr<LoxFunction> LoxFunction::bind(const std::shared_ptr<LoxInstance>& instance) const {
    // Get the unbound function
    auto orig = getUnbound();
    
    // Create a bound function that stores the instance for injection during call
    auto boundFunction = std::make_shared<LoxFunction>(orig->declaration, orig->closure, orig->isInitializer, orig);
    boundFunction->boundInstance = instance;
    return boundFunction;
}

std::shared_ptr<LoxFunction> LoxFunction::getUnbound() const {
    if (original) {
        return original->getUnbound();
    }
    return std::const_pointer_cast<LoxFunction>(shared_from_this());
}
