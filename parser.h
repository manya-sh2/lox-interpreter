#pragma once

#include "expr.h"
#include "stmt.h"
#include "token.h"

#include <vector>
#include <memory>
#include <stdexcept>
#include <initializer_list>

using namespace std;


// ============================================================
// PARSE ERROR
// ============================================================

class ParseError : public runtime_error {
public:

    ParseError()
        : runtime_error("") {}
};


// ============================================================
// PARSER
// ============================================================

class Parser {

public:

    Parser(const vector<Token>& tokens)
        : tokens(tokens),
          current(0) {}


    // ========================================================
    // PARSE
    // ========================================================

    vector<shared_ptr<Stmt>> parse() {

        vector<shared_ptr<Stmt>> statements;


        while (!isAtEnd()) {

            shared_ptr<Stmt> decl =
                declaration();


            if (decl != nullptr) {

                statements.push_back(decl);
            }
        }


        return statements;
    }


private:

    vector<Token> tokens;

    int current;


    // ========================================================
    // DECLARATION
    // ========================================================

    shared_ptr<Stmt> declaration() {

        try {

            // Function declaration
            if (match({FUN})) {

                return function("function");
            }


            // Variable declaration
            if (match({VAR})) {

                return varDeclaration();
            }


            return statement();
        }
        catch (ParseError&) {

            synchronize();

            return nullptr;
        }
    }


    // ========================================================
    // FUNCTION DECLARATION
    //
    // fun name(param1, param2) {
    //     ...
    // }
    // ========================================================

    shared_ptr<Stmt> function(
        const string& kind
    ) {

        Token name =
            consume(
                IDENTIFIER,
                "Expect " + kind + " name."
            );


        consume(
            LEFT_PAREN,
            "Expect '(' after " + kind + " name."
        );


        vector<Token> parameters;


        // Parameters
        if (!check(RIGHT_PAREN)) {

            do {

                if (parameters.size() >= 255) {

                    error(
                        peek(),
                        "Can't have more than 255 parameters."
                    );
                }


                parameters.push_back(
                    consume(
                        IDENTIFIER,
                        "Expect parameter name."
                    )
                );


            } while (match({COMMA}));
        }


        consume(
            RIGHT_PAREN,
            "Expect ')' after parameters."
        );


        consume(
            LEFT_BRACE,
            "Expect '{' before " + kind + " body."
        );


        vector<shared_ptr<Stmt>> body =
            block();


        return make_shared<FunctionStmt>(
            name,
            parameters,
            body
        );
    }


    // ========================================================
    // VARIABLE DECLARATION
    //
    // var x;
    // var x = expression;
    // ========================================================

    shared_ptr<Stmt> varDeclaration() {

        Token name =
            consume(
                IDENTIFIER,
                "Expect variable name."
            );


        shared_ptr<Expr> initializer =
            nullptr;


        if (match({EQUAL})) {

            initializer =
                expression();
        }


        consume(
            SEMICOLON,
            "Expect ';' after variable declaration."
        );


        return make_shared<VarStmt>(
            name,
            initializer
        );
    }


    // ========================================================
    // STATEMENT
    // ========================================================

    shared_ptr<Stmt> statement() {

        // print
        if (match({PRINT})) {

            return printStatement();
        }


        // return
        if (match({RETURN})) {

            return returnStatement();
        }


        // block
        if (match({LEFT_BRACE})) {

            return make_shared<BlockStmt>(
                block()
            );
        }


        // if
        if (match({IF})) {

            return ifStatement();
        }


        // while
        if (match({WHILE})) {

            return whileStatement();
        }


        // for
        if (match({FOR})) {

            return forStatement();
        }


        // expression statement
        return expressionStatement();
    }


    // ========================================================
    // RETURN STATEMENT
    //
    // return;
    // return expression;
    // ========================================================

    shared_ptr<Stmt> returnStatement() {

        Token keyword =
            previous();


        shared_ptr<Expr> value =
            nullptr;


        if (!check(SEMICOLON)) {

            value =
                expression();
        }


        consume(
            SEMICOLON,
            "Expect ';' after return value."
        );


        return make_shared<ReturnStmt>(
            keyword,
            value
        );
    }


    // ========================================================
    // PRINT STATEMENT
    // ========================================================

    shared_ptr<Stmt> printStatement() {

        shared_ptr<Expr> value =
            expression();


        consume(
            SEMICOLON,
            "Expect ';' after value."
        );


        return make_shared<PrintStmt>(
            value
        );
    }


    // ========================================================
    // EXPRESSION STATEMENT
    // ========================================================

    shared_ptr<Stmt> expressionStatement() {

        shared_ptr<Expr> expr =
            expression();


        consume(
            SEMICOLON,
            "Expect ';' after expression."
        );


        return make_shared<ExpressionStmt>(
            expr
        );
    }


    // ========================================================
    // BLOCK
    // ========================================================

    vector<shared_ptr<Stmt>> block() {

        vector<shared_ptr<Stmt>> statements;


        while (
            !check(RIGHT_BRACE) &&
            !isAtEnd()
        ) {

            statements.push_back(
                declaration()
            );
        }


        consume(
            RIGHT_BRACE,
            "Expect '}' after block."
        );


        return statements;
    }


    // ========================================================
    // IF
    // ========================================================

    shared_ptr<Stmt> ifStatement() {

        consume(
            LEFT_PAREN,
            "Expect '(' after 'if'."
        );


        shared_ptr<Expr> condition =
            expression();


        consume(
            RIGHT_PAREN,
            "Expect ')' after if condition."
        );


        shared_ptr<Stmt> thenBranch =
            statement();


        shared_ptr<Stmt> elseBranch =
            nullptr;


        if (match({ELSE})) {

            elseBranch =
                statement();
        }


        return make_shared<IfStmt>(
            condition,
            thenBranch,
            elseBranch
        );
    }


    // ========================================================
    // WHILE
    // ========================================================

    shared_ptr<Stmt> whileStatement() {

        consume(
            LEFT_PAREN,
            "Expect '(' after 'while'."
        );


        shared_ptr<Expr> condition =
            expression();


        consume(
            RIGHT_PAREN,
            "Expect ')' after condition."
        );


        shared_ptr<Stmt> body =
            statement();


        return make_shared<WhileStmt>(
            condition,
            body
        );
    }


    // ========================================================
    // FOR
    // ========================================================

    shared_ptr<Stmt> forStatement() {

        consume(
            LEFT_PAREN,
            "Expect '(' after 'for'."
        );


        shared_ptr<Stmt> initializer =
            nullptr;


        // for (; ...)
        if (match({SEMICOLON})) {

            initializer =
                nullptr;
        }


        // for (var i = ...; ...)
        else if (match({VAR})) {

            initializer =
                varDeclaration();
        }


        // for (i = ...; ...)
        else {

            initializer =
                expressionStatement();
        }


        // ----------------------------------------------------
        // Condition
        // ----------------------------------------------------

        shared_ptr<Expr> condition =
            nullptr;


        if (!check(SEMICOLON)) {

            condition =
                expression();
        }


        consume(
            SEMICOLON,
            "Expect ';' after loop condition."
        );


        // ----------------------------------------------------
        // Increment
        // ----------------------------------------------------

        shared_ptr<Expr> increment =
            nullptr;


        if (!check(RIGHT_PAREN)) {

            increment =
                expression();
        }


        consume(
            RIGHT_PAREN,
            "Expect ')' after for clauses."
        );


        // ----------------------------------------------------
        // Body
        // ----------------------------------------------------

        shared_ptr<Stmt> body =
            statement();


        // ----------------------------------------------------
        // Add increment at end of body
        // ----------------------------------------------------

        if (increment != nullptr) {

            vector<shared_ptr<Stmt>> statements;


            statements.push_back(
                body
            );


            statements.push_back(
                make_shared<ExpressionStmt>(
                    increment
                )
            );


            body =
                make_shared<BlockStmt>(
                    statements
                );
        }


        // ----------------------------------------------------
        // No condition means true
        // ----------------------------------------------------

        if (condition == nullptr) {

            condition =
                make_shared<Literal>(
                    Value(true)
                );
        }


        // ----------------------------------------------------
        // Convert for into while
        // ----------------------------------------------------

        body =
            make_shared<WhileStmt>(
                condition,
                body
            );


        // ----------------------------------------------------
        // Add initializer outside while
        // ----------------------------------------------------

        if (initializer != nullptr) {

            vector<shared_ptr<Stmt>> statements;


            statements.push_back(
                initializer
            );


            statements.push_back(
                body
            );


            body =
                make_shared<BlockStmt>(
                    statements
                );
        }


        return body;
    }


    // ========================================================
    // EXPRESSION
    // ========================================================

    shared_ptr<Expr> expression() {

        return assignment();
    }


    // ========================================================
    // ASSIGNMENT
    // ========================================================

    shared_ptr<Expr> assignment() {

        shared_ptr<Expr> expr =
            logicOr();


        if (match({EQUAL})) {

            Token equals =
                previous();


            shared_ptr<Expr> value =
                assignment();


            Variable* variable =
                dynamic_cast<Variable*>(
                    expr.get()
                );


            if (variable != nullptr) {

                return make_shared<Assign>(
                    variable->name,
                    value
                );
            }


            throw error(
                equals,
                "Invalid assignment target."
            );
        }


        return expr;
    }


    // ========================================================
    // LOGICAL OR
    // ========================================================

    shared_ptr<Expr> logicOr() {

        shared_ptr<Expr> expr =
            logicAnd();


        while (match({OR})) {

            Token op =
                previous();


            shared_ptr<Expr> right =
                logicAnd();


            expr =
                make_shared<Logical>(
                    expr,
                    op,
                    right
                );
        }


        return expr;
    }


    // ========================================================
    // LOGICAL AND
    // ========================================================

    shared_ptr<Expr> logicAnd() {

        shared_ptr<Expr> expr =
            equality();


        while (match({AND})) {

            Token op =
                previous();


            shared_ptr<Expr> right =
                equality();


            expr =
                make_shared<Logical>(
                    expr,
                    op,
                    right
                );
        }


        return expr;
    }


    // ========================================================
    // EQUALITY
    // ========================================================

    shared_ptr<Expr> equality() {

        shared_ptr<Expr> expr =
            comparison();


        while (
            match({
                BANG_EQUAL,
                EQUAL_EQUAL
            })
        ) {

            Token op =
                previous();


            shared_ptr<Expr> right =
                comparison();


            expr =
                make_shared<Binary>(
                    expr,
                    op,
                    right
                );
        }


        return expr;
    }


    // ========================================================
    // COMPARISON
    // ========================================================

    shared_ptr<Expr> comparison() {

        shared_ptr<Expr> expr =
            term();


        while (
            match({
                GREATER,
                GREATER_EQUAL,
                LESS,
                LESS_EQUAL
            })
        ) {

            Token op =
                previous();


            shared_ptr<Expr> right =
                term();


            expr =
                make_shared<Binary>(
                    expr,
                    op,
                    right
                );
        }


        return expr;
    }


    // ========================================================
    // TERM
    // ========================================================

    shared_ptr<Expr> term() {

        shared_ptr<Expr> expr =
            factor();


        while (
            match({
                MINUS,
                PLUS
            })
        ) {

            Token op =
                previous();


            shared_ptr<Expr> right =
                factor();


            expr =
                make_shared<Binary>(
                    expr,
                    op,
                    right
                );
        }


        return expr;
    }


    // ========================================================
    // FACTOR
    // ========================================================

    shared_ptr<Expr> factor() {

        shared_ptr<Expr> expr =
            unary();


        while (
            match({
                SLASH,
                STAR
            })
        ) {

            Token op =
                previous();


            shared_ptr<Expr> right =
                unary();


            expr =
                make_shared<Binary>(
                    expr,
                    op,
                    right
                );
        }


        return expr;
    }


    // ========================================================
    // UNARY
    // ========================================================

    shared_ptr<Expr> unary() {

        if (
            match({
                BANG,
                MINUS
            })
        ) {

            Token op =
                previous();


            shared_ptr<Expr> right =
                unary();


            return make_shared<Unary>(
                op,
                right
            );
        }


        return call();
    }


    // ========================================================
    // FUNCTION CALL
    // ========================================================

    shared_ptr<Expr> call() {

        shared_ptr<Expr> expr =
            primary();


        while (true) {

            if (match({LEFT_PAREN})) {

                expr =
                    finishCall(expr);
            }
            else {

                break;
            }
        }


        return expr;
    }


    // ========================================================
    // FINISH CALL
    // ========================================================

    shared_ptr<Expr> finishCall(
        shared_ptr<Expr> callee
    ) {

        vector<shared_ptr<Expr>> arguments;


        if (!check(RIGHT_PAREN)) {

            do {

                if (arguments.size() >= 255) {

                    error(
                        peek(),
                        "Can't have more than 255 arguments."
                    );
                }


                arguments.push_back(
                    expression()
                );


            } while (match({COMMA}));
        }


        Token paren =
            consume(
                RIGHT_PAREN,
                "Expect ')' after arguments."
            );


        return make_shared<Call>(
            callee,
            paren,
            arguments
        );
    }


    // ========================================================
    // PRIMARY
    // ========================================================

    shared_ptr<Expr> primary() {

        // false
        if (match({FALSE})) {

            return make_shared<Literal>(
                Value(false)
            );
        }


        // true
        if (match({TRUE})) {

            return make_shared<Literal>(
                Value(true)
            );
        }


        // nil
        if (match({NIL})) {

            return make_shared<Literal>(
                Value()
            );
        }


        // number
        if (match({NUMBER})) {

            double num =
                stod(
                    previous().literal.raw
                );


            return make_shared<Literal>(
                Value(num)
            );
        }


        // string
        if (match({STRING})) {

            return make_shared<Literal>(
                Value(
                    previous().literal.raw
                )
            );
        }


        // variable
        if (match({IDENTIFIER})) {

            return make_shared<Variable>(
                previous()
            );
        }


        // grouping
        if (match({LEFT_PAREN})) {

            shared_ptr<Expr> expr =
                expression();


            consume(
                RIGHT_PAREN,
                "Expect ')' after expression."
            );


            return make_shared<Grouping>(
                expr
            );
        }


        throw error(
            peek(),
            "Expect expression."
        );
    }


    // ========================================================
    // ERROR RECOVERY
    // ========================================================

    void synchronize() {

        advance();


        while (!isAtEnd()) {

            if (
                previous().type ==
                SEMICOLON
            ) {

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


    // ========================================================
    // MATCH
    //
    // IMPORTANT:
    // We use initializer_list only.
    //
    // So:
    // match({FUN})
    // match({VAR})
    // match({PLUS, MINUS})
    //
    // ========================================================

    bool match(
        initializer_list<TokenType> types
    ) {

        for (
            TokenType type :
            types
        ) {

            if (check(type)) {

                advance();

                return true;
            }
        }


        return false;
    }


    // ========================================================
    // CHECK
    // ========================================================

    bool check(
        TokenType type
    ) {

        if (isAtEnd()) {

            return (
                type ==
                END_OF_FILE
            );
        }


        return (
            peek().type ==
            type
        );
    }


    // ========================================================
    // ADVANCE
    // ========================================================

    Token advance() {

        if (!isAtEnd()) {

            current++;
        }


        return previous();
    }


    // ========================================================
    // END
    // ========================================================

    bool isAtEnd() {

        return (
            peek().type ==
            END_OF_FILE
        );
    }


    // ========================================================
    // PEEK
    // ========================================================

    Token peek() {

        return tokens[current];
    }


    // ========================================================
    // PREVIOUS
    // ========================================================

    Token previous() {

        return tokens[current - 1];
    }


    // ========================================================
    // CONSUME
    // ========================================================

    Token consume(
        TokenType type,
        const string& message
    ) {

        if (check(type)) {

            return advance();
        }


        throw error(
            peek(),
            message
        );
    }


    // ========================================================
    // ERROR
    // ========================================================

    ParseError error(
        Token token,
        const string& message
    ) {

        if (
            token.type ==
            END_OF_FILE
        ) {

            ::error(
                token.line,
                " at end " + message
            );
        }
        else {

            ::error(
                token.line,
                " at '" +
                token.lexeme +
                "' " +
                message
            );
        }


        return ParseError();
    }
};