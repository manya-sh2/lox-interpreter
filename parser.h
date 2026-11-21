#pragma once

#include "ast_storage.h"
#include "token.h"

#include <vector>
#include <stdexcept>

using namespace std;

// PARSE ERROR
class ParseError : public runtime_error {
public:
    ParseError() : runtime_error("") {}
};

// PARSER
class Parser {
public:
    Parser(const vector<Token>& tokens, AstStorage& ast) : tokens(tokens), ast(ast), current(0) {}

    // PARSE
    vector<Stmt*> parse() {
        vector<Stmt*> statements;
        // only the root of the asts are stored. 
        // it is different fro aststorage.h
        // in that we store each objects pointer so that we can 
        // clear the memory at last
        while (!isAtEnd()) {
            Stmt* decl = declaration();
            if (decl != nullptr) {
                statements.push_back(decl);
            }
        }
        return statements;
    }
private:
    vector<Token> tokens;
    AstStorage& ast;
    int current;

    // DECLARATION
    Stmt* declaration() {
        try {
            // Function declaration
            if (match(FUN)) {
                return function("function");
            }
            // Variable declaration
            if (match(VAR)) {
                return varDeclaration();
            }
            return statement();
        } catch (ParseError&) {
            synchronize();
            return nullptr;
        }
    }

    // FUNCTION DECLARATION
    //
    // fun name(param1, param2) {
    //     ...
    // }
    Stmt* function(const string& kind) {
        Token name = consume(IDENTIFIER, "Expect " + kind + " name.");
        consume(LEFT_PAREN, "Expect '(' after " + kind + " name.");
        vector<Token> parameters;
        // Parameters
        if (!check(RIGHT_PAREN)) {
            do {
                if (parameters.size() >= 255) {
                    error(peek(), "Can't have more than 255 parameters.");
                }
                parameters.push_back(consume(IDENTIFIER, "Expect parameter name."));
            } while (match(COMMA));
        }
        consume(RIGHT_PAREN, "Expect ')' after parameters.");
        consume(LEFT_BRACE, "Expect '{' before " + kind + " body.");
        vector<Stmt*> body = block();
        return ast.saveStmt(new FunctionStmt(name, parameters, body));
    }

    // VARIABLE DECLARATION
    //
    // var x;
    // var x = expression;
    Stmt* varDeclaration() {
        Token name = consume(IDENTIFIER, "Expect variable name.");
        Expr* initializer = nullptr;
        if (match(EQUAL)) {
            initializer = expression();
        }
        consume(SEMICOLON, "Expect ';' after variable declaration.");
        return ast.saveStmt(new VarStmt(name, initializer));
    }

    // STATEMENT
    Stmt* statement() {
        // print
        if (match(PRINT)) {
            return printStatement();
        }
        // return
        if (match(RETURN)) {
            return returnStatement();
        }
        // block
        if (match(LEFT_BRACE)) {
            return ast.saveStmt(new BlockStmt(block()));
        }
        // if
        if (match(IF)) {
            return ifStatement();
        }
        // while
        if (match(WHILE)) {
            return whileStatement();
        }
        // for
        if (match(FOR)) {
            return forStatement();
        }
        // expression statement
        return expressionStatement();
    }

    // RETURN STATEMENT
    //
    // return;
    // return expression;
    Stmt* returnStatement() {
        Token keyword = previous();
        Expr* value = nullptr;
        if (!check(SEMICOLON)) {
            value = expression();
        }
        consume(SEMICOLON, "Expect ';' after return value.");
        return ast.saveStmt(new ReturnStmt(keyword, value));
    }

    // PRINT STATEMENT
    Stmt* printStatement() {
        Expr* value = expression();
        consume(SEMICOLON, "Expect ';' after value.");
        return ast.saveStmt(new PrintStmt(value));
    }

    // EXPRESSION STATEMENT
    Stmt* expressionStatement() {
        Expr* expr = expression();
        consume(SEMICOLON, "Expect ';' after expression.");
        return ast.saveStmt(new ExpressionStmt(expr));
    }

    // BLOCK
    vector<Stmt*> block() {
        vector<Stmt*> statements;
        while (!check(RIGHT_BRACE) && !isAtEnd()) {
            statements.push_back(declaration());
        }
        consume(RIGHT_BRACE, "Expect '}' after block.");
        return statements;
    }

    // IF
    Stmt* ifStatement() {
        consume(LEFT_PAREN, "Expect '(' after 'if'.");
        Expr* condition = expression();
        consume(RIGHT_PAREN, "Expect ')' after if condition.");
        Stmt* thenBranch = statement();
        Stmt* elseBranch = nullptr;
        if (match(ELSE)) {
            elseBranch = statement();
        }
        return ast.saveStmt(new IfStmt(condition, thenBranch, elseBranch));
    }

    // WHILE
    Stmt* whileStatement() {
        consume(LEFT_PAREN, "Expect '(' after 'while'.");
        Expr* condition = expression();
        consume(RIGHT_PAREN, "Expect ')' after condition.");
        Stmt* body = statement();
        return ast.saveStmt(new WhileStmt(condition, body));
    }

    // FOR
    Stmt* forStatement() {
        consume(LEFT_PAREN, "Expect '(' after 'for'.");
        Stmt* initializer = nullptr;
        // for (; ...)
        if (match(SEMICOLON)) {
            initializer = nullptr;
        }
        // for (var i = ...; ...)
        else if (match(VAR)) {
            initializer = varDeclaration();
        }
        // for (i = ...; ...)
        else {
            initializer = expressionStatement();
        }
        // ----------------------------------------------------
        // Condition
        // ----------------------------------------------------
        Expr* condition = nullptr;
        if (!check(SEMICOLON)) {
            condition = expression();
        }
        consume(SEMICOLON, "Expect ';' after loop condition.");
        // ----------------------------------------------------
        // Increment
        // ----------------------------------------------------
        Expr* increment = nullptr;
        if (!check(RIGHT_PAREN)) {
            increment = expression();
        }
        consume(RIGHT_PAREN, "Expect ')' after for clauses.");
        // ----------------------------------------------------
        // Body
        // ----------------------------------------------------
        Stmt* body = statement();
        // ----------------------------------------------------
        // Add increment at end of body
        // ----------------------------------------------------
        if (increment != nullptr) {
            vector<Stmt*> statements;
            statements.push_back(body);
            statements.push_back(ast.saveStmt(new ExpressionStmt(increment)));
            body = ast.saveStmt(new BlockStmt(statements));
        }
        // ----------------------------------------------------
        // No condition means true
        // ----------------------------------------------------
        if (condition == nullptr) {
            condition = ast.saveExpr(new Literal(Value(true)));
        }
        // ----------------------------------------------------
        // Convert for into while
        // ----------------------------------------------------
        body = ast.saveStmt(new WhileStmt(condition, body));
        // ----------------------------------------------------
        // Add initializer outside while
        // ----------------------------------------------------
        if (initializer != nullptr) {
            vector<Stmt*> statements;
            statements.push_back(initializer);
            statements.push_back(body);
            body = ast.saveStmt(new BlockStmt(statements));
        }
        return body;
    }

    // EXPRESSION
    Expr* expression() {
        return assignment();
    }

    // ASSIGNMENT
    Expr* assignment() {
        Expr* expr = logicOr();
        if (match(EQUAL)) {
            Token equals = previous();
            Expr* value = assignment();
            Variable* variable = dynamic_cast<Variable*>(expr);
            if (variable != nullptr) {
                return ast.saveExpr(new Assign(variable->name, value));
            }
            throw error(equals, "Invalid assignment target.");
        }
        return expr;
    }

    // LOGICAL OR
    Expr* logicOr() {
        Expr* expr = logicAnd();
        while (match(OR)) {
            Token op = previous();
            Expr* right = logicAnd();
            expr = ast.saveExpr(new Logical(expr, op, right));
        }
        return expr;
    }

    // LOGICAL AND
    Expr* logicAnd() {
        Expr* expr = equality();
        while (match(AND)) {
            Token op = previous();
            Expr* right = equality();
            expr = ast.saveExpr(new Logical(expr, op, right));
        }
        return expr;
    }

    // EQUALITY
    Expr* equality() {
        Expr* expr = comparison();
        while (match(BANG_EQUAL) || match(EQUAL_EQUAL)) {
            Token op = previous();
            Expr* right = comparison();
            expr = ast.saveExpr(new Binary(expr, op, right));
        }
        return expr;
    }

    // COMPARISON
    Expr* comparison() {
        Expr* expr = term();
        while (match(GREATER) || match(GREATER_EQUAL) || match(LESS) || match(LESS_EQUAL)) {
            Token op = previous();
            Expr* right = term();
            expr = ast.saveExpr(new Binary(expr, op, right));
        }
        return expr;
    }

    // TERM
    Expr* term() {
        Expr* expr = factor();
        while (match(MINUS) || match(PLUS)) {
            Token op = previous();
            Expr* right = factor();
            expr = ast.saveExpr(new Binary(expr, op, right));
        }
        return expr;
    }

    // FACTOR
    Expr* factor() {
        Expr* expr = unary();
        while (match(SLASH) || match(STAR)) {
            Token op = previous();
            Expr* right = unary();
            expr = ast.saveExpr(new Binary(expr, op, right));
        }
        return expr;
    }

    // UNARY
    Expr* unary() {
        if (match(BANG) || match(MINUS)) {
            Token op = previous();
            Expr* right = unary();
            return ast.saveExpr(new Unary(op, right));
        }
        return call();
    }

    // FUNCTION CALL
    Expr* call() {
        Expr* expr = primary();
        while (true) {
            if (match(LEFT_PAREN)) {
                expr = finishCall(expr);
            } else {
                break;
            }
        }
        return expr;
    }

    // FINISH CALL
    Expr* finishCall(Expr* callee) {
        vector<Expr*> arguments;
        if (!check(RIGHT_PAREN)) {
            do {
                if (arguments.size() >= 255) {
                    error(peek(), "Can't have more than 255 arguments.");
                }
                arguments.push_back(expression());
            } while (match(COMMA));
        }
        Token paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");
        return ast.saveExpr(new Call(callee, paren, arguments));
    }

    // PRIMARY
    Expr* primary() {
        // false
        if (match(FALSE)) {
            return ast.saveExpr(new Literal(Value(false)));
        }
        // true
        if (match(TRUE)) {
            return ast.saveExpr(new Literal(Value(true)));
        }
        // nil
        if (match(NIL)) {
            return ast.saveExpr(new Literal(Value()));
        }
        // number
        if (match(NUMBER)) {
            double num = stod(previous().literal.raw);
            return ast.saveExpr(new Literal(Value(num)));
        }
        // string                                                                                              
        if (match(STRING)) {
            return ast.saveExpr(new Literal(Value(previous().literal.raw)));
        }
        // variable
        if (match(IDENTIFIER)) {
            return ast.saveExpr(new Variable(previous()));
        }
        // grouping
        if (match(LEFT_PAREN)) {
            Expr* expr = expression();
            consume(RIGHT_PAREN, "Expect ')' after expression.");
            return ast.saveExpr(new Grouping(expr));
        }
        throw error(peek(), "Expect expression.");
    }

    // ERROR RECOVERY
    void synchronize() {
        advance();
        while (!isAtEnd()) {
            if (previous().type == SEMICOLON) {
                return;
            }
            switch (peek().type) {
                case CLASS:
                case FUN:
                case VAR:
                case FOR:
                case IF:
                case WHILE:
                case PRINT:
                case RETURN:
                    return;
                default:
                    break;
            }
            advance();
        }
    }

    // MATCH
    bool match(TokenType type) {
        if (!check(type)) return false;
        advance();
        return true;
    }

    // CHECK
    bool check(TokenType type) {
        if (isAtEnd()) {
            return (type == END_OF_FILE);
        }
        return (peek().type == type);
    }

    // ADVANCE
    Token advance() {
        if (!isAtEnd()) {
            current++;
        }
        return previous();
    }

    // END
    bool isAtEnd() {
        return (peek().type == END_OF_FILE);
    }

    // PEEK
    Token peek() {
        return tokens[current];
    }

    // PREVIOUS
    Token previous() {
        return tokens[current - 1];
    }

    // CONSUME
    Token consume(TokenType type, const string& message) {
        if (check(type)) {
            return advance();
        }
        throw error(peek(), message);
    }

    // ERROR
    ParseError error(Token token, const string& message) {
        if (token.type == END_OF_FILE) {
            ::error(token.line, " at end " + message);
        } else {
            ::error(token.line, " at '" + token.lexeme + "' " + message);
        }
        return ParseError();
    }
};
