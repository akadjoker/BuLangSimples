#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Token.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"

class Parser
{
public:
    Parser();
    ~Parser();

    void Load( std::vector<Token> tokens);

    Program * parse();

    void clear();

private:


    std::vector<Token> tokens;
    int current;
    bool panicMode;
    int countBegins;
    int countEnds ;




    bool match(const std::vector<TokenType> &types);
    bool match(TokenType type);
    Token consume(TokenType type, const std::string &message);
    bool check(TokenType type);
    bool isAtEnd();
    void synchronize();
    Token advance();
    Token peek();
    Token previous();
    Token lookAhead();

    void Error(const Token &token,const std::string &message);
    void Error(const std::string &message);
    void Warning(const Token &token, const std::string &message);


    Expr* expression();
    Expr *equality();
    Expr* comparison();
    Expr * assignment();
    Expr* term();
    Expr* factor();
    Expr* unary();  
    Expr* primary(); 
    Expr* logical_or();
    Expr* logical_and();
    Expr* logical_xor(); 


    Expr *call(); 
    Expr *function_call(Expr *callee,  Token name );

    void freecalls();


    Expr *now();

    Program *program();

    Stmt *expression_statement();
    Stmt *variable_declaration();
    Stmt *function_declaration();
    Stmt *print_statement();
    Stmt *statement();
    Stmt *declarations();
    Stmt *block();

    Stmt *if_statement();
    Stmt *while_statement();
    Stmt *do_statement();
    Stmt *for_statement();
    Stmt *from_statement();
    Stmt *return_statement();
    Stmt *break_statement();
    Stmt *continue_statement();
    Stmt *switch_statement();
    Stmt *class_declaration();
    Stmt *struct_declaration();

    std::vector<CallExpr*> calls;
};