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

    return primary();
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
          NumberLiteral *b = Factory::as().make_number();
          b->value = 0;
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

    Error("Expect expression.");
    
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
   Expr *initializer = nullptr;
   bool is_initialized = false;
   if (match(TokenType::EQUAL))
   {
       initializer = expression();
       is_initialized = true;
   }
   consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
   Declaration *stmt =  Factory::as().make_declaration();
   stmt->name = name;
   stmt->is_initialized = is_initialized;
   stmt->initializer = initializer;
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
    if (match({TokenType::IF}))
    {
        return if_statement();
    }
    if (match({TokenType::WHILE}))
    {
        return while_statement();
    }
    if (match({TokenType::FOR}))
    {
        return for_statement();
    }
    if (match({TokenType::PRINT}))
    {
        return print_statement();
    }

    if (match({TokenType::LEFT_BRACE}))
    {
        return block();
    }

    return expression_statement();
}

Stmt *Parser::declarations()
{
    try 
    {
        if (match({TokenType::VAR}))
        {
            return variable_declaration();
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
    std::vector<Stmt *> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
    {
        statements.push_back(declarations());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    BlockStmt *stmt =  Factory::as().make_block();
    stmt->statements = statements;
    return stmt;
}

Stmt *Parser::if_statement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    Expr *condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");
    Stmt *thenBranch = statement();
    Stmt *elseBranch = nullptr;
    if (match(TokenType::ELSE))
    {
        elseBranch = statement();
    }
    IFStmt *stmt =  Factory::as().make_if();
    stmt->condition = condition;
    stmt->then_branch = thenBranch;
    stmt->else_branch = elseBranch;
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
