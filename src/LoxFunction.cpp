#include "LoxFunction.hpp"
#include "interpreter.hpp"
#include <iostream>

lox_literal LoxFunction::call(Interpreter& interpreter, const std::vector<lox_literal>& arguments) {
    // Create execution environment with closure as parent
    auto environment = std::make_shared<Environment>(closure);

    // For bound methods, inject 'this' into the execution environment
    if (boundInstance) {
        environment->define("this", boundInstance);
    }

    // Add function parameters to the execution environment
    for (size_t i = 0; i < declaration->params.size(); ++i) {
        environment->define(declaration->params[i].getLexeme(), arguments[i]);
    }

    try {
        // Execute the function body in the new environment
        if (auto block = std::dynamic_pointer_cast<Block>(declaration->body)) {
            interpreter.executeBlock(block->statements, environment);
        } else {
            std::vector<std::shared_ptr<Stmt>> bodyVec = {declaration->body};
            interpreter.executeBlock(bodyVec, environment);
        }
    } catch (const ReturnException& returnValue) {
        // Initializers always return 'this', even if they explicitly return something else
        if (isInitializer) {
            return environment->getAt(0, "this");
        }
        return returnValue.getValue();
    }
    
    // Implicit return for initializers
    if(isInitializer) {
        return environment->getAt(0, "this");
    }
    return lox_literal(std::monostate{});
}

std::shared_ptr<LoxFunction> LoxFunction::bind(const std::shared_ptr<LoxInstance>& instance) const {
    // Get the original unbound function to avoid chains of bound functions
    auto orig = getUnbound();
    
    // Create a new bound function that will inject 'this' during execution
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
