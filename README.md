# Lox Interpreter

A tree-walk interpreter for the Lox programming language, implemented from scratch in C++.

This project follows the interpreter architecture described in *Crafting Interpreters* and implements the complete pipeline from source code scanning to runtime execution.

## Features

- Lexical analysis / scanning
- Tokenization
- Recursive-descent parsing
- Abstract Syntax Tree (AST)
- Visitor Pattern for AST traversal
- Expression evaluation
- Variables and assignment
- Lexical scoping
- Nested environments
- Blocks and scope management
- `if / else` statements
- `while` loops
- `for` loops
- Logical `and` / `or` with short-circuit evaluation
- Functions and function calls
- Function parameters and arguments
- `return` statements
- First-class functions
- Closures
- Runtime error handling
- Parse error handling

## Architecture

The interpreter processes Lox source code through the following pipeline:

```text
Lox Source Code
       |
       v
    Scanner
       |
       v
     Tokens
       |
       v
     Parser
       |
       v
      AST
       |
       v
   Interpreter
       |
       v
  Environment
       |
       v
     Output