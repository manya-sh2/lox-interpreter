#pragma once

#include "token.h"
#include <string>
#include <vector>

using namespace std;

class Interpreter;
class LoxCallable;

// VALUE
struct Value {
    enum Type {
        NIL,
        BOOLEAN,
        NUMBER,
        STRING,
        CALLABLE
    };
    Type type;
    bool boolean;
    double number;
    string str;
    LoxCallable* callable;

    // nil
    Value()
        : type(NIL),
          boolean(false),
          number(0.0),
          str(""),
          callable(nullptr) {}

    // bool
    Value(bool b)
        : type(BOOLEAN),
          boolean(b),
          number(0.0),
          str(""),
          callable(nullptr) {}

    // number
    Value(double n)
        : type(NUMBER),
          boolean(false),
          number(n),
          str(""),
          callable(nullptr) {}

    // string
    Value(const string& s)
        : type(STRING),
          boolean(false),
          number(0.0),
          str(s),
          callable(nullptr) {}

    // callable
    Value(LoxCallable* c)
        : type(CALLABLE),
          boolean(false),
          number(0.0),
          str(""),
          callable(c) {}
    bool isNil() const {
        return type == NIL;
    }

    bool isBool() const {
        return type == BOOLEAN;
    }

    bool isNumber() const {
        return type == NUMBER;
    }

    bool isString() const {
        return type == STRING;
    }

    bool isCallable() const {
        return type == CALLABLE;
    }
};

// CALLABLE INTERFACE
class LoxCallable {
public:
    virtual ~LoxCallable() {}

    virtual int arity() = 0;
    virtual Value call(Interpreter* interpreter, const vector<Value>& arguments) = 0;
    virtual string toString() = 0;
};

// FORWARD DECLARATIONS
class Binary;
class Unary;
class Grouping;
class Literal;
class Variable;
class Assign;
class Logical;
class Call;

// STRING VISITOR
class StringVisitor {
public:
    virtual ~StringVisitor() {}

    virtual string visitBinaryExpr(Binary* expr) = 0;
    virtual string visitUnaryExpr(Unary* expr) = 0;
    virtual string visitGroupingExpr(Grouping* expr) = 0;
    virtual string visitLiteralExpr(Literal* expr) = 0;
    virtual string visitVariableExpr(Variable* expr) = 0;
    virtual string visitAssignExpr(Assign* expr) = 0;
    virtual string visitLogicalExpr(Logical* expr) = 0;
    virtual string visitCallExpr(Call* expr) = 0;
};

// VALUE VISITOR
class ValueVisitor {
public:
    virtual ~ValueVisitor() {}

    virtual Value visitBinaryExpr(Binary* expr) = 0;
    virtual Value visitUnaryExpr(Unary* expr) = 0;
    virtual Value visitGroupingExpr(Grouping* expr) = 0;
    virtual Value visitLiteralExpr(Literal* expr) = 0;
    virtual Value visitVariableExpr(Variable* expr) = 0;
    virtual Value visitAssignExpr(Assign* expr) = 0;
    virtual Value visitLogicalExpr(Logical* expr) = 0;
    virtual Value visitCallExpr(Call* expr) = 0;
};

// BASE EXPRESSION
class Expr {
public:
    virtual ~Expr() {}

    virtual string accept(StringVisitor* visitor) = 0;
    virtual Value accept(ValueVisitor* visitor) = 0;
};

// BINARY
class Binary : public Expr {
public:
    Expr* left;
    Token op;
    Expr* right;
    Binary(Expr* left, Token op, Expr* right)
        : left(left),
          op(op),
          right(right) {}
    string accept(StringVisitor* visitor) override {
        return visitor->visitBinaryExpr(this);
    }

    Value accept(ValueVisitor* visitor) override {
        return visitor->visitBinaryExpr(this);
    }
};

// UNARY
class Unary : public Expr {
public:
    Token op;
    Expr* right;
    Unary(Token op, Expr* right)
        : op(op),
          right(right) {}
    string accept(StringVisitor* visitor) override {
        return visitor->visitUnaryExpr(this);
    }

    Value accept(ValueVisitor* visitor) override {
        return visitor->visitUnaryExpr(this);
    }
};

// GROUPING
class Grouping : public Expr {
public:
    Expr* expression;
    Grouping(Expr* expression) : expression(expression) {}
    string accept(StringVisitor* visitor) override {
        return visitor->visitGroupingExpr(this);
    }

    Value accept(ValueVisitor* visitor) override {
        return visitor->visitGroupingExpr(this);
    }
};

// LITERAL
class Literal : public Expr {
public:
    Value value;
    Literal(Value value) : value(value) {}
    string accept(StringVisitor* visitor) override {
        return visitor->visitLiteralExpr(this);
    }

    Value accept(ValueVisitor* visitor) override {
        return visitor->visitLiteralExpr(this);
    }
};

// VARIABLE
class Variable : public Expr {
public:
    Token name;
    Variable(Token name) : name(name) {}
    string accept(StringVisitor* visitor) override {
        return visitor->visitVariableExpr(this);
    }

    Value accept(ValueVisitor* visitor) override {
        return visitor->visitVariableExpr(this);
    }
};

// ASSIGN
class Assign : public Expr {
public:
    Token name;
    Expr* value;
    Assign(Token name, Expr* value)
        : name(name),
          value(value) {}
    string accept(StringVisitor* visitor) override {
        return visitor->visitAssignExpr(this);
    }

    Value accept(ValueVisitor* visitor) override {
        return visitor->visitAssignExpr(this);
    }
};

// LOGICAL
class Logical : public Expr {
public:
    Expr* left;
    Token op;
    Expr* right;
    Logical(Expr* left, Token op, Expr* right)
        : left(left),
          op(op),
          right(right) {}
    string accept(StringVisitor* visitor) override {
        return visitor->visitLogicalExpr(this);
    }

    Value accept(ValueVisitor* visitor) override {
        return visitor->visitLogicalExpr(this);
    }
};

// CALL
class Call : public Expr {
public:
    Expr* callee;
    Token paren;
    vector<Expr*> arguments;
    Call(Expr* callee, Token paren, vector<Expr*> arguments)
        : callee(callee),
          paren(paren),
          arguments(arguments) {}
    string accept(StringVisitor* visitor) override {
        return visitor->visitCallExpr(this);
    }

    Value accept(ValueVisitor* visitor) override {
        return visitor->visitCallExpr(this);
    }
};
