#pragma once

#include "Arena.hpp"
#include "Utils.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"

class Environment;


void *Malloc(u32 size,const char* file, u32 line);
void  Free(void* p,u32 size);

#define ARENA_ALLOC(size)    Malloc(size, __FILE__, __LINE__)
#define ARENA_FREE(mem, size) Free(mem,size)


class Factory
{
    private:

        BlockArena arena;
        StackArena stack;
        friend void *Malloc(u32 size,const char* file, u32 line);
        friend void Free(void* p,u32 size);


        Factory();
        ~Factory();
    public:
        static Factory &as();
        void clear();
        u32 size() const { return arena.size(); }


        //
        // Expr
        //

        BinaryExpr *make_binary();
        void free_binary(BinaryExpr *expr);

        UnaryExpr *make_unary();
        void free_unary(UnaryExpr *expr);

        LogicalExpr *make_logical();
        void free_logical(LogicalExpr *expr);

        GroupingExpr *make_grouping();
        void free_grouping(GroupingExpr *expr);

        NumberLiteral *make_number();
        void free_number(NumberLiteral *expr);

        StringLiteral *make_string();
        void free_string(StringLiteral *expr);

        Literal *make_literal();
        void free_literal(Literal *expr);


        NowExpr *make_now();
        void free_now(NowExpr *expr);


        //
        // Stmt
        //


        PrintStmt *make_print();
        void free_print(PrintStmt *expr);

        ExpressionStmt *make_expression();
        void free_expression(ExpressionStmt *expr);

        BlockStmt *make_block();
        void free_block(BlockStmt *expr);

        Program *make_program();
        void free_program(Program *expr);


        void delete_statement(Stmt *expr);
        void delete_expression(Expr *expr);


        Declaration *make_declaration();
        void free_declaration(Declaration *expr);

        Variable *make_variable();
        void free_variable(Variable *expr);

        Assign *make_assign();
        void free_assign(Assign *expr);

        IFStmt *make_if();
        void free_if(IFStmt *expr);

        WhileStmt *make_while();
        void free_while(WhileStmt *expr);

        ForStmt *make_for();
        void free_for(ForStmt *expr);

        Environment *make_environment(Environment *parent);
        void free_environment(Environment *expr);


};