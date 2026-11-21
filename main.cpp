#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "scanner.h"
#include "parser.h"
#include "interpreter.h"
#include "ast_printer.h"
using namespace std;

static bool hadError        = false;
static bool hadRuntimeError = false;
static AstStorage ast;
static Interpreter interpreter;

void report(int line, const string& where, const string& message) {
    cerr << "[line " << line << "] Error" << where << ": " << message << endl;
    hadError = true;
}

void error(int line, const string& message) {
    report(line, "", message);
}

void runtimeError(const RuntimeError& error) {
    cerr << "[line " << error.token.line << "] Runtime Error: " << error.what() << endl;
    hadRuntimeError = true;
}

void run(const string& source) {
    // Step 1: Scan
    Scanner scanner(source);
    vector<Token> tokens = scanner.scanTokens();
    if (hadError) return;

    // Step 2: Parse
    Parser parser(tokens, ast);
    vector<Stmt*> statements = parser.parse();
    if (hadError) return;          // ← removed statements.empty() check
                                   //   empty input is valid, not an error

    // Step 3: Interpret
    interpreter.interpret(statements);
}

void runFile(const string& path) {
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Could not open file: " << path << endl;
        exit(74);
    }
    stringstream buffer;
    buffer << file.rdbuf();
    run(buffer.str());
    if (hadError)        exit(65);
    if (hadRuntimeError) exit(70);
}

void runPrompt() {
    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line)) break;
        run(line);
        hadError        = false;   // reset after each REPL line
        hadRuntimeError = false;   // ← added: runtime errors shouldn't
                                   //   permanently break the REPL either
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        cerr << "Usage: lox [script]" << endl;
        return 64;
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        runPrompt();
    }
    return 0;
}
