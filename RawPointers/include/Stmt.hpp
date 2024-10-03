#pragma once 
#include <vector>

#include "Config.hpp"
#include "Token.hpp"
#include "Utils.hpp"

struct Visitor;
struct Expr;
class Environment;

enum StmtType
{
    S_NONE = 0,
    BLOCK,
    EXPRESSION,
    DECLARATION,
    IF,
    WHILE,
    FOR,
    FROM,
    DO,
    SWITCH,
    RETURN,
    BREAK,
    CONTINUE,
    PRINT,
    FUNCTION,
    STRUCT,
    CLASS,
    ARRAY,
    MAP,
    PROGRAM,
    S_COUNT,
};

class Stmt
{
public:
    Stmt()  {}
    virtual ~Stmt() {
        INFO("Stmt destroyed %s", toString().c_str());
    }
    virtual u8 visit( Visitor &v) = 0;

    StmtType type{StmtType::S_NONE};

    std::string toString();
};


class BlockStmt : public Stmt
{
public:
    BlockStmt() : Stmt() { type = StmtType::BLOCK; }



    u8 visit( Visitor &v) override;

    std::vector<Stmt *> statements;
};


class ExpressionStmt : public Stmt
{
public:
    ExpressionStmt() : Stmt() { type = StmtType::EXPRESSION; }

    u8 visit( Visitor &v) override;

    Expr *expression;
};


struct ElifStmt
{
    Expr *condition;
    Stmt *then_branch;
};

class IFStmt : public Stmt
{
public:
    IFStmt();

    u8 visit( Visitor &v) override;

    Expr *condition;
    Stmt *then_branch;
    Stmt *else_branch;

    std::vector<ElifStmt*> elifBranch;

};

struct CaseStmt 
{
    Expr *condition;
    Stmt *body;
};

class SwitchStmt : public Stmt
{
public:
    SwitchStmt() : Stmt() { type = StmtType::SWITCH; }

    u8 visit( Visitor &v) override;

    Expr *condition;
    std::vector<CaseStmt*> cases;
    Stmt *defaultBranch;
};

class WhileStmt : public Stmt
{
public:
    WhileStmt() : Stmt() { type = StmtType::WHILE; }

    u8 visit( Visitor &v) override;

    Expr *condition;
    Stmt *body;

};

class DoStmt : public Stmt
{
public:
    DoStmt() : Stmt() { type = StmtType::DO; }

    u8 visit( Visitor &v) override;

    Expr *condition;
    Stmt *body;
};


class ForStmt : public Stmt
{
public:
    ForStmt() : Stmt() { type = StmtType::FOR; }

    u8 visit( Visitor &v) override;

    Stmt *initializer;
    Expr *condition;
    Expr *increment;
    Stmt *body;

};


class FromStmt : public Stmt
{
public:
    FromStmt() : Stmt() { type = StmtType::FROM; }
    u8 visit( Visitor &v) override;
    Expr *variable;
    Expr *array;
    Stmt *body;
};


class PrintStmt : public Stmt
{
public:
    PrintStmt() : Stmt() { type = StmtType::PRINT; }
    u8 visit( Visitor &v) override;

    Expr *expression;
};

class Declaration : public Stmt
{
public:
    Declaration() : Stmt() { type = StmtType::DECLARATION; }
    u8 visit( Visitor &v) override;
    std::vector<Token> names;
    bool is_initialized = false;
    Expr *initializer;
};

class ReturnStmt : public Stmt
{
public:
    ReturnStmt() : Stmt() { type = StmtType::RETURN; }
    u8 visit( Visitor &v) override;

    Expr *value;

};


class BreakStmt : public Stmt
{
public:
    BreakStmt() : Stmt() { type = StmtType::BREAK; }
    u8 visit( Visitor &v) override;
};


class ContinueStmt : public Stmt
{
public:
    ContinueStmt() : Stmt() { type = StmtType::CONTINUE; }
    u8 visit( Visitor &v) override;
};


class FunctionStmt : public Stmt
{
public:
    FunctionStmt() : Stmt() { type = StmtType::FUNCTION; }
    u8 visit( Visitor &v) override;
    std::vector<std::string> args;
    Token name;
    Stmt *body;

};

class StructStmt : public Stmt
{
public:
    StructStmt() : Stmt() { type = StmtType::STRUCT; }
    u8 visit( Visitor &v) override;
    
    std::vector<Token> fields;
    std::vector<Expr*> values;
    std::vector<Expr*> args;
    
    
    Token name;
};

class ClassStmt : public Stmt
{
public:
    ClassStmt() : Stmt() { type = StmtType::CLASS; }
    u8 visit( Visitor &v) override;
    std::vector<Expr*> fields;
    std::vector<FunctionStmt*> methods;
    std::unordered_map<std::string, FunctionStmt*> methodMap;
    std::unordered_map<std::string, Expr*> fieldMap;
    Token name;
};


class ArrayStmt : public Stmt
{
public:
    ArrayStmt() : Stmt() { type = StmtType::ARRAY; }
    u8 visit( Visitor &v) override;
    std::vector<Expr*> values;
    Token name;
};

class MapStmt : public Stmt
{
public:
    MapStmt() : Stmt() { type = StmtType::MAP; }
    u8 visit( Visitor &v) override;
    std::unordered_map<Expr*, Expr*> values;
        Token name;
};



class Program : public Stmt
{
public:
    Program() : Stmt() { type = StmtType::PROGRAM; }

    u8 visit( Visitor &v) override;

    std::vector<Stmt *> statements;
};