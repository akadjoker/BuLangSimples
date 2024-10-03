#include "pch.h"
#include "Parser.hpp"
#include "Utils.hpp"
#include "Factory.hpp"





Parser::Parser()
{

}

    
void Parser::Load( std::vector<Token> tokens)
{
this->tokens = tokens;
current = 0;
panicMode = false;
countBegins = 0;
countEnds = 0;
}

Parser::~Parser()
{
   
}

void Parser::clear()
{
    freecalls();

    tokens.clear();
    current = 0;
    panicMode = false;
    countBegins = 0;
    countEnds = 0;
}

bool Parser::match(const std::vector<TokenType> &types)
{
    for (auto type : types)
    {
        if (check(type))
        {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }

    return false;
}

Token Parser::consume(TokenType type,const std::string &message)
{
    if (check(type))
    {
      return   advance();
    }
    else
    {
        Error(tokens[current],message+" have '"+tokens[current].lexeme+"'");
        return peek();
    }
}

bool Parser::check(TokenType type)
{
    if (isAtEnd()) return false;
    return tokens[current].type == type;
}

bool Parser::isAtEnd()
{
    return tokens[current].type == TokenType::END_OF_FILE;
}

void Parser::synchronize()
{

    advance();
    while(!isAtEnd())
    {
        if (tokens[current].type == TokenType::SEMICOLON) return;

        switch (tokens[current].type)
        {
            case TokenType::CLASS:
            case TokenType::FUNCTION:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::NOW:
                return;
            }
        advance();
    }
}

Token Parser::advance()
{
    if (!isAtEnd()) current++;
    return previous();
}

Token Parser::peek()
{
    return tokens[current];
}

Token Parser::previous()
{
    return tokens[current - 1];
}

Token Parser::lookAhead()
{
    if (current + 1 >= (int)tokens.size()) return previous();
    return tokens[current + 1];
}



void Parser::Error(const Token &token,const std::string &message)
{
    panicMode = true;
    int line = token.line;
    std::string text =message+ " at line: " +std::to_string(line);
    Log(2, text.c_str());
    synchronize();

    throw FatalException(text);
}
void Parser::Error(const std::string &message)
{
    panicMode = true;
    Log(2, message.c_str());
    synchronize();

    throw FatalException(message);
}
void Parser::Warning(const Token &token,const std::string &message)
{
    int line = token.line;
    std::string text =message+ " at line: " +std::to_string(line);
    Log(1, text.c_str());
}

//******************************************************************************************************************* */

Expr *Parser::expression()
{
    return assignment();
}

Expr *Parser::assignment()
{
    Expr *expr = logical_or();
    Token token = previous();

   


    if (match(TokenType::EQUAL) )
    {
        Token op = previous();
        Expr *value = assignment();

        if (expr->type == ExprType::VARIABLE)
        {
           Variable *var = (Variable *)expr;
           
           Assign *assign = Factory::as().make_assign();
           assign->name = var->name;
           assign->value = value;
           expr = assign;
           return assign;
        } else if (expr->type == ExprType::GET)
        {
            GetExpr *get = (GetExpr *)expr;
           
            SetExpr *set = Factory::as().make_set();
            set->name  = get->name;
            set->object = get->object;
            set->value = value;
           
           return set;
        }
        else
        {
            Error("Invalid assignment target.");
        }
    }  else 
    if (match(TokenType::PLUS_EQUAL))
    {
       
        Expr *value = assignment();
        if (expr->type == ExprType::VARIABLE)
        {
            Variable *var = (Variable *)expr;
            Assign *assign = Factory::as().make_assign();
            assign->name = var->name;


            BinaryExpr *addition = Factory::as().make_binary();
            ((BinaryExpr *)addition)->left  = expr;
            ((BinaryExpr *)addition)->right = value;
            ((BinaryExpr *)addition)->op = Token(TokenType::PLUS_EQUAL, token.lexeme,token.literal, token.line);

           assign->value = addition;         

           expr = assign;
           return assign;
        } else
        {
            Error("Invalid binary target.");
        }
    } else 
    if (match(TokenType::MINUS_EQUAL))
    {
       
       Expr *value = assignment();
        if (expr->type == ExprType::VARIABLE)
        {
            Variable *var = (Variable *)expr;
            Assign *assign = Factory::as().make_assign();
            assign->name = var->name;


            BinaryExpr *addition = Factory::as().make_binary();
            ((BinaryExpr *)addition)->left  = expr;
            ((BinaryExpr *)addition)->right = value;
            ((BinaryExpr *)addition)->op = Token(TokenType::MINUS_EQUAL, token.lexeme,token.literal, token.line);

           assign->value = addition;         

           expr = assign;
           return assign;
        } else
        {
            Error("Invalid binary target.");
        }
    } else 
    if (match(TokenType::STAR_EQUAL))
    {
        Expr *value = assignment();
        if (expr->type == ExprType::VARIABLE)
        {
            Variable *var = (Variable *)expr;
            Assign *assign = Factory::as().make_assign();
            assign->name = var->name;


            BinaryExpr *addition = Factory::as().make_binary();
            ((BinaryExpr *)addition)->left  = expr;
            ((BinaryExpr *)addition)->right = value;
            ((BinaryExpr *)addition)->op = Token(TokenType::STAR_EQUAL, token.lexeme,token.literal, token.line);

           assign->value = addition;         

           expr = assign;
           return assign;
        } else
        {
            Error("Invalid binary target.");
        }
    } else 
    if (match(TokenType::SLASH_EQUAL))
    {
        Expr *value = assignment();
        if (expr->type == ExprType::VARIABLE)
        {
            Variable *var = (Variable *)expr;
            Assign *assign = Factory::as().make_assign();
            assign->name = var->name;


            BinaryExpr *addition = Factory::as().make_binary();
            ((BinaryExpr *)addition)->left  = expr;
            ((BinaryExpr *)addition)->right = value;
            ((BinaryExpr *)addition)->op = Token(TokenType::SLASH_EQUAL, token.lexeme,token.literal, token.line);

           assign->value = addition;         

           expr = assign;
           return assign;
        } else
        {
            Error("Invalid binary target.");
        }
    }

    return expr;
}

Expr *Parser::logical_or()
{
    Expr *expr = logical_and();
    while (match({TokenType::OR}))
    {
        Token op = previous();
        Expr *right = logical_and();
        Expr *left = expr;
         expr = Factory::as().make_logical();
        ((LogicalExpr *)expr)->left  = left;
        ((LogicalExpr *)expr)->right = right;
        ((LogicalExpr *)expr)->op = op;
    }
    return expr;
}

Expr *Parser::logical_and()
{
    Expr *expr = logical_xor();
    while (match({TokenType::AND}))
    {
        Token op = previous();
        Expr *right = logical_xor();
        Expr *left = expr;
         expr = Factory::as().make_logical();
        ((LogicalExpr *)expr)->left  = left;
        ((LogicalExpr *)expr)->right = right;
        ((LogicalExpr *)expr)->op = op;
    }
    return expr;
}

Expr *Parser::logical_xor()
{
    Expr *expr = equality();
    while (match({TokenType::XOR}))
    {
        Token op = previous();
        Expr *right = equality();
        Expr *left = expr;
         expr = Factory::as().make_logical();
        ((LogicalExpr *)expr)->left  = left;
        ((LogicalExpr *)expr)->right = right;
        ((LogicalExpr *)expr)->op = op;
    }
    return expr;

}



Expr *Parser::equality()
{
    Expr *expr = comparison();

    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL}))
    {
        Token op = previous();
        Expr *right = comparison();
        Expr *left = expr;
         expr = Factory::as().make_binary();
        ((BinaryExpr *)expr)->left  = left;
        ((BinaryExpr *)expr)->right = right;
        ((BinaryExpr *)expr)->op = op;
    
    }
    return expr;
}

Expr *Parser::comparison()
{
    Expr *expr = term();
    while (match({TokenType::GREATER, TokenType::LESS, TokenType::GREATER_EQUAL, TokenType::LESS_EQUAL}))
    {
        Token op = previous();
        Expr *right = term();
        Expr *left = expr;
         expr = Factory::as().make_binary();
        ((BinaryExpr *)expr)->left  = left;
        ((BinaryExpr *)expr)->right = right;
        ((BinaryExpr *)expr)->op = op;
    }
    return expr;
}



Expr *Parser::term()
{
    Expr *expr = factor();

    while (match({TokenType::MINUS, TokenType::PLUS}))
    {
         Token op = previous();
         Expr *right = factor();
         Expr *left = expr;
         expr = Factory::as().make_binary();
        ((BinaryExpr *)expr)->left  = left;
        ((BinaryExpr *)expr)->right = right;
        ((BinaryExpr *)expr)->op = op;
    }
    return expr;
}

Expr *Parser::factor()
{
    Expr *expr = unary();

    while (match({TokenType::SLASH, TokenType::STAR, TokenType::MOD}))
    {
        Token op = previous();
        Expr *right = unary();
        Expr *left = expr;
         expr = Factory::as().make_binary();
        ((BinaryExpr *)expr)->left  = left;
        ((BinaryExpr *)expr)->right = right;
        ((BinaryExpr *)expr)->op = op;
    }
    return expr;

}

Expr *Parser::unary()
{
    if (match({TokenType::BANG, TokenType::MINUS, TokenType::INC, TokenType::DEC}))
    {
        Token op = previous();
        Expr *right = unary();
        UnaryExpr *u_expr =  Factory::as().make_unary();
        u_expr->right = right;
        u_expr->op = op;
        u_expr->isPrefix = (op.type == TokenType::INC || op.type == TokenType::DEC);
        return u_expr;
    }

    return call();
}

static int count = 10;

Expr *Parser::call()
{
    Expr *expr = primary();



    while (true)
    {
        Token name = previous();
        if (match(TokenType::LEFT_PAREN))
        {
            expr = function_call(expr, name);
        } else if (match(TokenType::DOT))
        {
            Token name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
           //      INFO("Get property: %s %s", name.lexeme.c_str(),expr->toString().c_str());

                if (match(TokenType::LEFT_PAREN))
                {
                    GetDefinitionExpr *get = Factory::as().make_get_definition();
                    if (!check(TokenType::RIGHT_PAREN))
                    {
                        do
                        {
                            Expr *value  =  expression();
                            get->values.push_back(std::move(value));
                            
                        } while (match(TokenType::COMMA));
                    }
                    consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");

                    get->name = std::move(name);
                    get->variable = expr;
                    return get;
                }
            



            GetExpr *get = Factory::as().make_get();
            get->name = std::move(name);
            get->object = std::move(expr);
            return get;
        } 
        else
        {
            break;
        }
    }

    return expr;
}


Expr *Parser::function_call(Expr *expr,  Token name )
{
    CallExpr *f = Factory::as().make_call();
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            Expr *arg = expression();
            f->args.push_back(std::move(arg));
            
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");

  //  INFO("Function call: %s", name.lexeme.c_str());


   f->callee = expr;
   f->name = name;
   calls.push_back(f);
    return f;
}

void Parser::freecalls()
{
    for (auto call : calls)
    {
       Factory::as().free_call(call);
    }
}

Expr *Parser::primary()
{
     if (match(TokenType::FALSE))
    {
        
          NumberLiteral *b = Factory::as().make_number();
          b->value = 0;
          return b;
    }
    if (match(TokenType::TRUE))
    {
          NumberLiteral *b = Factory::as().make_number();
          b->value = 1;
          return b;
    }
    
    if (match(TokenType::NIL))
    {
          Literal *b = Factory::as().make_literal();
          
          return b;
    }


    if (match(TokenType::STRING))
    {
        StringLiteral *s =  Factory::as().make_string();
        s->value = previous().literal;
        return s;
    }
    if (match(TokenType::NUMBER))
    {
        NumberLiteral *f = Factory::as().make_number();

        f->value = std::stof(previous().literal);
        return f;
    }
    if (match(TokenType::NOW))
    {
        NowExpr *expr =  Factory::as().make_now();
        return expr;
    }
    if (match(TokenType::IDENTIFIER))
    {
        Token name = previous();
        Variable *expr =  Factory::as().make_variable();
        expr->name = name;

          
        


        if (match(TokenType::INC))
        {
  
            Token op = previous();
            UnaryExpr *u_expr =  Factory::as().make_unary();
            u_expr->right = expr;
            u_expr->op = op;
            u_expr->isPrefix = false;
            return u_expr;
        }
        if (match(TokenType::DEC))
        {
            Token op = previous();
            UnaryExpr *u_expr =  Factory::as().make_unary();    
            u_expr->right = expr;
            u_expr->op = op;
            u_expr->isPrefix = false;
            return u_expr;
        }
    

        return expr;
    }


    if (match(TokenType::LEFT_PAREN))
    {
        Expr *expr = expression();
        consume(TokenType::RIGHT_PAREN,"Expect ')' after expression.");
        return expr;
    }
    Token current = peek();
    Error("Primary : Expect expression. "  + current.lexeme);
    
    return nullptr;
}


Expr *Parser::now()
{
    NowExpr *expr =  Factory::as().make_now();
    return expr;
}

Program *Parser::program()
{
    Program *p =  Factory::as().make_program();
    while (!isAtEnd())
    {
        p->statements.push_back(declarations());
    }
    return p;
}

Stmt *Parser::expression_statement()
{
    Expr *expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    ExpressionStmt *stmt =  Factory::as().make_expression();
    stmt->expression = expr;
    return stmt;
}

Stmt *Parser::variable_declaration()
{
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
    std::vector<Token> names;
    names.push_back(name);

   Expr *initializer = nullptr;
   bool is_initialized = false;
   if (match(TokenType::LEFT_BRACKET))//array
   {
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after array declaration.");
        std::vector<Expr*> values;
    
        if (match(TokenType::EQUAL))
        {
            consume(TokenType::LEFT_BRACKET, "Expect '[' array initializer.");
            Expr *exp = expression();
            values.push_back(exp);
            while (match(TokenType::COMMA)  && !isAtEnd())
            {
                Expr *exp = expression();
                values.push_back(exp);
            }
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after array initializer.");
        }
        consume(TokenType::SEMICOLON, "Expect ';' after array declaration.");

        ArrayStmt *stmt =  Factory::as().make_array();
        stmt->name = std::move(name);
        stmt->values = std::move(values);
        return stmt;
   } else if  (match(TokenType::LEFT_BRACE)) 
   {
        consume(TokenType::RIGHT_BRACE, "Expect '}' after dictionary declaration.");
        std::unordered_map<Expr*, Expr*> values;
    
         if (match(TokenType::EQUAL))
         {
              consume(TokenType::LEFT_BRACE, "Expect '{' dictionary initializer.");
              Expr *key = expression();
              consume(TokenType::COLON, "Expect ':' after dictionary key.");
              Expr *value = expression();
              values[key] = value;  
             while (match(TokenType::COMMA)  && !isAtEnd())
             {
                 Expr *key = expression();
                 consume(TokenType::COLON, "Expect ':' after dictionary key.");
                 Expr *value = expression();
                 values[key] = value;
             }
             consume(TokenType::RIGHT_BRACE, "Expect '}' after dictionary initializer.");
        }
        consume(TokenType::SEMICOLON, "Expect ';' after dictionary declaration.");

        MapStmt *stmt =  Factory::as().make_map();
        stmt->name = std::move(name);
        stmt->values = std::move(values);       
        return stmt;

   } if (match(TokenType::EQUAL))
   {
       initializer = expression();
       is_initialized = true;
   } else 
   { 

        while (match(TokenType::COMMA) && !isAtEnd())
        {
           Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
           names.push_back(name);
        }
         if (match(TokenType::EQUAL))
        {
            initializer = expression();
            is_initialized = true;
        } 
   }
   consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
   Declaration *stmt =  Factory::as().make_declaration();
   stmt->names = std::move(names);
   stmt->is_initialized = is_initialized;
   stmt->initializer = initializer;
   return stmt;
}

Stmt *Parser::function_declaration()
{
    Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
    std::vector<std::string> names;

    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
           Token name =  consume(TokenType::IDENTIFIER, "Expect parameter name.");
           names.push_back(std::move(name.lexeme));
        } while (match(TokenType::COMMA));
    }
    

    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");


    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");

    FunctionStmt *stmt =  Factory::as().make_function();
    stmt->name = std::move(name);
    stmt->args = std::move(names);
    stmt->body = block();
    return stmt;
}

Stmt *Parser::print_statement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'print'.");
    Expr *expr = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after value.");
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    PrintStmt *stmt =  Factory::as().make_print();
    stmt->expression = expr;
    return stmt;
}

//******************************************************************************************************************* */
Stmt *Parser::statement()
{
    if (match(TokenType::FUNCTION))
    {
        return function_declaration();
    }
    if (match(TokenType::IF))
    {
        return if_statement();
    }
    if (match(TokenType::SWITCH))
    {
        return switch_statement();
    }
    if (match(TokenType::RETURN))
    {
        return return_statement();
    }
    if (match(TokenType::BREAK))
    {
        return break_statement();
    }
    if (match(TokenType::CONTINUE))
    {
        return continue_statement();
    }
    if (match(TokenType::WHILE))
    {
        return while_statement();
    }
    if (match(TokenType::DO))
    {
        return do_statement();
    }
    if (match(TokenType::FOR))
    {
        return for_statement();
    }
    if (match(TokenType::FROM))
    {
        return from_statement();
    }
    if (match(TokenType::PRINT))
    {
        return print_statement();
    }

    if (match(TokenType::LEFT_BRACE))
    {
        return block();
    }

    return expression_statement();
}

Stmt *Parser::declarations()
{
    try 
    {
        if (match(TokenType::VAR))
        {
            return variable_declaration();
        }
        if (match(TokenType::STRUCT))
        {
            return struct_declaration();
        }

        if (match(TokenType::CLASS))
        {
            return class_declaration();
        }
        return statement();
    }
    catch (FatalException e)
    {
        synchronize();
        return   nullptr;
    }
    

    return   nullptr;
}

Stmt *Parser::block()
{
    BlockStmt *stmt =  Factory::as().make_block();
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
    {
        stmt->statements.push_back(declarations());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");


    return stmt;
}

Stmt *Parser::if_statement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    Expr *condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");
    Stmt *thenBranch = statement();


    std::vector<ElifStmt*> elifBranch;
    while (match(TokenType::ELIF))
    {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'elif'.");
        Expr* condition = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
        Stmt *thenBranch = statement();
        ElifStmt *elif =  Factory::as().make_elif();
        elif->condition = condition;
        elif->then_branch = thenBranch;
        elifBranch.push_back(elif);
    }
    


    Stmt *elseBranch = nullptr;
    if (match(TokenType::ELSE))
    {
        elseBranch = statement();
    }
    IFStmt *stmt =  Factory::as().make_if();
    stmt->condition = condition;
    stmt->then_branch = thenBranch;
    stmt->else_branch = elseBranch;
    stmt->elifBranch = std::move(elifBranch);
    return stmt;
}

Stmt *Parser::while_statement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    Expr *condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");
    Stmt *body = statement();
    WhileStmt *stmt =  Factory::as().make_while();
    stmt->condition = condition;
    stmt->body = body;
    return stmt;
}

Stmt *Parser::do_statement()
{

    Stmt *body = statement();
    consume(TokenType::WHILE, "Expect 'while' after 'do'.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    Expr *condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after 'while' condition.");
    consume(TokenType::SEMICOLON, "Expect ';' after 'while' body.");
    DoStmt *stmt =  Factory::as().make_do();
    stmt->condition = condition;
    stmt->body = body;
    return stmt;    
}


Stmt *Parser::from_statement()
{

    consume(TokenType::LEFT_PAREN, "Expect '(' after 'from'.");
    consume(TokenType::VAR, "Expect variable declaration  .");
    Expr * var =  expression();
    consume(TokenType::COLON, "Expect ':' after variable.");
    Expr *array =  expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after 'from' condition.");
    Stmt *body = statement();
    FromStmt *stmt =  Factory::as().make_from();
    stmt->variable = var;
    stmt->array = array;
    stmt->body = body;
    return stmt;
}

Stmt *Parser::for_statement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
    Stmt *initializer=nullptr;
    if (match({TokenType::SEMICOLON}))
    {
       Error(peek(), "Missing 'for' initializer.");
    } else if (match({TokenType::VAR}))
    {
        initializer = variable_declaration();
    } else
    {
        initializer = expression_statement();
    }


    if (match({TokenType::SEMICOLON}))
    {
       Error("Missing 'for' condition.");
    }
    Expr *condition  = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after for condition.");




    if (match(TokenType::SEMICOLON))
    {
        Error(peek(), "Missing 'for' step.");
    }

    Expr *increment =   expression();
    
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");
    
    Stmt *body = statement();


    ForStmt *stmt =  Factory::as().make_for();
    
    stmt->initializer = initializer;
    stmt->condition = condition;
    stmt->increment = increment;
    stmt->body = body;


    return stmt;
}

Stmt *Parser::return_statement()
{
    Token keyword = previous();
    Expr *value = nullptr;
    if (!check(TokenType::SEMICOLON))
    {
        value = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    ReturnStmt *stmt =  Factory::as().make_return();
    stmt->value = value;
    return stmt;
}

Stmt *Parser::break_statement()
{
    consume(TokenType::SEMICOLON, "Expect ';' after 'break'.");
    BreakStmt *stmt =  Factory::as().make_break();
    return stmt;
}

Stmt *Parser::continue_statement()
{
    consume(TokenType::SEMICOLON, "Expect ';' after 'continue'.");
    ContinueStmt *stmt =  Factory::as().make_continue();
    return stmt;

}

Stmt *Parser::switch_statement()
{
    consume(TokenType::LEFT_PAREN,"Expect '(' after 'switch'.");
    Expr *condition = expression();
    consume(TokenType::RIGHT_PAREN,"Expect ')' after condition.");
    consume(TokenType::LEFT_BRACE,"Expect '{' before switch block.");
    
    
    std::vector<CaseStmt*> cases;
    while (match(TokenType::CASE))
    {
        Expr *exp = expression();
        consume(TokenType::COLON,"Expect ':' after case expression.");
        Stmt *body = statement();
        CaseStmt *case_stmt =  Factory::as().make_case();
        case_stmt->condition = exp;
        case_stmt->body = body;
        cases.push_back(case_stmt);
    }


    Stmt* default_case= nullptr;
    if (match(TokenType::DEFAULT))
    {
        consume(TokenType::COLON,"Expect ':' after default case.");
        default_case = statement();
    }

    consume(TokenType::RIGHT_BRACE,"Expect '}' after switch block.");

    if (default_case == nullptr && cases.size() == 0)
    {
        Error(peek(), "Switch statement must have at least one case or default case.");
    }
   

    SwitchStmt *stmt =  Factory::as().make_switch();
    stmt->condition = condition;
    stmt->cases = std::move(cases);
    stmt->defaultBranch = default_case;
    return stmt;
}

Stmt *Parser::class_declaration()
{

    Token name = consume(TokenType::IDENTIFIER, "Expect class name.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");

    ClassStmt *stmt =  Factory::as().make_class();
    stmt->name = std::move(name);

    

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
    {
        //stmt->fields.push_back(variable_declaration());
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");

    return stmt;
}

Stmt *Parser::struct_declaration()
{

    Token name = consume(TokenType::IDENTIFIER, "Expect struct name.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before struct body.");

    StructStmt *stmt =  Factory::as().make_struct();
    stmt->name = std::move(name);

    

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
    {
        if (match(TokenType::VAR))
        {
            Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
            stmt->fields.push_back(std::move(name));
            if (match(TokenType::EQUAL))
            {
                stmt->values.push_back(expression());
            } else 
            {
                Literal *l =  Factory::as().make_literal();
                stmt->values.push_back(l);
            }
            consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
        }
          
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after struct body.");
    consume(TokenType::SEMICOLON, "Expect ';' after struct declaration.");

    return stmt;
}

//*******************************************************************************************************************


Program *Parser::parse()
{
    try 
    {
        return   program();
    }
    catch (FatalException e)
    {
        return   nullptr;
    }

    return   nullptr;
}
