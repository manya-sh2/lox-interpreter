#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include "token.h"

using namespace std;

// Forward declaration of error() defined in main.cpp.
void error(int line, const string& message);

//reads the source code character by character and build tokens
class Scanner {
private:
    string source;
    vector<Token> tokens;

    int start   = 0;
    int current = 0;
    int line    = 1;

    unordered_map<string, TokenType> keywords = {
        {"and",    AND},   {"class",  CLASS}, {"else",   ELSE},
        {"false",  FALSE}, {"for",    FOR},   {"fun",    FUN},
        {"if",     IF},    {"nil",    NIL},   {"or",     OR},
        {"print",  PRINT}, {"return", RETURN},{"super",  SUPER},
        {"this",   THIS},  {"true",   TRUE},  {"var",    VAR},
        {"while",  WHILE}
    };

public:
    Scanner(const string& source);
    vector<Token> scanTokens();

private:

    bool isAtEnd() { return current >= (int)source.length(); } //checks whether scannning finished
    char advance()  { return source[current++]; } //consumes current character and moves ahead

    char peek() {
        if (isAtEnd()) return '\0';
        return source[current];
    } //looks at current character without consuming it

    char peekNext() {
        if (current + 1 >= (int)source.length()) return '\0';
        return source[current + 1];
    } //looks at one character ahead

    bool match(char expected) {
        if (isAtEnd())                   return false;
        if (source[current] != expected) return false;
        current++;
        return true;
    } //checks whether current character matches expected

    void addToken(TokenType type) {
        string text = source.substr(start, current - start);
        tokens.push_back(Token(type, text, LiteralVal(), line));
    } //creates a token object and stores it in token vector, used for tokens that don't have any extra data

    void addToken(TokenType type, const string& raw, bool isNum) {
        string text = source.substr(start, current - start);
        tokens.push_back(Token(type, text, LiteralVal(raw, isNum), line));
    } //for token with extra data

    //scans one token
    void scanToken() {
        char c = advance();
        switch (c) {
            case '(': addToken(LEFT_PAREN);  break;
            case ')': addToken(RIGHT_PAREN); break;
            case '{': addToken(LEFT_BRACE);  break;
            case '}': addToken(RIGHT_BRACE); break;
            case ',': addToken(COMMA);       break;
            case '.': addToken(DOT);         break;
            case '-': addToken(MINUS);       break;
            case '+': addToken(PLUS);        break;
            case ';': addToken(SEMICOLON);   break;
            case '*': addToken(STAR);        break;

            case '!': addToken(match('=') ? BANG_EQUAL    : BANG);    break;
            case '=': addToken(match('=') ? EQUAL_EQUAL   : EQUAL);   break;
            case '<': addToken(match('=') ? LESS_EQUAL    : LESS);    break;
            case '>': addToken(match('=') ? GREATER_EQUAL : GREATER); break;

            case '/':
                if (match('/')) {
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    addToken(SLASH);
                }
                break;

            case ' ': case '\r': case '\t': break;
            case '\n': line++; break;

            case '"': stringLiteral(); break;

            default:
                if (isdigit(c))            numberLiteral();
                else if (isalpha(c) || c == '_') identifier();
                else error(line, "Unexpected character.");
                break;
        }
    }

    void stringLiteral() {
        while (peek() != '"' && !isAtEnd()) {
            if (peek() == '\n') line++;
            advance();
        }
        if (isAtEnd()) { error(line, "Unterminated string."); return; }
        advance(); // closing "
        string value = source.substr(start + 1, current - start - 2);
        addToken(STRING, value, false);
    }

    void numberLiteral() {
        while (isdigit(peek())) advance();
        if (peek() == '.' && isdigit(peekNext())) {
            advance();
            while (isdigit(peek())) advance();
        }
        // A number immediately followed by a letter or '_' is invalid (e.g. "2_", "3abc")
        if (isalpha(peek()) || peek() == '_') {
            while (isalnum(peek()) || peek() == '_') advance(); // consume the bad chars
            string bad = source.substr(start, current - start);
            error(line, "Invalid token '" + bad + "'.");
            return;
        }
        string text = source.substr(start, current - start);
        addToken(NUMBER, text, true);
    }

    void identifier() {
        while (isalnum(peek()) || peek() == '_') advance();
        string text = source.substr(start, current - start);
        auto it = keywords.find(text);
        if (it != keywords.end()) addToken(it->second);
        else                      addToken(IDENTIFIER);
    }
};