#pragma once

#include "stmt.h"
#include "environment.h"

using namespace std;


// ============================================================
// LOX FUNCTION
// ============================================================

class LoxFunction : public LoxCallable {
public:

    shared_ptr<FunctionStmt> declaration;

    shared_ptr<Environment> closure;


    LoxFunction(
        shared_ptr<FunctionStmt> declaration,
        shared_ptr<Environment> closure
    )
        : declaration(declaration),
          closure(closure) {}


    int arity() override {
        return static_cast<int>(declaration->params.size());
    }


    Value call(
        Interpreter* interpreter,
        const vector<Value>& arguments
    ) override;


    string toString() override {
        return "<fn " + declaration->name.lexeme + ">";
    }
};