#include "pch.h"

#include "Interpreter.hpp"
#include "Utils.hpp"
#include "Factory.hpp"

Expr *Compiler::visit(Expr *node)
{
    if (!node) return nullptr;
    return node->accept(*this);
}

Expr *Compiler::visit_assign(Assign *node)
{
    if (!node) return nullptr;
    Expr *value = evaluate(node->value);
    if (value == nullptr)
    {
        throw FatalException("Invalid assignment target.");
    }
    Environment *environment = top();
    if (!environment->assign(node->name.lexeme, value))
    {
       throw FatalException("Undefined variable: '" + node->name.lexeme +"' at line  "+ std::to_string(node->name.line )+" or mixe types.");
    }
  //  Factory::as().free_assign(node);
    return value;
}

Expr *Compiler::evaluate(Expr *node)
{
    Expr * result = visit(node);
    if (result == nullptr)
    {
        throw FatalException("Evaluation error: Unknown expression type");
    }
    return result;
}

Expr *Compiler::visit_now_expression(NowExpr *node)
{
    Factory::as().free_now(node);
    NumberLiteral *result = Factory::as().make_number();
    result->value = time_now();
    return result;
}



void Compiler::execute(Stmt *stmt)
{
    if (stmt == nullptr)
    {
        return;
    }
    stmt->visit(*this);
}

void Compiler::visit_block_smt(BlockStmt *node)
{
    if (!node) return ;
    if (node->statements.size() == 0) return ;
    begin_scope();
    for (Stmt *s : node->statements)
    {
        execute(s);
    }
   // Factory::as().free_block(node);
    end_scope();
}

void Compiler::visit_expression_smt(ExpressionStmt *node)
{
    if (node->expression == nullptr)
    {
        throw FatalException("[EXPRESSION STATEMENT] Unknown expression type");
    }
    evaluate(node->expression);
  //  Factory::as().free_expression(node);
}

void Compiler::visit_print_smt(PrintStmt *node)
{
    if (!node) return ;

    //INFO("[PRINT] %s", node->expression->toString().c_str());
    bool isVar = node->expression->type == ExprType::VARIABLE;

    Expr *result = evaluate(node->expression);
    if (result == nullptr)
    {
        throw FatalException("[PRINT] Unknown expression type");
    }
    if (result->type == ExprType::L_NUMBER)
    {
        NumberLiteral *nl = static_cast<NumberLiteral *>(result);
        PRINT("%f", nl->value);
         
    }
    else if (result->type == ExprType::L_STRING)
    {
        StringLiteral *sl = static_cast<StringLiteral *>(result);
        PRINT("%s", sl->value.c_str());
    } else if (result->type == ExprType::LITERAL)
    {
        PRINT("nil");
    }else 
    {
       WARNING("[PRINT] Unknown literal type");
    }
    if (!isVar)
       Factory::as().delete_expression(result);
  //  Factory::as().free_print(node);
}


Expr *Compiler::visit_variable(Variable *node)
{
   // INFO("READ Variable: %s", node->name.lexeme.c_str());
    Environment *environment = top();
    Expr *result = environment->get(node->name.lexeme);
    if (result == nullptr)
    {
        throw FatalException("Undefined variable: '" + node->name.lexeme +"' at line "+ std::to_string(node->name.line ));
    }
    if (result->type == ExprType::LITERAL)
    {
      WARNING("Variable: %s is not initialized", node->name.lexeme.c_str());
    }
    return result;
}

void Compiler::visit_declaration(Declaration *node)
{
    //INFO("Define variable: %s", node->name.lexeme.c_str());
    Environment *environment = top();
    // if (environment->contains(node->name.lexeme))
    // {
    //     throw FatalException("Variable already defined: " + node->name.lexeme +" at line "+ std::to_string(node->name.line ));
    // }

    if (node->is_initialized)
    {
        Expr *value = evaluate(node->initializer);
        environment->define(node->name.lexeme, value);
    } else 
    {
        Literal *value = Factory::as().make_literal();
        environment->define(node->name.lexeme, value);
    }


  
}

static bool is_truthy(Expr *expr)
{
    if (expr == nullptr)
    {
        return true;
    }
    if (expr->type == ExprType::L_NUMBER)
    {
        NumberLiteral *nl = static_cast<NumberLiteral *>(expr);
        return nl->value != 0;
    }
    if (expr->type == ExprType::L_STRING)
    {
        StringLiteral *sl = static_cast<StringLiteral *>(expr);
        return !sl->value.empty();
    }
    return true;
}

void Compiler::visit_if(IFStmt *node)
{
    Expr *condition = evaluate(node->condition);
    if (condition == nullptr)
    {
        throw FatalException("Invalid condition");
    }
    if (is_truthy(condition))
    {
        execute(node->then_branch);

        Factory::as().delete_statement(node->else_branch);

    } else if (node->else_branch != nullptr)
    {
        execute(node->else_branch);

        Factory::as().delete_statement(node->then_branch);
    }

    Factory::as().delete_expression(condition);
    Factory::as().free_if(node);
}

void Compiler::visit_while(WhileStmt *node)
{
}

void Compiler::visit_program(Program *node)
{
    for (Stmt *s : node->statements)
    {
        execute(s);
    }
    Factory::as().free_program(node);
}

void Compiler::visit_for(ForStmt *node)
{
  
    execute(node->initializer);

    while (true)
    {
      
            auto condition = evaluate(node->condition);
            if (!is_truthy(condition))
                break;
            Factory::as().delete_expression(condition);


        

        execute(node->body);
         
        evaluate(node->increment);
    }

    Factory::as().free_for(node);
    
}

Compiler::Compiler(Interpreter *i, Compiler *c)
{
    interpreter = i;
    parent = c;
    if (parent != nullptr)
        main = Factory::as().make_environment(parent->main);
    else
        main = Factory::as().make_environment(nullptr);
    
    stack.push(main);
    
}

Compiler::~Compiler()
{
    interpreter = nullptr;
    Factory::as().free_environment(main);
    main = nullptr;
}

void Compiler::begin_scope()
{
    Environment *last = stack.top();
    Environment *top = Factory::as().make_environment(last);
    stack.push(top);
}

void Compiler::end_scope()
{
    if (stack.size() <= 1) return;
    Environment *top = stack.top();
    stack.pop();
    Factory::as().free_environment(top);
}

Environment *Compiler::top()
{
    return stack.top();
}

Expr *Compiler::visit_empty_expression(Expr *node)
{
    interpreter->Error("Empty expression");
    Factory::as().delete_expression(node);
    return nullptr;
}

Expr *Compiler::visit_binary(BinaryExpr *node)
{
    Expr *left  = evaluate(node->left);
    Expr *right = evaluate(node->right);
  
    switch (node->op.type)
    {
        case TokenType::GREATER:
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value > r->value ? 1 : 0;
                return result;
            } 

            break;
        }

        case TokenType::GREATER_EQUAL:
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value >= r->value ? 1 : 0;
                return result;
            }

            break;
        }

        case TokenType::LESS:
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value < r->value ? 1 : 0;
                return result;
            }

            break;
        }

        case TokenType::LESS_EQUAL:
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value <= r->value ? 1 : 0;
                return result;
            }

            break;
        }
        case TokenType::PLUS:
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value + r->value;
                return result;
            } else if (left->type == ExprType::L_STRING && right->type == ExprType::L_STRING)
            {
                StringLiteral *l = static_cast<StringLiteral *>(left);
                StringLiteral *r = static_cast<StringLiteral *>(right);
                StringLiteral *result = Factory::as().make_string();
                result->value = l->value + r->value;
                return result;
            } else if (left->type == ExprType::L_STRING && right->type == ExprType::L_NUMBER)
            {
                StringLiteral *l = static_cast<StringLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                StringLiteral *result = Factory::as().make_string();
                result->value = l->value + std::to_string(r->value);

                return result;
            } else if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_STRING)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                StringLiteral *r = static_cast<StringLiteral *>(right);
                StringLiteral *result = Factory::as().make_string();
                result->value = std::to_string(l->value) + r->value;
                return result;
            }
            break;
        }
        case TokenType::MINUS:
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value - r->value;

                return result;
            }
            break;
        }
        case TokenType::SLASH:
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                if (r->value == 0)
                {

                    throw FatalException("Division by zero");
                }
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value / r->value;

                return result;
            }
            break;
        }
        case TokenType::STAR:
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();    
                result->value = l->value * r->value;

                return result;
            }
            break;
        }
        case TokenType::MOD:
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value =std::fmod(l->value, r->value);
   
                return result;
            }
        }
        case TokenType::BANG_EQUAL:
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value != r->value ? 1 : 0;
   
                return result;
            } else if (left->type == ExprType::L_STRING && right->type == ExprType::L_STRING)
            {
                StringLiteral *l = static_cast<StringLiteral *>(left);
                StringLiteral *r = static_cast<StringLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value != r->value ? 1 : 0;

                return result;
            }
            break;
        }

        case TokenType::EQUAL_EQUAL:
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value == r->value ? 1 : 0;

                return result;
            } else if (left->type == ExprType::L_STRING && right->type == ExprType::L_STRING)
            {
                StringLiteral *l = static_cast<StringLiteral *>(left);
                StringLiteral *r = static_cast<StringLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value == r->value ? 1 : 0;

                return result;
            }
            break;
        }
        case TokenType::PLUS_EQUAL://+=
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value += r->value;

                return result;
            }
            break;
        }
        case TokenType::MINUS_EQUAL:// -=
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                

                result->value = l->value -= r->value;

                return result;
            }
            break;
        }
        case TokenType::STAR_EQUAL:// *=
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value *= r->value;

                return result;
            }
            break;
        }
        case TokenType::SLASH_EQUAL:// /=
        {
            if (left->type == ExprType::L_NUMBER && right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *l = static_cast<NumberLiteral *>(left);
                NumberLiteral *r = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                result->value = l->value /= r->value;
                if (r->value == 0)
                {
                     throw FatalException("Division by zero");
                }

                return result;
            }
            break;
        }
    }

    interpreter->Error(node->op, "[BINARY] Unknown operator");

    return nullptr;
}

Expr *Compiler::visit_unary(UnaryExpr *expr)
{
    Expr *right = evaluate(expr->right);
    switch (expr->op.type)
    {

        case TokenType::MINUS:
        {
            if (right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *num = static_cast<NumberLiteral *>(right);   
                NumberLiteral *result = Factory::as().make_number();
                result->value = -num->value;

                return result;
            }
            break;
        }
        case TokenType::BANG:
        {
            if (right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *num = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();

 
  
                result->value = (num->value==0) ? 0 : 1;


                return result;
            }
            break;
        }
        case TokenType::INC:
        {
            if (right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *num = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();

                if (expr->isPrefix)
                {
                    result->value = ++num->value;
                }
                else
                {
                    result->value = num->value;
                    num->value++;
                }
          

                
                return result;
            }
            break;
        }
        case TokenType::DEC:
        {
            if (right->type == ExprType::L_NUMBER)
            {
                NumberLiteral *num = static_cast<NumberLiteral *>(right);
                NumberLiteral *result = Factory::as().make_number();
                if (expr->isPrefix)
                {
                    result->value = --num->value;
                }
                else
                {
                    result->value = num->value;
                    num->value--;
                }
                return result;
            }
            break;
        }
       
    
    }
  
    interpreter->Error(expr->op, "[UNARY] Unknown operator");
   
    return nullptr;
}

Expr *Compiler::visit_logical(LogicalExpr *node)
{
    Expr *left = evaluate(node->left);

    if (left->type == ExprType::L_NUMBER)
    {
        NumberLiteral *l = static_cast<NumberLiteral *>(left);
       
        if (node->op.type == TokenType::OR)
        {
            if (l->value != 0)
            {
                return left;
            }
        }else  if (node->op.type == TokenType::AND)
        {
            if (l->value == 0)
            {
                return left;
            }
        } else if (node->op.type == TokenType::XOR)
        {
            if (l->value != 0)
            {
                return left;
            }
        } 
    }
    return evaluate(node->right);
}

Expr *Compiler::visit_grouping(GroupingExpr *node)
{
    return evaluate(node->expr);
}

Expr *Compiler::visit_literal(Literal *node)
{
    
    NumberLiteral *result = Factory::as().make_number();
    result->value = 0;
    Factory::as().free_literal(node);
    return result;
}

Expr *Compiler::visit_number_literal(NumberLiteral *node)
{
   
    //NumberLiteral *result = Factory::as().make_number();
    //result->value = node->value;
    //return result;
    return node;
}

Expr *Compiler::visit_string_literal(StringLiteral *node)
{
   // INFO("String: %s", node->value.c_str());
  //  StringLiteral *result = Factory::as().make_string();
   // result->value = node->value;
  //  return result;
    return node;
}

Interpreter::~Interpreter()
{

    
}   

Interpreter::Interpreter()
{
    compiler = new Compiler(this);
   
}

bool Interpreter::compile(const std::string &source)
{
    Lexer lexer;
    lexer.initialize();
    lexer.Load(source);
    std::vector<Token> tokens =  lexer.GetTokens();
    if (tokens.size() == 0)
    {
        return false;
    }

    for (auto token : tokens)
    {
        INFO("Token: %s" ,token.toString().c_str());
    }

    Program *program=nullptr;

    try 
    {
        Parser parser;
        parser.Load(tokens);
        program =  parser.parse();
        if (program == nullptr)
        {
            return false;
        }
        compiler->execute(program);
        
    }
    catch (FatalException e)
    {
        Factory::as().free_program(program);
        Error(e.what());
        return false;
    }
   
    return false;
}

void Interpreter::clear()
{
    INFO("Interpreter destroyed");
    if (compiler != nullptr)
    {
        delete compiler;
        compiler = nullptr;
    }
   
    INFO("Factory destroyed");
    Factory::as().clear();
}

void Interpreter::Error(const Token &token, const std::string &message)
{
    int line = token.line;
    std::string text =message+ " at line: " +std::to_string(line);
    Log(2, text.c_str());
    throw FatalException(text);
}
void Interpreter::Error(const std::string &message)
{
    Log(2, message.c_str());
    throw FatalException(message);
}
void Interpreter::Warning(const Token &token,const std::string &message)
{
    int line = token.line;
    std::string text =message+ " at line: " +std::to_string(line);
    Log(1, text.c_str());
}

void Interpreter::Warning(const std::string &message)
{
   Log(1, message.c_str());
}

void Interpreter::Info(const std::string &message)
{
   Log(0, message.c_str());
}

//***************************************************************************************** */

Environment::Environment( Environment *parent)
{

    this->parent = parent;
}

Environment::~Environment()
{
    for (auto it = m_values.begin(); it != m_values.end(); it++)
    {
        Expr *l = it->second;
        if (l != nullptr)
        {
         //   INFO("Freeing %s", l->toString().c_str());
            Factory::as().delete_expression(l);
        }
    }
}

bool Environment::define(const std::string &name, Expr *value)
{
    if (m_values.find(name) != m_values.end())
    {
        return false;
    }
    m_values[name] = value;
    return true;
}

Expr *Environment::get(const std::string &name)
{
    if (m_values.find(name) != m_values.end())
    {
        return m_values[name];
    }
    if (parent != nullptr)
    {
        return parent->get(name);
    }
    return nullptr;
}

bool Environment::set(const std::string &name, Expr *value)
{
    if (m_values.find(name) != m_values.end())
    {
        m_values[name] = value;
        return true;
    }
    if (parent != nullptr)
    {
        return parent->set(name, value);
    }
    return false;
}

bool Environment::contains(const std::string &name)
{
    if (m_values.find(name) != m_values.end())
    {
        return true;
    }
    if (parent != nullptr)
    {
        return parent->contains(name);
    }
    return false;
}

bool Environment::assign(const std::string &name, Expr *value)
{
    Expr *expr = get(name);
    if (expr == nullptr || value == nullptr)
    {
        return false;
    }
    if (expr->type == ExprType::LITERAL)
    {
        return replace(name, value);
    }else 
    if (expr->type == ExprType::L_NUMBER && value->type == ExprType::L_NUMBER)
    {
        NumberLiteral *l = static_cast<NumberLiteral *>(expr);
        NumberLiteral *r = static_cast<NumberLiteral *>(value);
        l->value = r->value;
        return true;
    } else if (expr->type == ExprType::L_STRING && value->type == ExprType::L_STRING)
    {
        StringLiteral *l = static_cast<StringLiteral *>(expr);
        StringLiteral *r = static_cast<StringLiteral *>(value);
        l->value = r->value;
        return true;
    }

    if (expr->type == ExprType::L_NUMBER && value->type == ExprType::L_STRING)
    {
        ERROR("Cannot assign string to number");
        return false;
    } else if (expr->type == ExprType::L_STRING && value->type == ExprType::L_NUMBER)
    {
        ERROR("Cannot assign number to string");
        return false;
    }
    return false;
}

bool Environment::replace(const std::string &name, Expr *value)
{
    if (m_values.find(name) != m_values.end())
    {
        m_values[name] = value;
        return true;
    }
    if (parent != nullptr)
    {
        return parent->replace(name, value);
    }
    return false;
}

bool Environment::addInteger(const std::string &name, int value)
{
    NumberLiteral *nl = Factory::as().make_number();
    nl->value = value;
    return define(name, nl);
}

bool Environment::addDouble(const std::string &name, double value)
{
    NumberLiteral *nl = Factory::as().make_number();
    nl->value = value;
    return define(name, nl);
}

bool Environment::addString(const std::string &name, std::string value)
{
    StringLiteral *sl = Factory::as().make_string();
    sl->value = value;
    return define(name, sl);
}

bool Environment::addBoolean(const std::string &name, bool value)
{
    NumberLiteral *bl = Factory::as().make_number();
    bl->value = value ? 1 : 0;
    return define(name, bl);
}
