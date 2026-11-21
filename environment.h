#pragma once

#include "expr.h"
#include "token.h"
#include "runtime_error.h"

#include <map>
#include <string>

using namespace std;

// ENVIRONMENT
//
// Stores variables for one scope.
//
// Example:
//
// Global:
//     a = 10
//
// Block:
//     b = 20
//
// Block's enclosing environment = Global

class Environment {
public:
    // GLOBAL ENVIRONMENT
    Environment() : enclosing(nullptr) {}

    // LOCAL ENVIRONMENT
    //
    // This environment has another environment as parent.
    Environment(Environment* enclosing) : enclosing(enclosing) {}

    // DEFINE
    //
    // Creates a variable in CURRENT scope.
    //
    // var x = 10;
    void define(const string& name, const Value& value) {
        values[name] = value;
    }

    // GET
    //
    // First search current scope.
    //
    // If not found:
    // search enclosing scope.
    //
    // Example:
    //
    // global:
    //     x = 10
    //
    // block:
    //     print x;
    //
    // block doesn't have x,
    // so search global.
    Value get(const Token& name) {
        if (values.count(name.lexeme)) {
            return values[name.lexeme];
        }
        if (enclosing != nullptr) {
            return enclosing->get(name);
        }
        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }

    // ASSIGN
    //
    // Assignment DOES NOT create a new variable.
    //
    // x = 20;
    //
    // First search current scope.
    // If not found, search parent.
    void assign(const Token& name, const Value& value) {
        if (values.count(name.lexeme)) {
            values[name.lexeme] = value;
            return;
        }
        if (enclosing != nullptr) {
            enclosing->assign(name, value);
            return;
        }
        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }
private:
    friend class Interpreter;
    bool captured = false;

    // Variables belonging to THIS scope.
    map<string, Value> values;

    // Parent / enclosing scope.
    Environment* enclosing;
};
