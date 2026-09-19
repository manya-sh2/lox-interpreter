#pragma once

#include "expr.h"
#include <memory>
#include <vector>

using namespace std;


// ============================================================
// FORWARD DECLARATIONS
// ============================================================

class ExpressionStmt;
class PrintStmt;
class VarStmt;
class BlockStmt;
class IfStmt;
class WhileStmt;
class FunctionStmt;
class ReturnStmt;


// ============================================================
// STATEMENT VISITOR
// ============================================================

class StmtVisitor {
public:

    virtual ~StmtVisitor() {}

    virtual void visitExpressionStmt(ExpressionStmt* stmt) = 0;

    virtual void visitPrintStmt(PrintStmt* stmt) = 0;

    virtual void visitVarStmt(VarStmt* stmt) = 0;

    virtual void visitBlockStmt(BlockStmt* stmt) = 0;

    virtual void visitIfStmt(IfStmt* stmt) = 0;

    virtual void visitWhileStmt(WhileStmt* stmt) = 0;

    virtual void visitFunctionStmt(FunctionStmt* stmt) = 0;

    virtual void visitReturnStmt(ReturnStmt* stmt) = 0;
};


// ============================================================
// BASE STATEMENT
// ============================================================

class Stmt {
public:

    virtual ~Stmt() {}

    virtual void accept(StmtVisitor* visitor) = 0;
};


// ============================================================
// EXPRESSION STATEMENT
// ============================================================

class ExpressionStmt : public Stmt {
public:

    shared_ptr<Expr> expression;

    ExpressionStmt(shared_ptr<Expr> expression)
        : expression(expression) {}

    void accept(StmtVisitor* visitor) override {
        visitor->visitExpressionStmt(this);
    }
};


// ============================================================
// PRINT STATEMENT
// ============================================================

class PrintStmt : public Stmt {
public:

    shared_ptr<Expr> expression;

    PrintStmt(shared_ptr<Expr> expression)
        : expression(expression) {}

    void accept(StmtVisitor* visitor) override {
        visitor->visitPrintStmt(this);
    }
};


// ============================================================
// VARIABLE STATEMENT
// ============================================================

class VarStmt : public Stmt {
public:

    Token name;

    shared_ptr<Expr> initializer;

    VarStmt(
        Token name,
        shared_ptr<Expr> initializer
    )
        : name(name),
          initializer(initializer) {}

    void accept(StmtVisitor* visitor) override {
        visitor->visitVarStmt(this);
    }
};


// ============================================================
// BLOCK STATEMENT
// ============================================================

class BlockStmt : public Stmt {
public:

    vector<shared_ptr<Stmt>> statements;

    BlockStmt(vector<shared_ptr<Stmt>> statements)
        : statements(statements) {}

    void accept(StmtVisitor* visitor) override {
        visitor->visitBlockStmt(this);
    }
};


// ============================================================
// IF STATEMENT
// ============================================================

class IfStmt : public Stmt {
public:

    shared_ptr<Expr> condition;

    shared_ptr<Stmt> thenBranch;

    shared_ptr<Stmt> elseBranch;

    IfStmt(
        shared_ptr<Expr> condition,
        shared_ptr<Stmt> thenBranch,
        shared_ptr<Stmt> elseBranch
    )
        : condition(condition),
          thenBranch(thenBranch),
          elseBranch(elseBranch) {}

    void accept(StmtVisitor* visitor) override {
        visitor->visitIfStmt(this);
    }
};


// ============================================================
// WHILE STATEMENT
// ============================================================

class WhileStmt : public Stmt {
public:

    shared_ptr<Expr> condition;

    shared_ptr<Stmt> body;

    WhileStmt(
        shared_ptr<Expr> condition,
        shared_ptr<Stmt> body
    )
        : condition(condition),
          body(body) {}

    void accept(StmtVisitor* visitor) override {
        visitor->visitWhileStmt(this);
    }
};


// ============================================================
// FUNCTION STATEMENT
// ============================================================

class FunctionStmt : public Stmt {
public:

    Token name;

    vector<Token> params;

    vector<shared_ptr<Stmt>> body;

    FunctionStmt(
        Token name,
        vector<Token> params,
        vector<shared_ptr<Stmt>> body
    )
        : name(name),
          params(params),
          body(body) {}

    void accept(StmtVisitor* visitor) override {
        visitor->visitFunctionStmt(this);
    }
};


// ============================================================
// RETURN STATEMENT
// ============================================================

class ReturnStmt : public Stmt {
public:

    Token keyword;

    shared_ptr<Expr> value;

    ReturnStmt(
        Token keyword,
        shared_ptr<Expr> value
    )
        : keyword(keyword),
          value(value) {}

    void accept(StmtVisitor* visitor) override {
        visitor->visitReturnStmt(this);
    }
};