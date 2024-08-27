#pragma once

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"


class Interpreter;

struct Visitor
{
    virtual ~Visitor() {}
    virtual Expr *visit( Expr *node) = 0;
    
    virtual Expr *visit_empty_expression( Expr *node) = 0;
    virtual Expr *visit_binary( BinaryExpr *node) = 0;
    virtual Expr *visit_unary( UnaryExpr *node) = 0;
    virtual Expr *visit_logical( LogicalExpr *node) = 0;
    virtual Expr *visit_grouping( GroupingExpr *node) = 0;
    virtual Expr *visit_literal( Literal *node) = 0;
    virtual Expr *visit_number_literal( NumberLiteral *node) = 0;
    virtual Expr *visit_string_literal( StringLiteral *node) = 0;
    virtual Expr *visit_now_expression( NowExpr *node) = 0;
    virtual Expr *visit_variable(Variable *node) =0;
    virtual Expr *visit_assign(Assign *node) =0;
    

    virtual void execute(Stmt *stmt) = 0;
    virtual void visit_block_smt(BlockStmt *node) = 0;
    virtual void visit_expression_smt(ExpressionStmt *node) = 0;
    virtual void visit_print_smt(PrintStmt *node) = 0;
    virtual void visit_declaration(Declaration *node) = 0;
    virtual void visit_if(IFStmt *node) = 0;
    virtual void visit_while(WhileStmt *node) = 0;
    virtual void visit_program(Program *node) = 0;
    virtual void visit_for(ForStmt *node) = 0;

    


    
};

class Environment
{

private:

    Environment *parent;
    std::unordered_map<std::string, Expr*> m_values;
 

public:
    Environment( Environment *parent=nullptr);
    virtual ~Environment();


    bool    define (const std::string &name, Expr *value);
    Expr *get(const std::string &name);
    bool     set(const std::string &name, Expr *value);

    bool contains(const std::string &name);

    bool assign(const std::string &name, Expr *value);
    bool replace(const std::string &name, Expr *value);

    bool addInteger(const std::string &name, int value);
    bool addDouble(const std::string &name, double value);
    bool addString(const std::string &name, std::string value);
    bool addBoolean(const std::string &name, bool value);
};

struct  Compiler : public Visitor
{
       
        Expr *visit( Expr *node) override;
        Expr *visit_empty_expression( Expr *node) override;
        Expr *visit_binary( BinaryExpr *node) override;
        Expr *visit_unary( UnaryExpr *node) override;
        Expr *visit_logical( LogicalExpr *node) override;
        Expr *visit_grouping( GroupingExpr *node) override;
        Expr *visit_literal(Literal *node) override;
        Expr *visit_number_literal( NumberLiteral *node) override;
        Expr *visit_string_literal(StringLiteral *node) override;
        Expr *visit_now_expression( NowExpr *node) override;
        Expr *visit_variable(Variable *node) override;
        Expr *visit_assign(Assign *node) override;
        Expr *evaluate(Expr *node);

        void execute(Stmt *stmt);

        void visit_block_smt(BlockStmt *node)override; 
        void visit_expression_smt(ExpressionStmt *node)override;
        void visit_print_smt(PrintStmt *node)override;
        void visit_declaration(Declaration *node)override;
        void visit_if(IFStmt  *node)override;
        void visit_while(WhileStmt *node)override;
        void visit_for(ForStmt *node)override;
        void visit_program(Program *node)override;

       

        Compiler(Interpreter *i, Compiler *parent = nullptr);
        ~Compiler();

        void begin_scope();
        void end_scope();

        Environment *top();

    private:
        friend class Interpreter;
        Interpreter *interpreter;
        Environment *main;
        
        Compiler *parent;
        std::stack<Environment*> stack;
};



class Interpreter
{

public:
        ~Interpreter() ;
        Interpreter();

        bool compile(const std::string &source);

        void clear();

    private:
    friend class Compiler;

    Compiler *compiler;

    void Error(const Token &token,const std::string &message);
    void Error(const std::string &message);
    void Warning(const Token &token, const std::string &message);
    void Warning(const std::string &message);
    void Info(const std::string &message);
};

