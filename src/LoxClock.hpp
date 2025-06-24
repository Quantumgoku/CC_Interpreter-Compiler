#pragma once
#include <chrono>
#include "LoxCallable.hpp"
#include "interpreter.hpp"
#include "literal.hpp"

class LoxClock : public LoxCallable {
public:
    std::string toString() const override {
        return "<native fn clock>";
    }

    size_t arity() const override {
        return 0; // Clock function takes no arguments
    }

    lox_literal call(Interpreter& interpreter, const std::vector<lox_literal>& arguments) override {
        // Get the current time in seconds since epoch
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        return static_cast<double>(millis) / 1000.0;
    }
};