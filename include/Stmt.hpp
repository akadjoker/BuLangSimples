#pragma once 
#include <vector>
#include "Token.hpp"


struct Visitor;
struct Expr;

enum StmtType
{
    S_NONE = 0,
    BLOCK,
    EXPRESSION,
    DECLARATION,
    IF,
    WHILE,
    FOR,
    DO,
    PRINT,
    PROGRAM,
    S_COUNT,
};

class Stmt
{
public:
    Stmt()  {}
    virtual ~Stmt() {}
    virtual void visit( Visitor &v) = 0;

    StmtType type{StmtType::S_NONE};
};


class BlockStmt : public Stmt
{
public:
    BlockStmt() : Stmt() { type = StmtType::BLOCK; }


    void visit( Visitor &v) override;

    std::vector<Stmt *> statements;
};


class ExpressionStmt : public Stmt
{
public:
    ExpressionStmt() : Stmt() { type = StmtType::EXPRESSION; }

    void visit( Visitor &v) override;

    Expr *expression;
};


class IFStmt : public Stmt
{
public:
    IFStmt();

    void visit( Visitor &v) override;

    Expr *condition;
    Stmt *then_branch;
    Stmt *else_branch;

};


class WhileStmt : public Stmt
{
public:
    WhileStmt() : Stmt() { type = StmtType::WHILE; }

    void visit( Visitor &v) override;

    Expr *condition;
    Stmt *body;

};


class ForStmt : public Stmt
{
public:
    ForStmt() : Stmt() { type = StmtType::FOR; }

    void visit( Visitor &v) override;

    Stmt *initializer;
    Expr *condition;
    Expr *increment;
    Stmt *body;
};

class PrintStmt : public Stmt
{
public:
    PrintStmt() : Stmt() { type = StmtType::PRINT; }
    void visit( Visitor &v) override;

    Expr *expression;
};

class Declaration : public Stmt
{
public:
    Declaration() : Stmt() { type = StmtType::DECLARATION; }
    void visit( Visitor &v) override;

    Token name;
    bool is_initialized = false;
    Expr *initializer;
};

class Program : public Stmt
{
public:
    Program() : Stmt() { type = StmtType::PROGRAM; }

    void visit( Visitor &v) override;

    std::vector<Stmt *> statements;
};