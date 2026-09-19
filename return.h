#pragma once

#include "expr.h"

using namespace std;


// ============================================================
// RETURN EXCEPTION
// ============================================================

class ReturnException {
public:

    Value value;

    ReturnException(Value value)
        : value(value) {}
};