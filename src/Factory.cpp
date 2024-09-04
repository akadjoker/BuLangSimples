#include "pch.h"
#include "Arena.hpp"
#include "Factory.hpp"
#include "Interpreter.hpp"
#include "Utils.hpp"


Factory::Factory()
{
}

Factory::~Factory()
{
}

Factory &Factory::as()
{
    static Factory factory;
    return factory;
}

static float memoryInMB(size_t bytes)
{
    return static_cast<float>(bytes) / (1024.0f * 1024.0f);
}

static float memoryInKB(size_t bytes)
{
    return static_cast<float>(bytes) / 1024.0f;
}

struct AllocationInfo
{
	u32 size;
	std::string file;
	u32 line;
};

static std::unordered_map<void *, AllocationInfo> g_allocations;
static bool g_debugMode = false;

void *Malloc(u32 size, const char *file, u32 line)
{
	void *mem = Factory::as().arena.Allocate(size);
	if (g_debugMode && mem != nullptr)
	{
		g_allocations[mem] = {size, file, line};
	}
	return mem;
}

void Free(void *mem, u32 size)
{
	if (g_debugMode && mem != nullptr)
	{
		auto it = g_allocations.find(mem);
		if (it != g_allocations.end())
		{
			g_allocations.erase(it);
		}
	}

    Factory::as().arena.Free(mem, size);
}
void ReportMemoryLeaks()
{
	if (!g_debugMode)
		return;

	if (!g_allocations.empty())
	{
		WARNING("Memory Leaks Detected:");
		for (const auto &entry : g_allocations)
		{
			WARNING("Leaked %d bytes at %p, allocated at %s:%d",
				   entry.second.size,
				   entry.first,
				   entry.second.file.c_str(),
				   entry.second.line);
		}
	}
	else
	{
		INFO("No memory leaks detected.\n");
	}
}

static const  char* memoryIn(size_t bytes)
{
    if (bytes >= 1.0e6)
    {
        return FormatText("%.2f MB", memoryInMB(bytes));
    }
    else if (bytes >= 1.0e3)
    {
        return FormatText("%.2f KB", memoryInKB(bytes));
    }
    return FormatText("%zu bytes", bytes);
}

void Factory::clear()
{
    ReportMemoryLeaks();
    INFO("Arena cleared %s",memoryIn(arena.size()));
}

Environment *Factory::make_environment( Environment *parent)
{
    void *p = arena.Allocate(sizeof(Environment));
    return new (p) Environment(parent);
}



void Factory::free_environment(Environment *expr)
{
    expr->~Environment();
    arena.Free(expr, sizeof(Environment));
}

EmptyExpr *Factory::make_empty()
{

    void *p = ARENA_ALLOC(sizeof(EmptyExpr));
    return new (p) EmptyExpr();
}


void Factory::free_empty(EmptyExpr *expr)
{
    expr->~EmptyExpr();
    ARENA_FREE(expr, sizeof(EmptyExpr));
}

BinaryExpr *Factory::make_binary()
{

    void *p = ARENA_ALLOC(sizeof(BinaryExpr));
    return new (p) BinaryExpr();
}




void Factory::free_binary(BinaryExpr *expr)
{

   // delete_expression(expr->left);
    delete_expression(expr->right);
    expr->~BinaryExpr();
    ARENA_FREE(expr, sizeof(BinaryExpr));
}

UnaryExpr *Factory::make_unary()
{
    void *p = ARENA_ALLOC(sizeof(UnaryExpr));
    return new (p) UnaryExpr();
}

void Factory::free_unary(UnaryExpr *expr)
{
  //  delete_expression(expr->right);
    

    expr->~UnaryExpr();
    ARENA_FREE(expr, sizeof(UnaryExpr));
}

LogicalExpr *Factory::make_logical()
{
    void *p = ARENA_ALLOC(sizeof(LogicalExpr));
    return new (p) LogicalExpr();
}

void Factory::free_logical(LogicalExpr *expr)
{

    delete_expression(expr->left);
    delete_expression(expr->right);
    expr->~LogicalExpr();
    ARENA_FREE(expr, sizeof(LogicalExpr));
}

GroupingExpr *Factory::make_grouping()
{
    void *p = ARENA_ALLOC(sizeof(GroupingExpr));
    return new (p) GroupingExpr();
}

void Factory::free_grouping(GroupingExpr *expr)
{

    delete_expression(expr->expr);
    expr->~GroupingExpr();
    ARENA_FREE(expr, sizeof(GroupingExpr));
}

NumberLiteral *Factory::make_number()
{
    void *p = ARENA_ALLOC(sizeof(NumberLiteral));
    return new (p) NumberLiteral();
}

void Factory::free_number(NumberLiteral *expr)
{
    expr->~NumberLiteral();
    ARENA_FREE(expr, sizeof(NumberLiteral));
}

StringLiteral *Factory::make_string()
{
    void *p = ARENA_ALLOC(sizeof(StringLiteral));
    return new (p) StringLiteral();
}

void Factory::free_string(StringLiteral *expr)
{
    expr->~StringLiteral();
    ARENA_FREE(expr, sizeof(StringLiteral));
}

Literal *Factory::make_literal()
{
    void *p = ARENA_ALLOC(sizeof(Literal));
    return new (p) Literal();
}

void Factory::free_literal(Literal *expr)
{
    expr->~Literal();
    ARENA_FREE(expr, sizeof(Literal));
}

ArrayLiteral *Factory::create_array()
{
    void *p = ARENA_ALLOC(sizeof(ArrayLiteral));
    return new (p) ArrayLiteral();
}

void Factory::delete_array(ArrayLiteral *expr)
{
    expr->~ArrayLiteral();
    ARENA_FREE(expr, sizeof(ArrayLiteral));
}

MapLiteral *Factory::create_map()
{
    void *p = ARENA_ALLOC(sizeof(MapLiteral));
    return new (p) MapLiteral();
}

void Factory::delete_map(MapLiteral *expr)
{
    expr->~MapLiteral();
    ARENA_FREE(expr, sizeof(MapLiteral));
}

NowExpr *Factory::make_now()
{
    void *p = ARENA_ALLOC(sizeof(NowExpr));
    return new (p) NowExpr();
}

void Factory::free_now(NowExpr *expr)
{
    expr->~NowExpr();
    ARENA_FREE(expr, sizeof(NowExpr));
}

PrintStmt *Factory::make_print()
{
    void *p = ARENA_ALLOC(sizeof(PrintStmt));
    return new (p) PrintStmt();
}

void Factory::free_print(PrintStmt *expr)
{
    expr->~PrintStmt();
    ARENA_FREE(expr, sizeof(PrintStmt));
}

ExpressionStmt *Factory::make_expression()
{
    void *p = ARENA_ALLOC(sizeof(ExpressionStmt));
    return new (p) ExpressionStmt();
}

void Factory::free_expression(ExpressionStmt *expr)
{
       // INFO("Free expression statmnent");
        delete_expression(expr->expression);
 
        expr->~ExpressionStmt();
        ARENA_FREE(expr, sizeof(ExpressionStmt));
    

}

BlockStmt *Factory::make_block()
{
    void *p = ARENA_ALLOC(sizeof(BlockStmt));
    return new (p) BlockStmt();
}

void Factory::free_block(BlockStmt *expr)
{
   // INFO("Free block");
    for (auto &stmt : expr->statements)
    {
       delete_statement(stmt);
    }
    expr->~BlockStmt();
    ARENA_FREE(expr, sizeof(BlockStmt));
}

Program *Factory::make_program()
{
    void *p = ARENA_ALLOC(sizeof(Program));
    return new (p) Program();
}

void Factory::free_program(Program *expr)
{
//
    INFO("Free program");
    for (auto &stmt : expr->statements)
    {
        delete_statement(stmt);
    }
    expr->statements.clear();
    expr->~Program();
    ARENA_FREE(expr, sizeof(Program));
}

FunctionStmt *Factory::make_function()
{
    void *p = ARENA_ALLOC(sizeof(FunctionStmt));
    return new (p) FunctionStmt();
}

void Factory::free_function(FunctionStmt *expr)
{

  //  INFO("Free function");
    delete_statement(expr->body);
    expr->args.clear();
    expr->~FunctionStmt();
    ARENA_FREE(expr, sizeof(FunctionStmt));
}

Function *Factory::createFunction()
{
    void *p = ARENA_ALLOC(sizeof(Function));
    return new (p) Function();
}

void Factory::deleteFunction(Function *node)
{
    node->~Function();
    ARENA_FREE(node, sizeof(Function));
}

StructLiteral *Factory::createStruct()
{
    void *p = ARENA_ALLOC(sizeof(StructLiteral));
    return new (p) StructLiteral();
}

void Factory::deleteStruct(StructLiteral *node)
{
//    WARNING("Deleting struct: %s", node->name.c_str());
    node->members.clear();
    
    node->~StructLiteral();
    ARENA_FREE(node, sizeof(StructLiteral));
}

ClassLiteral *Factory::createClass()
{
    void *p = ARENA_ALLOC(sizeof(ClassLiteral));
    return new (p) ClassLiteral();
}

void Factory::deleteClass(ClassLiteral *node)
{
    node->~ClassLiteral();
    ARENA_FREE(node, sizeof(ClassLiteral));
}

Native *Factory::createNative()
{
    void *p = ARENA_ALLOC(sizeof(Native));
    return new (p) Native();
}

void Factory::deleteNative(Native *node)
{
    node->~Native();
    ARENA_FREE(node, sizeof(Native));
}

CallExpr *Factory::make_call()
{
    void *p = ARENA_ALLOC(sizeof(CallExpr));
    return new (p) CallExpr();
}

void Factory::free_call(CallExpr *expr)
{
    for (auto &arg : expr->args)
    {
       delete_expression(arg);
    }

    expr->args.clear();
    expr->~CallExpr();
    ARENA_FREE(expr, sizeof(CallExpr));
}

GetExpr *Factory::make_get()
{
    void *p = ARENA_ALLOC(sizeof(GetExpr));
    return new (p) GetExpr();
}

void Factory::free_get(GetExpr *expr)
{
    expr->~GetExpr();
    ARENA_FREE(expr, sizeof(GetExpr));
}

SetExpr *Factory::make_set()
{
    void *p = ARENA_ALLOC(sizeof(SetExpr));
    return new (p) SetExpr();
}

void Factory::free_set(SetExpr *expr)
{
    expr->~SetExpr();
    ARENA_FREE(expr, sizeof(SetExpr));
}

GetDefinitionExpr *Factory::make_get_definition()
{
    void *p = ARENA_ALLOC(sizeof(GetDefinitionExpr));
    return new (p) GetDefinitionExpr();
}

void Factory::free_get_definition(GetDefinitionExpr *expr)
{
    expr->~GetDefinitionExpr();
    ARENA_FREE(expr, sizeof(GetDefinitionExpr));
}

void Factory::delete_statement(Stmt *expr)
{
    
    if (expr)
    {
        if (expr->type == StmtType::DECLARATION)
        {
          //  WARNING("Free declaration");
            free_declaration(static_cast<Declaration*>(expr));
        } else if (expr->type == StmtType::PRINT)
        {
           // WARNING("Free print");
            free_print(static_cast<PrintStmt*>(expr));
        } else if (expr->type == StmtType::EXPRESSION)
        {
          //  WARNING("Free expression");
           // free_expression(static_cast<ExpressionStmt*>(expr));
        } else if (expr->type == StmtType::BLOCK)
        {
          //  WARNING("Free block");
            free_block(static_cast<BlockStmt*>(expr));

        } else if (expr->type == StmtType::IF)
        {
           // WARNING("Free if");
            free_if(static_cast<IFStmt*>(expr));
        } else if (expr->type == StmtType::WHILE)
        {
            free_while(static_cast<WhileStmt*>(expr));
        }   else if (expr->type == StmtType::BREAK)
        {
           // WARNING("Free break");
            free_break(static_cast<BreakStmt*>(expr));            
        }  else if (expr->type == StmtType::RETURN)
        {
           // WARNING("Free return");
            free_return(static_cast<ReturnStmt*>(expr));
        } else if (expr->type == StmtType::CONTINUE)
        {
          //  WARNING("Free continue");
            free_continue(static_cast<ContinueStmt*>(expr));
        } 
        else if (expr->type == StmtType::FOR)
        {
          //  WARNING("Free for");
            free_for(static_cast<ForStmt*>(expr));
        } else if (expr->type == StmtType::SWITCH)
        {
          //  WARNING("Free switch");
            free_switch(static_cast<SwitchStmt*>(expr));
        } else if (expr->type == StmtType::DO)
        {
           // WARNING("Free do");
            free_do(static_cast<DoStmt*>(expr));
        } else if (expr->type == StmtType::FUNCTION)
        {
           // WARNING("Free function");
            free_function(static_cast<FunctionStmt*>(expr));
        } else if (expr->type == StmtType::CLASS)
        {
           // WARNING("Free class");
            free_class(static_cast<ClassStmt*>(expr));
        } else if (expr->type == StmtType::STRUCT)
        {

            free_struct(static_cast<StructStmt*>(expr));
        } else if (expr->type == StmtType::ARRAY)
        {
            free_array(static_cast<ArrayStmt*>(expr));
        } else if (expr->type == StmtType::MAP)
        {
            free_map(static_cast<MapStmt*>(expr));
        } else if (expr->type == StmtType::FROM)
        {
            free_from(static_cast<FromStmt*>(expr));
        }
        
        else 
        {
            INFO("Unknown statement type %d",(  int)expr->type);
        }
    }
}

void Factory::delete_expression(Expr *expr)
{
    return;
    if (expr)
    {
        if (expr->type == ExprType::BINARY)
        {
          //  free_binary(static_cast<BinaryExpr*>(expr));
        } else if (expr->type == ExprType::UNARY)
        {
            //free_unary(static_cast<UnaryExpr*>(expr));
        } else if (expr->type == ExprType::LOGICAL)
        {
            //free_logical(static_cast<LogicalExpr*>(expr));
        } else if (expr->type == ExprType::GROUPING)
        {
           // free_grouping(static_cast<GroupingExpr*>(expr));
        } else if (expr->type == ExprType::LITERAL)
        {
            free_literal(static_cast<Literal*>(expr));
        } else if (expr->type == ExprType::L_NUMBER)
        {
            free_number(static_cast<NumberLiteral*>(expr));
        } else if (expr->type == ExprType::L_STRING)
        {
            free_string(static_cast<StringLiteral*>(expr));
        } else if (expr->type == ExprType::NOW)
        {
           // free_now(static_cast<NowExpr*>(expr));
        } else if (expr->type == ExprType::L_STRUCT)
        {
            deleteStruct(static_cast<StructLiteral*>(expr));
        } else if (expr->type == ExprType::VARIABLE)
        {
            free_variable(static_cast<Variable*>(expr));
        } else if (expr->type == ExprType::GET)
        {
            free_get(static_cast<GetExpr*>(expr));
        } else if (expr->type == ExprType::GET_DEF)
        {
            free_get_definition(static_cast<GetDefinitionExpr*>(expr));
        } 
        else if (expr->type == ExprType::SET)
        {
            free_set(static_cast<SetExpr*>(expr));
        }  else if (expr->type == ExprType::CALL)
        {
          //  free_call(static_cast<CallExpr*>(expr));
        }  else if (expr->type == ExprType::ASSIGN)
        {
            free_assign(static_cast<Assign*>(expr));
        } else if (expr->type == ExprType::L_FUNCTION)
        {
           // free_function(static_cast<Function*>(expr));
        }
        else 
        {
            INFO("Unknown expression type %d",(  int)expr->type);
        }
    }
}

Declaration *Factory::make_declaration()
{
    
    void *p = ARENA_ALLOC(sizeof(Declaration));
    return new (p) Declaration();
}

void Factory::free_declaration(Declaration *expr)
{
   // INFO("Free  declaration");
    expr->~Declaration();
    ARENA_FREE(expr, sizeof(Declaration));
}

Variable *Factory::make_variable()
{
    void *p = ARENA_ALLOC(sizeof(Variable));
    return new (p) Variable();
}

void Factory::free_variable(Variable *expr)
{
   // INFO("Free  variable");

    expr->~Variable();
    ARENA_FREE(expr, sizeof(Variable));
}

Assign *Factory::make_assign()
{
    void *p = ARENA_ALLOC(sizeof(Assign));
    return new (p) Assign();
}

void Factory::free_assign(Assign *expr)
{
   // INFO("Free  assign");
    //delete_expression(expr->value);
    expr->~Assign();
    ARENA_FREE(expr, sizeof(Assign));
}

IFStmt *Factory::make_if()
{
    void *p = ARENA_ALLOC(sizeof(IFStmt));
    return new (p) IFStmt();
}

void Factory::free_if(IFStmt *expr)
{
   // INFO("Free  if");
    delete_expression(expr->condition);
    delete_statement(expr->then_branch);
    delete_statement(expr->else_branch);
    for (auto &elif : expr->elifBranch)
    {
         free_elif(elif);
    }
    expr->~IFStmt();
    ARENA_FREE(expr, sizeof(IFStmt));
}

ElifStmt *Factory::make_elif()
{
    void *p = ARENA_ALLOC(sizeof(ElifStmt));
    return new (p) ElifStmt();
}

void Factory::free_elif(ElifStmt *expr)
{
 //   INFO("Free  elif");
    delete_expression(expr->condition);
    delete_statement(expr->then_branch);
    expr->~ElifStmt();
    ARENA_FREE(expr, sizeof(ElifStmt));
}

WhileStmt *Factory::make_while()
{
    void *p = ARENA_ALLOC(sizeof(WhileStmt));
    return new (p) WhileStmt();
}

void Factory::free_while(WhileStmt *expr)
{
   // INFO("Free  while");
    delete_expression(expr->condition);
    delete_statement(expr->body);
    expr->~WhileStmt();
    ARENA_FREE(expr, sizeof(WhileStmt));
}

DoStmt *Factory::make_do()
{
    void *p = ARENA_ALLOC(sizeof(DoStmt));
    return new (p) DoStmt();
}

void Factory::free_do(DoStmt *expr)
{
  //  INFO("Free  do");
    delete_expression(expr->condition);
    delete_statement(expr->body);
    expr->~DoStmt();
    ARENA_FREE(expr, sizeof(DoStmt));
}

ForStmt *Factory::make_for()
{
    void *p = ARENA_ALLOC(sizeof(ForStmt));
    return new (p) ForStmt();
}

void Factory::free_for(ForStmt *expr)
{
   // INFO("Free  for");
    delete_statement(expr->initializer);
    delete_expression(expr->condition);
    delete_expression(expr->increment);
    delete_statement(expr->body);



    expr->~ForStmt();
    ARENA_FREE(expr, sizeof(ForStmt));
}

FromStmt *Factory::make_from()
{
    void *p = ARENA_ALLOC(sizeof(FromStmt));
    return new (p) FromStmt();
}

void Factory::free_from(FromStmt *expr)
{
    delete_expression(expr->array);
    delete_expression(expr->variable);
    delete_statement(expr->body);
    expr->~FromStmt();
    ARENA_FREE(expr, sizeof(FromStmt));
}

ReturnStmt *Factory::make_return()
{
    void *p = ARENA_ALLOC(sizeof(ReturnStmt));
    return new (p) ReturnStmt();
}

void Factory::free_return(ReturnStmt *expr)
{
   // delete_expression(expr->value);
    expr->~ReturnStmt();
    ARENA_FREE(expr, sizeof(ReturnStmt));
}

BreakStmt *Factory::make_break()
{
    void *p = ARENA_ALLOC(sizeof(BreakStmt));
    return new (p) BreakStmt();
    
}

void Factory::free_break(BreakStmt *expr)
{
    expr->~BreakStmt();
    ARENA_FREE(expr, sizeof(BreakStmt));
}

ContinueStmt *Factory::make_continue()
{
    void *p = ARENA_ALLOC(sizeof(ContinueStmt));
    return new (p) ContinueStmt();
}

void Factory::free_continue(ContinueStmt *expr)
{
    expr->~ContinueStmt();
    ARENA_FREE(expr, sizeof(ContinueStmt));
}

CaseStmt *Factory::make_case()
{
    void *p = ARENA_ALLOC(sizeof(CaseStmt));
    return new (p) CaseStmt();
}

void Factory::free_case(CaseStmt *expr)
{
   /// INFO("Free  case");
    delete_expression(expr->condition);
    delete_statement(expr->body);
    
    expr->~CaseStmt();
    ARENA_FREE(expr, sizeof(CaseStmt));
}

SwitchStmt *Factory::make_switch()
{
    void *p = ARENA_ALLOC(sizeof(SwitchStmt));
    return new (p) SwitchStmt();
}

void Factory::free_switch(SwitchStmt *expr)
{
   // INFO("Free  switch");
    for (auto &caseStmt : expr->cases)
    {
        free_case(caseStmt);
    }
    delete_expression(expr->condition);
    if (expr->defaultBranch!=nullptr)
        delete_statement(expr->defaultBranch);
    
    expr->~SwitchStmt();
    ARENA_FREE(expr, sizeof(SwitchStmt));
}

StructStmt *Factory::make_struct()
{
    void *p = ARENA_ALLOC(sizeof(StructStmt));
    return new (p) StructStmt();
}

void Factory::free_struct(StructStmt *expr)
{
   // INFO("Free  struct declaration %s", expr->name.lexeme.c_str());
    expr->values.clear();
    expr->fields.clear();
    expr->~StructStmt();
    ARENA_FREE(expr, sizeof(StructStmt));
}

ClassStmt *Factory::make_class()
{
    void *p = ARENA_ALLOC(sizeof(ClassStmt));
    return new (p) ClassStmt();
}

void Factory::free_class(ClassStmt *expr)
{
    INFO("Free  class");
    for (auto &field : expr->fields)
    {
      //  delete_expression(field);
    }
    for (auto &method : expr->methods)
    {
       // free_function(method);
    }
    expr->~ClassStmt();
    ARENA_FREE(expr, sizeof(ClassStmt));
}

ArrayStmt *Factory::make_array()
{
    void *p = ARENA_ALLOC(sizeof(ArrayStmt));
    return new (p) ArrayStmt();
}

void Factory::free_array(ArrayStmt *expr)
{
    expr->~ArrayStmt();
    ARENA_FREE(expr, sizeof(ArrayStmt));
}

MapStmt *Factory::make_map()
{
    void *p = ARENA_ALLOC(sizeof(MapStmt));
    return new (p) MapStmt();
}

void Factory::free_map(MapStmt *expr)
{
    expr->~MapStmt();
    ARENA_FREE(expr, sizeof(MapStmt));
}
