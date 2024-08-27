
#include "pch.h" 
#include "Stmt.hpp" 
#include "Expr.hpp"
#include "Interpreter.hpp"

void BlockStmt::visit(Visitor &v)
{
    v.visit_block_smt(this);
}

void ExpressionStmt::visit(Visitor &v)
{
    v.visit_expression_smt(this);
}

void PrintStmt::visit(Visitor &v)
{
    v.visit_print_smt(this);
}

void Program::visit(Visitor &v)
{
    v.visit_program(this);
}

void Declaration::visit(Visitor &v)
{
    v.visit_declaration(this);
}

IFStmt::IFStmt():Stmt(), condition(nullptr), then_branch(nullptr), else_branch(nullptr)
{
    type = StmtType::IF;
}

void IFStmt::visit(Visitor &v)
{
    v.visit_if(this);
}

void WhileStmt::visit(Visitor &v)
{
    v.visit_while(this);
}

void ForStmt::visit(Visitor &v)
{
    v.visit_for(this);
}
