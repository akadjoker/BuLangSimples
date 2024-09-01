#pragma once
#include "Config.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Arena.hpp"

class Interpreter;
class Context;

typedef Literal* (*NativeFunction)(Context* ctx, int argc);
typedef struct 
{
    const char* name;
    NativeFunction func;
} NativeFuncDef;


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
    virtual Expr *visit_call(CallExpr *node) = 0;
    virtual Expr* visit_get(GetExpr *node) = 0;
    virtual Expr* visit_get_definition(GetDefinitionExpr *node) = 0;
    virtual Expr* visit_set(SetExpr *node) = 0;
    

    virtual u8 execute(Stmt *stmt) = 0;
    virtual u8 visit_block_smt(BlockStmt *node) = 0;
    virtual u8 visit_expression_smt(ExpressionStmt *node) = 0;
    virtual u8 visit_print_smt(PrintStmt *node) = 0;
    virtual u8 visit_declaration(Declaration *node) = 0;
    virtual u8 visit_if(IFStmt *node) = 0;
    virtual u8 visit_while(WhileStmt *node) = 0;
    virtual u8 visit_do(DoStmt *node) = 0;
    virtual u8 visit_program(Program *node) = 0;
    virtual u8 visit_function(FunctionStmt *node) = 0;
    virtual u8 visit_for(ForStmt *node) = 0;
    virtual u8 visit_return(ReturnStmt *node) = 0;
    virtual u8 visit_break(BreakStmt *node) = 0;
    virtual u8 visit_switch(SwitchStmt *node) = 0;
    virtual u8 visit_continue(ContinueStmt *node) = 0;
    virtual u8 visit_struct(StructStmt *node) = 0;
    virtual u8 visit_class(ClassStmt *node) = 0;
    virtual u8 visit_array(ArrayStmt *node) = 0;
    virtual u8 visit_map(MapStmt *node) = 0;

    


    
};

class Environment
{

private:

    Environment *parent;
    u32 depth;
    std::unordered_map<std::string, Expr *> m_values;

public:
   Environment( );
    Environment( Environment *parent);
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

    u32 getDepth() const { return depth; }
};

class Context
{
    public:
        long        getLong(u8 index);
        int         getInt(u8 index);
        double      getDouble(u8 index);
        float       getFloat(u8 index);
        std::string getString(u8 index);
        bool        getBoolean(u8 index);

        Literal *asFloat(float value);
        Literal *asDouble(double value);
        Literal *asInt(int value);
        Literal *asLong(long value);
        Literal *asString(std::string value);
        Literal *asBoolean(bool value);


        bool isNumber(u8 index);
        bool isString(u8 index);
        


    private:
    friend class Interpreter;
    friend class Compiler;

    Context(Interpreter *interpreter);
    ~Context();

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    std::vector<Literal*> literals;
    Interpreter *interpreter;

    void clear();
};

struct Function : public Literal 
{
    std::string args[32];
    u32 arity;
    Token name;
    Stmt *body;
    Function();
};


struct Native : public Literal
{
    Token name;
    std::vector<Expr*> args;

    Native();
};

struct ClassLiteral : public Literal
{
    std::string name;
    ClassLiteral();
};

struct StructLiteral : public Literal
{
    std::string name;
    std::unordered_map<std::string, Expr *> members;
    StructLiteral();
    
};

struct ArrayLiteral : public Literal
{
    std::vector<Expr *> values;
    std::string name;
    ArrayLiteral();
};

struct MapLiteral : public Literal
{
    std::unordered_map<std::string, Expr *> values;
    MapLiteral();
};

struct Compiler : public Visitor
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
        Expr *visit_call(CallExpr *node) override;
        Expr* visit_get(GetExpr *node) override;
        Expr* visit_get_definition(GetDefinitionExpr *node) override;
        Expr *visit_set(SetExpr *node) override;
        Expr *visit_call_native(CallExpr *node) ;
        

        u8 execute(Stmt *stmt);

        u8 visit_block_smt(BlockStmt *node)override; 
        u8 visit_expression_smt(ExpressionStmt *node)override;
        u8 visit_print_smt(PrintStmt *node)override;
        u8 visit_declaration(Declaration *node)override;
        u8 visit_if(IFStmt  *node)override;
        u8 visit_while(WhileStmt *node)override;
        u8 visit_for(ForStmt *node)override;
        u8 visit_do(DoStmt *node);
        u8 visit_return(ReturnStmt *node)override;
        u8 visit_break(BreakStmt *node)override;
        u8 visit_continue(ContinueStmt *node)override;
        u8 visit_switch(SwitchStmt *node)override;
        u8 visit_program(Program *node)override;
        u8 visit_function(FunctionStmt *node)override;

        u8 visit_struct(StructStmt *node)override;
        u8 visit_class(ClassStmt *node) override;

        u8 visit_array(ArrayStmt *node) override;
        u8 visit_map(MapStmt *node) override;


        Compiler(Interpreter *i, Compiler *parent = nullptr);
        ~Compiler();




    private:
        friend class Interpreter;
        Interpreter *interpreter;
        Environment *global;
        Compiler *parent;
        u32 loop_count = 0;
        std::vector<Function*> functions;
        std::unordered_map<std::string, FunctionStmt*> functionMap;
        std::vector<StructLiteral*> structs;
        std::vector<ArrayLiteral*>  arrays;
        std::vector<MapLiteral*>    maps;

        

    void enterBlock();  
    void exitBlock();
    Environment* enterLocal(Environment *parent);

    Environment *currentEnvironment();
    Environment *globalEnvironment();
    std::stack<Environment *> environmentStack;





    void clear();
};



class Interpreter
{

public:
        ~Interpreter() ;
        Interpreter();

        bool compile(const std::string &source);

        void clear();

        void registerFunction(const std::string &name, NativeFunction function);

        bool isnative(const std::string &name);


    private:
    friend class Compiler;
    friend class Context;

    Compiler *compiler;
    Context *context;

    std::unordered_map<std::string, NativeFunction> nativeFunctions;
    Literal *CallNativeFunction(const std::string &name, int argc);


    void Error(const Token &token,const std::string &message);
    void Error(const std::string &message);
    void Warning(const Token &token, const std::string &message);
    void Warning(const std::string &message);
    void Info(const std::string &message);
};

