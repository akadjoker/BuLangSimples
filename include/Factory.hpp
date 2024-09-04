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

struct Function;
struct ClassLiteral;
struct StructLiteral;
struct ArrayLiteral;
struct MapLiteral;
struct Native;

class Factory
{
    private:

        BlockArena arena;
        StackArena stack;
        friend void *Malloc(u32 size,const char* file, u32 line);
        friend void Free(void* p,u32 size);

        std::vector<Expr> exprs;
        std::vector<Stmt> stmts;
        
        Factory();
        ~Factory();
    public:
        static Factory &as();
        void clear();
        u32 size() const { return arena.size(); }


        //
        // Expr
        //

        EmptyExpr *make_empty();
        void free_empty(EmptyExpr *expr);

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

        ArrayLiteral *create_array();
        void delete_array(ArrayLiteral *node);

        MapLiteral *create_map();
        void delete_map(MapLiteral *node);

        Function *createFunction();
        void deleteFunction(Function *node);

        StructLiteral *createStruct();
        void deleteStruct(StructLiteral *node);

        ClassLiteral *createClass();
        void deleteClass(ClassLiteral *node);

        Native *createNative();
        void deleteNative(Native *node);



        GetExpr *make_get();
        void free_get(GetExpr *expr);

        SetExpr *make_set();
        void free_set(SetExpr *expr);


        GetDefinitionExpr *make_get_definition();
        void free_get_definition(GetDefinitionExpr *expr);


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

        FunctionStmt *make_function();
        void free_function(FunctionStmt *expr);





        CallExpr *make_call();
        void free_call(CallExpr *expr);



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

        ElifStmt *make_elif();
        void free_elif(ElifStmt *expr);

        WhileStmt *make_while();
        void free_while(WhileStmt *expr);

        DoStmt *make_do();
        void free_do(DoStmt *expr);

        ForStmt *make_for();
        void free_for(ForStmt *expr);

        FromStmt *make_from();
        void free_from(FromStmt *expr);


        ReturnStmt *make_return();
        void free_return(ReturnStmt *expr);


        BreakStmt *make_break();
        void free_break(BreakStmt *expr);


        ContinueStmt *make_continue();
        void free_continue(ContinueStmt *expr);

        CaseStmt *make_case();
        void free_case(CaseStmt *expr);

        SwitchStmt *make_switch();
        void free_switch(SwitchStmt *expr);


        StructStmt *make_struct();
        void free_struct(StructStmt *expr);

        ClassStmt *make_class();
        void free_class(ClassStmt *expr);

        ArrayStmt *make_array();
        void free_array(ArrayStmt *expr);

        MapStmt *make_map();
        void free_map(MapStmt *expr);


    

        Environment *make_environment(Environment *parent);
        void free_environment(Environment *expr);


};