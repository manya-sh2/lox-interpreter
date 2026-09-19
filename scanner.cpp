#include "scanner.h"

Scanner::Scanner(const string& source) {
    this->source = source;
}

vector<Token> Scanner::scanTokens() {
    while (!isAtEnd()) {
        // We are at the beginning of the next lexeme.
        start = current;
        scanToken();
    }

    tokens.push_back(Token(END_OF_FILE, "", LiteralVal(), line));
    return tokens;
}
