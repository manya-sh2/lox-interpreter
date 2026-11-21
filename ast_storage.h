#pragma once
#include "stmt.h"

// Keep function declarations and their bodies alive across REPL lines.
class AstStorage {
public:
    AstStorage() = default;
    AstStorage(const AstStorage&) = delete;
    AstStorage& operator=(const AstStorage&) = delete;
    Expr* saveExpr(Expr* expr) {
        expressions.push_back(expr);
        return expr;
    }

    Stmt* saveStmt(Stmt* stmt) {
        statements.push_back(stmt);
        return stmt;
    }

    ~AstStorage() {
        for (Stmt* stmt : statements) delete stmt;
        for (Expr* expr : expressions) delete expr;
    }
private:
    vector<Expr*> expressions;
    vector<Stmt*> statements;
};
