#pragma once

#include "expr.h"
#include "stmt.h"
#include "environment.h"
#include "runtime_error.h"
#include "lox_function.h"
#include "return.h"

#include <iostream>
#include <string>
#include <cstdio>
#include <vector>

using namespace std;

// INTERPRETER
class Interpreter : public ValueVisitor, public StmtVisitor {
public:
    Environment* globals;
    Environment* environment;

    // CONSTRUCTOR
    Interpreter() {
        globals = new Environment();
        environment = globals;
    }

    Interpreter(const Interpreter&) = delete;
    Interpreter& operator=(const Interpreter&) = delete;
    ~Interpreter() {
        for (LoxFunction* function : functions) delete function;
        for (Environment* scope : capturedScopes) delete scope;
        delete globals;
    }

    // INTERPRET
    void interpret(const vector<Stmt*>& statements) {
        try {
            for (Stmt* statement : statements) {
                execute(statement);
            }
        } catch (RuntimeError& error) {
            runtimeError(error);
        } catch (ReturnException&) {
            cerr << "Error: Can't return from top-level code." << endl;
        }
    }

    // EXPRESSION STATEMENT
    void visitExpressionStmt(ExpressionStmt* stmt) override {
        evaluate(stmt->expression);
    }

    // PRINT
    void visitPrintStmt(PrintStmt* stmt) override {
        Value value = evaluate(stmt->expression);
        cout << stringify(value) << endl;
    }

    // VARIABLE
    void visitVarStmt(VarStmt* stmt) override {
        Value value;
        if (stmt->initializer != nullptr) {
            value = evaluate(stmt->initializer);
        }
        environment->define(stmt->name.lexeme, value);
    }

    // BLOCK
    void visitBlockStmt(BlockStmt* stmt) override {
        Environment* newEnvironment = new Environment(environment);
        executeBlock(stmt->statements, newEnvironment);
    }

    // IF
    void visitIfStmt(IfStmt* stmt) override {
        Value condition = evaluate(stmt->condition);
        if (isTruthy(condition)) {
            execute(stmt->thenBranch);
        } else if (stmt->elseBranch != nullptr) {
            execute(stmt->elseBranch);
        }
    }

    // WHILE
    void visitWhileStmt(WhileStmt* stmt) override {
        while (isTruthy(evaluate(stmt->condition))) {
            execute(stmt->body);
        }
    }

    // FUNCTION DECLARATION
    void visitFunctionStmt(FunctionStmt* stmt) override {
        LoxFunction* function = new LoxFunction(stmt, environment);
        functions.push_back(function);
        // A closure may look up variables in any of its parent scopes.
        Environment* scope = environment;
        while (scope != globals && !scope->captured) {
            capturedScopes.push_back(scope);
            scope->captured = true;
            scope = scope->enclosing;
        }
        environment->define(stmt->name.lexeme, Value(function));
    }

    // RETURN
    void visitReturnStmt(ReturnStmt* stmt) override {
        Value value;
        if (stmt->value != nullptr) {
            value = evaluate(stmt->value);
        }
        throw ReturnException(value);
    }

    
    // these things are evaluated because they are expressions
    // LITERAL
    Value visitLiteralExpr(Literal* expr) override {
        return expr->value;
    }

    // GROUPING
    Value visitGroupingExpr(Grouping* expr) override {
        return evaluate(expr->expression);
    }

    // UNARY
    Value visitUnaryExpr(Unary* expr) override {
        Value right = evaluate(expr->right);
        switch (expr->op.type) {
            case MINUS:
                checkNumberOperand(expr->op, right);
                return Value(-right.number);
            case BANG:
                return Value(!isTruthy(right));
            default:
                break;
        }
        return Value();
    }

    // BINARY
    Value visitBinaryExpr(Binary* expr) override {
        Value left = evaluate(expr->left);
        Value right = evaluate(expr->right);
        switch (expr->op.type) {
            case MINUS:
                checkNumberOperands(expr->op, left, right);
                return Value(left.number - right.number);
            case SLASH:
                checkNumberOperands(expr->op, left, right);
                return Value(left.number / right.number);
            case STAR:
                checkNumberOperands(expr->op, left, right);
                return Value(left.number * right.number);
            case PLUS:
                if (left.isNumber() && right.isNumber()) {
                    return Value(left.number + right.number);
                }
                if (left.isString() && right.isString()) {
                    return Value(left.str + right.str);
                }
                throw RuntimeError(expr->op, "Operands must be two numbers or two strings.");
            case GREATER:
                checkNumberOperands(expr->op, left, right);
                return Value(left.number > right.number);
            case GREATER_EQUAL:
                checkNumberOperands(expr->op, left, right);
                return Value(left.number >= right.number);
            case LESS:
                checkNumberOperands(expr->op, left, right);
                return Value(left.number < right.number);
            case LESS_EQUAL:
                checkNumberOperands(expr->op, left, right);
                return Value(left.number <= right.number);
            case BANG_EQUAL:
                return Value(!isEqual(left, right));
            case EQUAL_EQUAL:
                return Value(isEqual(left, right));
            default:
                break;
        }
        return Value();
    }

    // VARIABLE EXPRESSION
    Value visitVariableExpr(Variable* expr) override {
        return environment->get(expr->name);
    }

    // ASSIGNMENT
    Value visitAssignExpr(Assign* expr) override {
        Value value = evaluate(expr->value);
        environment->assign(expr->name, value);
        return value;
    }

    // LOGICAL
    Value visitLogicalExpr(Logical* expr) override {
        Value left = evaluate(expr->left);
        // OR:
        // if left is truthy, don't evaluate right.
        if (expr->op.type == OR) {
            if (isTruthy(left)) {
                return left;
            }
        } else {
            // AND:
            // if left is false, don't evaluate right.
            if (!isTruthy(left)) {
                return left;
            }
        }
        return evaluate(expr->right);
    }

    // FUNCTION CALL
    Value visitCallExpr(Call* expr) override {
        Value callee = evaluate(expr->callee);
        vector<Value> arguments;
        for (Expr* argument : expr->arguments) {
            arguments.push_back(evaluate(argument));
        }
        if (!callee.isCallable()) {
            throw RuntimeError(expr->paren, "Can only call functions and classes.");
        }
        LoxCallable* function = callee.callable;
        size_t expected = function->arity();
        if (arguments.size() != expected) {
            throw RuntimeError(
                expr->paren,
                "Expected " +
                to_string(function->arity()) +
                " arguments but got " +
                to_string(arguments.size()) +
                "."
            );
        }
        return function->call(this, arguments);
    }

    // EXECUTE BLOCK
    void executeBlock(const vector<Stmt*>& statements, Environment* newEnvironment) {
        Environment* previous = environment;
        try {
            environment = newEnvironment;
            for (Stmt* statement : statements) {
                execute(statement);
            }
        } catch (...) {
            environment = previous;
            if (!newEnvironment->captured) delete newEnvironment;
            throw;
        }
        environment = previous;
        if (!newEnvironment->captured) delete newEnvironment;
    }
private:
    vector<LoxFunction*> functions;
    vector<Environment*> capturedScopes;

    // EXECUTE
    void execute(Stmt* stmt) {
        stmt->accept(this);
    }

    // EVALUATE
    Value evaluate(Expr* expr) {
        return expr->accept(this);
    }

    // TRUTHINESS
    bool isTruthy(const Value& value) {
        if (value.isNil()) {
            return false;
        }
        if (value.isBool()) {
            return value.boolean;
        }
        return true;
    }

    // EQUALITY
    bool isEqual(const Value& a, const Value& b) {
        if (a.type != b.type) {
            return false;
        }
        if (a.isNil()) {
            return true;
        }
        if (a.isBool()) {
            return a.boolean == b.boolean;
        }
        if (a.isNumber()) {
            return a.number == b.number;
        }
        if (a.isString()) {
            return a.str == b.str;
        }
        if (a.isCallable()) {
            return a.callable == b.callable;
        }
        return false;
    }

    // NUMBER CHECK
    void checkNumberOperand(const Token& op, const Value& operand) {
        if (operand.isNumber()) {
            return;
        }
        throw RuntimeError(op, "Operand must be a number.");
    }

    // TWO NUMBER CHECK
    void checkNumberOperands(const Token& op, const Value& left, const Value& right) {
        if (left.isNumber() && right.isNumber()) {
            return;
        }
        throw RuntimeError(op, "Operands must be numbers.");
    }

    // STRINGIFY
    string stringify(const Value& value) {
        if (value.isNil()) {
            return "nil";
        }
        if (value.isBool()) {
            return value.boolean ? "true" : "false";
        }
        if (value.isNumber()) {
            double d = value.number;
            if (d == static_cast<long long>(d)) {
                return to_string(static_cast<long long>(d));
            }
            char buffer[50];
            sprintf(buffer, "%.15g", d);
            return string(buffer);
        }
        if (value.isString()) {
            return value.str;
        }
        if (value.isCallable()) {
            return value.callable->toString();
        }
        return "nil";
    }

    // RUNTIME ERROR
    void runtimeError(RuntimeError& error) {
        cerr << error.what() << endl;
    }
};

// LOX FUNCTION CALL IMPLEMENTATION
Value LoxFunction::call(Interpreter* interpreter, const vector<Value>& arguments) {
    Environment* functionEnvironment = new Environment(closure);
    for (size_t i = 0; i < declaration->params.size(); i++) {
        functionEnvironment->define(declaration->params[i].lexeme, arguments[i]);
    }

    try {
        interpreter->executeBlock(declaration->body, functionEnvironment);
    } catch (ReturnException& returnValue) {
        return returnValue.value;
    }

    // Function without return
    // implicitly returns nil.
    return Value();
}
