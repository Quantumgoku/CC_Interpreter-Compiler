#pragma once
#include <chrono>
#include "LoxCallable.hpp"

class LoxClock : public LoxCallable {
public:
    std::string toString() const override {
        return "<native fn clock>";
    }

    size_t arity() const override {
        return 0; // Clock function takes no arguments
    }

    literal call(const Interpreter& interpreter, const std::vector<literal>& arguments) const override {
        // Get the current time in seconds since epoch
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        return static_cast<double>(seconds);
    }
};