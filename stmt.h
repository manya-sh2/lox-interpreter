#pragma once

#include "expr.h"
#include <vector>

using namespace std;

// FORWARD DECLARATIONS
class ExpressionStmt;
class PrintStmt;
class VarStmt;
class BlockStmt;
class IfStmt;
class WhileStmt;
class FunctionStmt;
class ReturnStmt;

// STATEMENT VISITOR
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

// BASE STATEMENT
class Stmt {
public:
    virtual ~Stmt() {}

    virtual void accept(StmtVisitor* visitor) = 0;
};

// EXPRESSION STATEMENT
class ExpressionStmt : public Stmt {
public:
    Expr* expression;
    ExpressionStmt(Expr* expression) : expression(expression) {}
    void accept(StmtVisitor* visitor) override {
        visitor->visitExpressionStmt(this);
    }
};

// PRINT STATEMENT
class PrintStmt : public Stmt {
public:
    Expr* expression;
    PrintStmt(Expr* expression) : expression(expression) {}
    void accept(StmtVisitor* visitor) override {
        visitor->visitPrintStmt(this);
    }
};

// VARIABLE STATEMENT
class VarStmt : public Stmt {
public:
    Token name;
    Expr* initializer;
    VarStmt(Token name, Expr* initializer)
        : name(name),
          initializer(initializer) {}
    void accept(StmtVisitor* visitor) override {
        visitor->visitVarStmt(this);
    }
};

// BLOCK STATEMENT
class BlockStmt : public Stmt {
public:
    vector<Stmt*> statements;
    BlockStmt(vector<Stmt*> statements) : statements(statements) {}
    void accept(StmtVisitor* visitor) override {
        visitor->visitBlockStmt(this);
    }
};

// IF STATEMENT
class IfStmt : public Stmt {
public:
    Expr* condition;
    Stmt* thenBranch;
    Stmt* elseBranch;
    IfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch)
        : condition(condition),
          thenBranch(thenBranch),
          elseBranch(elseBranch) {}
    void accept(StmtVisitor* visitor) override {
        visitor->visitIfStmt(this);
    }
};

// WHILE STATEMENT
class WhileStmt : public Stmt {
public:
    Expr* condition;
    Stmt* body;
    WhileStmt(Expr* condition, Stmt* body)
        : condition(condition),
          body(body) {}
    void accept(StmtVisitor* visitor) override {
        visitor->visitWhileStmt(this);
    }
};

// FUNCTION STATEMENT
class FunctionStmt : public Stmt {
public:
    Token name;
    vector<Token> params;
    vector<Stmt*> body;
    FunctionStmt(Token name, vector<Token> params, vector<Stmt*> body)
        : name(name),
          params(params),
          body(body) {}
    void accept(StmtVisitor* visitor) override {
        visitor->visitFunctionStmt(this);
    }
};

// RETURN STATEMENT
class ReturnStmt : public Stmt {
public:
    Token keyword;
    Expr* value;
    ReturnStmt(Token keyword, Expr* value)
        : keyword(keyword),
          value(value) {}
    void accept(StmtVisitor* visitor) override {
        visitor->visitReturnStmt(this);
    }
};
