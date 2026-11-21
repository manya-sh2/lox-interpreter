#pragma once
#include <stdexcept>
#include "token.h"

using namespace std;

// Represents a runtime error during interpretation.
// Carries the token where the error occurred for error reporting.
class RuntimeError : public runtime_error {
public:
    Token token;
    RuntimeError(Token token, const string& message) : runtime_error(message), token(token) {}
};
