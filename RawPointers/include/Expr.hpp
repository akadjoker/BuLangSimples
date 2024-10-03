#pragma once 
#include "Token.hpp"
#include "Utils.hpp"

struct Visitor;

enum ExprType
{
    E_NONE,
    EMPTY_EXPR,
    BINARY,
    UNARY,
    GROUPING,
    LITERAL,
    L_NUMBER,
    L_STRING,
    L_FUNCTION,
    L_NATIVE,
    L_CLASS,
    L_STRUCT,
    L_ARRAY,
    L_MAP,
    GET,
    GET_DEF,
    SET,
    VARIABLE,
    ASSIGN,
    LOGICAL,
    CALL,
    NOW,
    E_COUNT
};

class Expr
{
public:
    Expr()  {}
    virtual ~Expr() 
    {
        INFO("Expr destroyed %s", toString().c_str());
    }

   

    virtual Expr *accept( Visitor &v) = 0;

    ExprType type{ExprType::E_NONE};

    std::string toString();

    virtual std::size_t hash()  const { return 0; }

    virtual void print() {};
    virtual Expr* clone() { return nullptr; }
};

class EmptyExpr : public Expr
{
public:
    EmptyExpr() : Expr() { type = ExprType::EMPTY_EXPR; }

    Expr *accept( Visitor &v) override;
};


class BinaryExpr : public Expr
{
public:
    BinaryExpr() : Expr() { type = ExprType::BINARY; }

    Expr *accept( Visitor &v) override;

    Expr *left;
    Expr *right;
    Token op;
};


class UnaryExpr : public Expr
{
public:
    UnaryExpr() : Expr() { type = ExprType::UNARY; }

    Expr *accept( Visitor &v) override;

    Expr *right;
    Token op;
    bool isPrefix;
};

class GroupingExpr : public Expr
{
public:
    GroupingExpr() : Expr() { type = ExprType::GROUPING; }

    Expr *accept( Visitor &v) override;

    Expr *expr;
};

class LogicalExpr : public Expr
{
public:
    LogicalExpr() : Expr() { type = ExprType::LOGICAL; }

    Expr *accept( Visitor &v) override;

    Expr *left;
    Expr *right;
    Token op;
};



class Literal : public Expr
{
public:
    Literal() : Expr() { type = ExprType::LITERAL; }

    Expr *accept( Visitor &v) override;
    virtual ~Literal() 
    {
        INFO("Expr destroyed Literal");
    }

    
    virtual void print() override;
};

class NumberLiteral : public Literal
{
public:
    NumberLiteral() : Literal() { type = ExprType::L_NUMBER; }

    ~NumberLiteral() 
    {
        INFO("Expr destroyed NumberLiteral %f", value);
    }

    bool operator==(const NumberLiteral& outra) const 
    {
        return value == outra.value;
    }

    std::size_t hash() const override 
    {
        return std::hash<double>()(value);
    }

    Expr *accept( Visitor &v) override;

    void print() override;

    Expr *clone() override;

    double value;
};




class StringLiteral : public Literal
{
public:
    StringLiteral() : Literal() { type = ExprType::L_STRING; }

    ~StringLiteral() 
    {
        INFO("Expr destroyed StringLiteral %s", value.c_str());
    }
 
    Expr *accept( Visitor &v) override;

    void print() override;

    Expr *clone() override;

    bool operator==(const StringLiteral& outra) const 
    {
        return value == outra.value;
    }
    std::size_t hash() const override 
    {
        return std::hash<std::string>()(value);
    }

    std::string value;
};

class NowExpr : public Expr
{
public:
    NowExpr() : Expr() { type = ExprType::NOW; }
    Expr *accept( Visitor &v) override;

};




class Variable : public Expr
{
public:
    Variable() : Expr() { type = ExprType::VARIABLE; }
    Expr *accept( Visitor &v) override;

    Token name;
};

class Assign : public Expr
{
public:
    Assign() : Expr() { type = ExprType::ASSIGN; }
    Expr *accept( Visitor &v) override;

    Token name;   
    Expr *value;
};

class CallExpr : public Expr
{
public:
    CallExpr() : Expr() { type = ExprType::CALL; }
    Expr *accept( Visitor &v) override;
    Token name;
    Expr *callee;
    std::vector<Expr*> args;

};


class GetExpr : public Expr
{
public:
    GetExpr() : Expr() { type = ExprType::GET; }
    Expr *accept( Visitor &v) override;
    Token name;
    Expr *object;

};

class GetDefinitionExpr : public Expr
{
public:
    GetDefinitionExpr() : Expr() { type = ExprType::GET_DEF; }
    Expr *accept( Visitor &v) override;
    Token name;
    Expr *variable;
    std::vector<Expr*> values;
};


class SetExpr : public Expr
{
public:
    SetExpr() : Expr() { type = ExprType::SET; }
    Expr *accept( Visitor &v) override;
    Token name;
    Expr *object;
    Expr *value;
};