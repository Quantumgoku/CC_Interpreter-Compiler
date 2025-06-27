#include "LoxFunction.hpp"
#include "interpreter.hpp"
#include <iostream>

lox_literal LoxFunction::call(Interpreter& interpreter, const std::vector<lox_literal>& arguments) {
    // Always create a new environment for parameters, with closure as parent
    auto environment = std::make_shared<Environment>(closure);

    // If we have a bound instance, inject 'this' directly
    if (boundInstance) {
        environment->define("this", boundInstance);
    }
    // Legacy: If this is a method (closure has 'this'), propagate 'this' to the new environment
    else if (closure->has("this")) {
        environment->define("this", closure->getAt(0, "this"));
    }
    
    for (size_t i = 0; i < declaration->params.size(); ++i) {
        environment->define(declaration->params[i].getLexeme(), arguments[i]);
    }
    auto prevClosure = interpreter.closureForNestedFunctions;
    interpreter.closureForNestedFunctions = environment;  // Set to current function's environment, not closure
    try {
        if (auto block = std::dynamic_pointer_cast<Block>(declaration->body)) {
            // Only for bound methods, create a separate body environment to match resolver's expectations
            if (boundInstance) {
                auto bodyEnvironment = std::make_shared<Environment>(environment);
                // For initializers, also copy parameters to body environment to match resolver expectations
                if (isInitializer) {
                    for (size_t i = 0; i < declaration->params.size(); ++i) {
                        bodyEnvironment->define(declaration->params[i].getLexeme(), arguments[i]);
                    }
                }
                interpreter.executeBlock(block->statements, bodyEnvironment);
            } else {
                interpreter.executeBlock(block->statements, environment);
            }
        } else {
            std::vector<std::shared_ptr<Stmt>> bodyVec = {declaration->body};
            if (boundInstance) {
                auto bodyEnvironment = std::make_shared<Environment>(environment);
                // For initializers, also copy parameters to body environment to match resolver expectations
                if (isInitializer) {
                    for (size_t i = 0; i < declaration->params.size(); ++i) {
                        bodyEnvironment->define(declaration->params[i].getLexeme(), arguments[i]);
                    }
                }
                interpreter.executeBlock(bodyVec, bodyEnvironment);
            } else {
                interpreter.executeBlock(bodyVec, environment);
            }
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
    // Don't create an extra environment level - just store the instance for later injection
    auto orig = getUnbound();
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
