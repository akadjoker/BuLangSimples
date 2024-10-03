
#include "pch.h" 
#include "Expr.hpp" 
#include "Interpreter.hpp"
#include "Utils.hpp"
#include "Factory.hpp"

Expr *EmptyExpr::accept(Visitor &v)
{
    return v.visit_empty_expression(this);
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

void NumberLiteral::print()
{
    PRINT("%f", value);
}

Expr *NumberLiteral::clone()
{
    NumberLiteral *expr = Factory::as().make_number();
    expr->value = value;
    return expr;
}

Expr *StringLiteral::accept(Visitor &v)
{
    return  v.visit_string_literal(this);
}

void StringLiteral::print()
{
    PRINT("%s", value.c_str());
}

Expr *StringLiteral::clone()
{
    StringLiteral *expr = Factory::as().make_string();
    expr->value = value;
    return expr;
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

void Literal::print()
{
    PRINT("nil");
}

std::string Expr::toString()
{
   switch (type)
   {
       case ExprType::E_NONE: return "NONE";
       case ExprType::L_NUMBER: return "NUMBER";
       case ExprType::L_STRING: return "STRING";
       case ExprType::L_FUNCTION: return "FUNCTION";
       case ExprType::L_CLASS: return "CLASS";
       case ExprType::L_STRUCT: return "STRUCT";
       case ExprType::L_ARRAY: return "ARRAY";
       case ExprType::L_MAP: return "MAP";
       case ExprType::L_NATIVE: return "NATIVE";
       case ExprType::LITERAL: return "LITERAL";
       case ExprType::BINARY: return "BINARY";
       case ExprType::UNARY: return "UNARY";
       case ExprType::GROUPING: return "GROUPING";
       case ExprType::LOGICAL: return "LOGICAL";
       case ExprType::NOW: return "NOW";
       case ExprType::VARIABLE: return "VARIABLE";
       case ExprType::EMPTY_EXPR: return "EMPTY_EXPR";
       case ExprType::ASSIGN: return "ASSIGN";
       case ExprType::CALL: return "CALL";
       case ExprType::GET: return "GET";
       case ExprType::SET: return "SET";
       case ExprType::GET_DEF: return "GET_DEF";
       
       default: return "UNKNOWN";
       
   }

   return "UNKNOWN";
}

Expr *CallExpr::accept(Visitor &v)
{
    return v.visit_call(this);
}

Expr *GetExpr::accept(Visitor &v)
{
    return v.visit_get(this);
}

Expr *SetExpr::accept(Visitor &v)
{
    return v.visit_set(this);
}

Expr *GetDefinitionExpr::accept(Visitor &v)
{
    return v.visit_get_definition(this);
}
