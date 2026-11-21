#pragma once

#include "expr.h"

#include <string>
#include <sstream>

using namespace std;

// AST PRINTER
class AstPrinter : public StringVisitor {
public:
    string print(Expr* expr) {
        return expr->accept(this);
    }

    // BINARY
    string visitBinaryExpr(Binary* expr) override {
        return parenthesize(expr->op.lexeme, expr->left, expr->right);
    }

    // GROUPING
    string visitGroupingExpr(Grouping* expr) override {
        return parenthesize("group", expr->expression);
    }

    // LITERAL
    string visitLiteralExpr(Literal* expr) override {
        if (expr->value.isNil()) {
            return "nil";
        }
        if (expr->value.isBool()) {
            return expr->value.boolean ? "true" : "false";
        }
        if (expr->value.isNumber()) {
            double d = expr->value.number;
            if (d == static_cast<long long>(d)) {
                return to_string(static_cast<long long>(d));
            }
            ostringstream oss;
            oss << d;
            return oss.str();
        }
        if (expr->value.isString()) {
            return expr->value.str;
        }
        if (expr->value.isCallable()) {
            return expr->value.callable->toString();
        }
        return "nil";
    }

    // UNARY
    string visitUnaryExpr(Unary* expr) override {
        return parenthesize(expr->op.lexeme, expr->right);
    }

    // VARIABLE
    string visitVariableExpr(Variable* expr) override {
        return expr->name.lexeme;
    }

    // ASSIGN
    string visitAssignExpr(Assign* expr) override {
        return "(= " +
            expr->name.lexeme +
            " " +
            expr->value->accept(this) +
            ")";
    }

    // LOGICAL
    string visitLogicalExpr(Logical* expr) override {
        return parenthesize(expr->op.lexeme, expr->left, expr->right);
    }

    // CALL
    string visitCallExpr(Call* expr) override {
        string result = "(call ";
        result +=
            expr->callee->accept(this);
        for (Expr* argument : expr->arguments) {
            result += " ";
            result +=
                argument->accept(this);
        }
        result += ")";
        return result;
    }
private:
    // ONE CHILD
    string parenthesize(const string& name, Expr* expr) {
        return "(" +
            name +
            " " +
            expr->accept(this) +
            ")";
    }

    // TWO CHILDREN
    string parenthesize(const string& name, Expr* left, Expr* right) {
        return "(" +
            name +
            " " +
            left->accept(this) +
            " " +
            right->accept(this) +
            ")";
    }
};
