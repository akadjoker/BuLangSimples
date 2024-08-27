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
    arena.Clear();
}

Environment *Factory::make_environment( Environment *parent)
{
    void *p = stack.Allocate(sizeof(Environment));
    return new (p) Environment(parent);
}



void Factory::free_environment(Environment *expr)
{
    expr->~Environment();
    stack.Free(expr);
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
    for (auto &stmt : expr->statements)
    {
        delete_statement(stmt);
    }
    expr->statements.clear();
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

    expr->statements.clear();
    expr->~Program();
    ARENA_FREE(expr, sizeof(Program));
}

void Factory::delete_statement(Stmt *expr)
{
    if (expr)
    {
        if (expr->type == StmtType::DECLARATION)
        {
            free_declaration(static_cast<Declaration*>(expr));
        } else if (expr->type == StmtType::PRINT)
        {
            free_print(static_cast<PrintStmt*>(expr));
        } else if (expr->type == StmtType::EXPRESSION)
        {
            free_expression(static_cast<ExpressionStmt*>(expr));
        } else if (expr->type == StmtType::BLOCK)
        {
            free_block(static_cast<BlockStmt*>(expr));
        } else if (expr->type == StmtType::IF)
        {
            free_if(static_cast<IFStmt*>(expr));
        } else if (expr->type == StmtType::WHILE)
        {
            free_while(static_cast<WhileStmt*>(expr));
        }
    }
}

void Factory::delete_expression(Expr *expr)
{
    if (expr)
    {
        if (expr->type == ExprType::BINARY)
        {
            free_binary(static_cast<BinaryExpr*>(expr));
        } else if (expr->type == ExprType::UNARY)
        {
            free_unary(static_cast<UnaryExpr*>(expr));
        } else if (expr->type == ExprType::LOGICAL)
        {
            free_logical(static_cast<LogicalExpr*>(expr));
        } else if (expr->type == ExprType::GROUPING)
        {
            free_grouping(static_cast<GroupingExpr*>(expr));
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
            free_now(static_cast<NowExpr*>(expr));
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
    expr->~IFStmt();
    ARENA_FREE(expr, sizeof(IFStmt));
}

WhileStmt *Factory::make_while()
{
    void *p = ARENA_ALLOC(sizeof(WhileStmt));
    return new (p) WhileStmt();
}

void Factory::free_while(WhileStmt *expr)
{
    expr->~WhileStmt();
    ARENA_FREE(expr, sizeof(WhileStmt));
}

ForStmt *Factory::make_for()
{
    void *p = ARENA_ALLOC(sizeof(ForStmt));
    return new (p) ForStmt();
}

void Factory::free_for(ForStmt *expr)
{
    delete_statement(expr->initializer);
    delete_expression(expr->condition);
    delete_expression(expr->increment);
    delete_statement(expr->body);



    expr->~ForStmt();
    ARENA_FREE(expr, sizeof(ForStmt));
}
