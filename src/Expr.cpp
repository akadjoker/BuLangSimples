
#include "pch.h" 
#include "Expr.hpp" 
#include "Interpreter.hpp"
#include "Utils.hpp"

Expr *EmptyExpr::accept(Visitor &v)
{
    return v.visit(this);
}



Expr *BinaryExpr::accept(Visitor &v)
{
    return v.visit_binary(this);
}



Expr *UnaryExpr::accept(Visitor &v)
{
    return v.visit_unary(this);
}



Expr *GroupingExpr::accept(Visitor &v)
{
    return v.visit_grouping(this);
}



Expr *LogicalExpr::accept(Visitor &v)
{
    return v.visit_logical(this);
}



Expr *NumberLiteral::accept(Visitor &v)
{
    return v.visit_number_literal(this);
}





Expr *StringLiteral::accept(Visitor &v)
{
    return  v.visit_string_literal(this);
}

Expr *NowExpr::accept(Visitor &v)
{
    return v.visit_now_expression(this);
}

Expr *Variable::accept(Visitor &v)
{
    return  v.visit_variable(this);
}

Expr *Assign::accept(Visitor &v)
{
    return  v.visit_assign(this);
}

Expr *Literal::accept(Visitor &v)
{
    return  v.visit_literal(this);
}

std::string Expr::toString()
{
   switch (type)
   {
       case ExprType::E_NONE: return "NONE";
       case ExprType::L_NUMBER: return "NUMBER";
       case ExprType::L_STRING: return "STRING";
       case ExprType::LITERAL: return "LITERAL";
       case ExprType::BINARY: return "BINARY";
       case ExprType::UNARY: return "UNARY";
       case ExprType::GROUPING: return "GROUPING";
       case ExprType::LOGICAL: return "LOGICAL";
       case ExprType::NOW: return "NOW";
       case ExprType::VARIABLE: return "VARIABLE";
       case ExprType::EMPTY_EXPR: return "EMPTY_EXPR";
       case ExprType::ASSIGN: return "ASSIGN";
       
   }

   return "UNKNOWN";
}
