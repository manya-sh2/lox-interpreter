#pragma once
#include <string>
using namespace std;

//defines all possible token categories
enum TokenType {
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    // One or two character tokens
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals
    IDENTIFIER, STRING, NUMBER,

    // Keywords
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,
    END_OF_FILE
    // Special token added when source code finishes.
    // Helps parser know input is over.
};

// Stores the literal value of a token (used for NUMBER and STRING tokens).
// For NUMBER tokens, raw is the numeric string (e.g. "3.14").
// For STRING tokens, raw is the string content (without quotes).
// For all other tokens, raw is empty.
struct LiteralVal {
    enum class Kind { NONE, NUMBER, STRING } kind;
    string raw;  // raw text; Parser converts NUMBER to double via stod()
    LiteralVal()                     : kind(Kind::NONE)   {}
    LiteralVal(const string& s, bool isNum) : kind(isNum ? Kind::NUMBER : Kind::STRING), raw(s) {}
};

//represents one complete token
struct Token {
    TokenType  type;
    string     lexeme;
    LiteralVal literal;
    int        line;

    //creates a token
    Token(TokenType type, const string& lexeme, LiteralVal literal, int line)
        : type(type), lexeme(lexeme), literal(literal), line(line) {}

    //return token in text
    string toString() const { return lexeme; }
};
