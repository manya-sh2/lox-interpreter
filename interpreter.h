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
#include <memory>
#include <vector>

using namespace std;


// ============================================================
// INTERPRETER
// ============================================================

class Interpreter : public ValueVisitor, public StmtVisitor {

public:

    shared_ptr<Environment> globals;

    shared_ptr<Environment> environment;


    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    Interpreter() {

        globals = make_shared<Environment>();

        environment = globals;
    }


    // ========================================================
    // INTERPRET
    // ========================================================

    void interpret(
        const vector<shared_ptr<Stmt>>& statements
    ) {

        try {

            for (auto& statement : statements) {

                execute(statement);
            }
        }
        catch (RuntimeError& error) {

            runtimeError(error);
        }
        catch (ReturnException&) {

            cerr
                << "Error: Can't return from top-level code."
                << endl;
        }
    }


    // ========================================================
    // EXPRESSION STATEMENT
    // ========================================================

    void visitExpressionStmt(
        ExpressionStmt* stmt
    ) override {

        evaluate(stmt->expression);
    }


    // ========================================================
    // PRINT
    // ========================================================

    void visitPrintStmt(
        PrintStmt* stmt
    ) override {

        Value value =
            evaluate(stmt->expression);


        cout
            << stringify(value)
            << endl;
    }


    // ========================================================
    // VARIABLE
    // ========================================================

    void visitVarStmt(
        VarStmt* stmt
    ) override {

        Value value;


        if (stmt->initializer != nullptr) {

            value =
                evaluate(stmt->initializer);
        }


        environment->define(
            stmt->name.lexeme,
            value
        );
    }


    // ========================================================
    // BLOCK
    // ========================================================

    void visitBlockStmt(
        BlockStmt* stmt
    ) override {

        shared_ptr<Environment> newEnvironment =
            make_shared<Environment>(
                environment
            );


        executeBlock(
            stmt->statements,
            newEnvironment
        );
    }


    // ========================================================
    // IF
    // ========================================================

    void visitIfStmt(
        IfStmt* stmt
    ) override {

        Value condition =
            evaluate(stmt->condition);


        if (isTruthy(condition)) {

            execute(stmt->thenBranch);
        }
        else if (
            stmt->elseBranch != nullptr
        ) {

            execute(stmt->elseBranch);
        }
    }


    // ========================================================
    // WHILE
    // ========================================================

    void visitWhileStmt(
        WhileStmt* stmt
    ) override {

        while (
            isTruthy(
                evaluate(stmt->condition)
            )
        ) {

            execute(stmt->body);
        }
    }


    // ========================================================
    // FUNCTION DECLARATION
    // ========================================================

    void visitFunctionStmt(
        FunctionStmt* stmt
    ) override {

        // Create a shared_ptr so the function object
        // can be stored inside Value.
        shared_ptr<FunctionStmt> declaration(
            stmt,
            [](FunctionStmt*) {}
        );


        shared_ptr<LoxFunction> function =
            make_shared<LoxFunction>(
                declaration,
                environment
            );


        environment->define(
            stmt->name.lexeme,
            Value(function)
        );
    }


    // ========================================================
    // RETURN
    // ========================================================

    void visitReturnStmt(
        ReturnStmt* stmt
    ) override {

        Value value;


        if (stmt->value != nullptr) {

            value =
                evaluate(stmt->value);
        }


        throw ReturnException(value);
    }


    // ========================================================
    // LITERAL
    // ========================================================

    Value visitLiteralExpr(
        Literal* expr
    ) override {

        return expr->value;
    }


    // ========================================================
    // GROUPING
    // ========================================================

    Value visitGroupingExpr(
        Grouping* expr
    ) override {

        return evaluate(
            expr->expression
        );
    }


    // ========================================================
    // UNARY
    // ========================================================

    Value visitUnaryExpr(
        Unary* expr
    ) override {

        Value right =
            evaluate(expr->right);


        switch (expr->op.type) {

            case MINUS:

                checkNumberOperand(
                    expr->op,
                    right
                );

                return Value(
                    -right.number
                );


            case BANG:

                return Value(
                    !isTruthy(right)
                );


            default:

                break;
        }


        return Value();
    }


    // ========================================================
    // BINARY
    // ========================================================

    Value visitBinaryExpr(
        Binary* expr
    ) override {

        Value left =
            evaluate(expr->left);


        Value right =
            evaluate(expr->right);


        switch (expr->op.type) {

            case MINUS:

                checkNumberOperands(
                    expr->op,
                    left,
                    right
                );

                return Value(
                    left.number -
                    right.number
                );


            case SLASH:

                checkNumberOperands(
                    expr->op,
                    left,
                    right
                );

                return Value(
                    left.number /
                    right.number
                );


            case STAR:

                checkNumberOperands(
                    expr->op,
                    left,
                    right
                );

                return Value(
                    left.number *
                    right.number
                );


            case PLUS:

                if (
                    left.isNumber() &&
                    right.isNumber()
                ) {

                    return Value(
                        left.number +
                        right.number
                    );
                }


                if (
                    left.isString() &&
                    right.isString()
                ) {

                    return Value(
                        left.str +
                        right.str
                    );
                }


                throw RuntimeError(
                    expr->op,
                    "Operands must be two numbers or two strings."
                );


            case GREATER:

                checkNumberOperands(
                    expr->op,
                    left,
                    right
                );

                return Value(
                    left.number >
                    right.number
                );


            case GREATER_EQUAL:

                checkNumberOperands(
                    expr->op,
                    left,
                    right
                );

                return Value(
                    left.number >=
                    right.number
                );


            case LESS:

                checkNumberOperands(
                    expr->op,
                    left,
                    right
                );

                return Value(
                    left.number <
                    right.number
                );


            case LESS_EQUAL:

                checkNumberOperands(
                    expr->op,
                    left,
                    right
                );

                return Value(
                    left.number <=
                    right.number
                );


            case BANG_EQUAL:

                return Value(
                    !isEqual(
                        left,
                        right
                    )
                );


            case EQUAL_EQUAL:

                return Value(
                    isEqual(
                        left,
                        right
                    )
                );


            default:

                break;
        }


        return Value();
    }


    // ========================================================
    // VARIABLE EXPRESSION
    // ========================================================

    Value visitVariableExpr(
        Variable* expr
    ) override {

        return environment->get(
            expr->name
        );
    }


    // ========================================================
    // ASSIGNMENT
    // ========================================================

    Value visitAssignExpr(
        Assign* expr
    ) override {

        Value value =
            evaluate(expr->value);


        environment->assign(
            expr->name,
            value
        );


        return value;
    }


    // ========================================================
    // LOGICAL
    // ========================================================

    Value visitLogicalExpr(
        Logical* expr
    ) override {

        Value left =
            evaluate(expr->left);


        // OR:
        // if left is truthy, don't evaluate right.
        if (
            expr->op.type == OR
        ) {

            if (isTruthy(left)) {

                return left;
            }
        }
        else {

            // AND:
            // if left is false, don't evaluate right.
            if (!isTruthy(left)) {

                return left;
            }
        }


        return evaluate(
            expr->right
        );
    }


    // ========================================================
    // FUNCTION CALL
    // ========================================================

    Value visitCallExpr(
        Call* expr
    ) override {

        Value callee =
            evaluate(expr->callee);


        vector<Value> arguments;


        for (
            auto& argument :
            expr->arguments
        ) {

            arguments.push_back(
                evaluate(argument)
            );
        }


        if (!callee.isCallable()) {

            throw RuntimeError(
                expr->paren,
                "Can only call functions and classes."
            );
        }


        shared_ptr<LoxCallable> function =
            callee.callable;


        if (
            arguments.size() !=
            static_cast<size_t>(
                function->arity()
            )
        ) {

            throw RuntimeError(
                expr->paren,
                "Expected " +
                to_string(function->arity()) +
                " arguments but got " +
                to_string(arguments.size()) +
                "."
            );
        }


        return function->call(
            this,
            arguments
        );
    }


    // ========================================================
    // EXECUTE BLOCK
    // ========================================================

    void executeBlock(
        const vector<shared_ptr<Stmt>>& statements,
        shared_ptr<Environment> newEnvironment
    ) {

        shared_ptr<Environment> previous =
            environment;


        try {

            environment =
                newEnvironment;


            for (auto& statement : statements) {

                execute(statement);
            }
        }
        catch (...) {

            environment =
                previous;

            throw;
        }


        environment =
            previous;
    }


private:

    // ========================================================
    // EXECUTE
    // ========================================================

    void execute(
        shared_ptr<Stmt> stmt
    ) {

        stmt->accept(
            static_cast<StmtVisitor*>(this)
        );
    }


    // ========================================================
    // EVALUATE
    // ========================================================

    Value evaluate(
        shared_ptr<Expr> expr
    ) {

        return expr->accept(
            static_cast<ValueVisitor*>(this)
        );
    }


    // ========================================================
    // TRUTHINESS
    // ========================================================

    bool isTruthy(
        const Value& value
    ) {

        if (value.isNil()) {

            return false;
        }


        if (value.isBool()) {

            return value.boolean;
        }


        return true;
    }


    // ========================================================
    // EQUALITY
    // ========================================================

    bool isEqual(
        const Value& a,
        const Value& b
    ) {

        if (a.type != b.type) {

            return false;
        }


        if (a.isNil()) {

            return true;
        }


        if (a.isBool()) {

            return
                a.boolean ==
                b.boolean;
        }


        if (a.isNumber()) {

            return
                a.number ==
                b.number;
        }


        if (a.isString()) {

            return
                a.str ==
                b.str;
        }


        if (a.isCallable()) {

            return
                a.callable ==
                b.callable;
        }


        return false;
    }


    // ========================================================
    // NUMBER CHECK
    // ========================================================

    void checkNumberOperand(
        const Token& op,
        const Value& operand
    ) {

        if (operand.isNumber()) {

            return;
        }


        throw RuntimeError(
            op,
            "Operand must be a number."
        );
    }


    // ========================================================
    // TWO NUMBER CHECK
    // ========================================================

    void checkNumberOperands(
        const Token& op,
        const Value& left,
        const Value& right
    ) {

        if (
            left.isNumber() &&
            right.isNumber()
        ) {

            return;
        }


        throw RuntimeError(
            op,
            "Operands must be numbers."
        );
    }


    // ========================================================
    // STRINGIFY
    // ========================================================

    string stringify(
        const Value& value
    ) {

        if (value.isNil()) {

            return "nil";
        }


        if (value.isBool()) {

            return
                value.boolean
                ? "true"
                : "false";
        }


        if (value.isNumber()) {

            double d =
                value.number;


            if (
                d ==
                static_cast<long long>(d)
            ) {

                return to_string(
                    static_cast<long long>(d)
                );
            }


            char buffer[50];

            sprintf(
                buffer,
                "%.15g",
                d
            );


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


    // ========================================================
    // RUNTIME ERROR
    // ========================================================

    void runtimeError(
        RuntimeError& error
    ) {

        cerr
            << error.what()
            << endl;
    }
};


// ============================================================
// LOX FUNCTION CALL IMPLEMENTATION
// ============================================================

Value LoxFunction::call(
    Interpreter* interpreter,
    const vector<Value>& arguments
) {

    shared_ptr<Environment> functionEnvironment =
        make_shared<Environment>(
            closure
        );


    for (
        size_t i = 0;
        i < declaration->params.size();
        i++
    ) {

        functionEnvironment->define(
            declaration->params[i].lexeme,
            arguments[i]
        );
    }


    try {

        interpreter->executeBlock(
            declaration->body,
            functionEnvironment
        );
    }
    catch (ReturnException& returnValue) {

        return returnValue.value;
    }


    // Function without return
    // implicitly returns nil.
    return Value();
}