#include "pch.h"

#include "Interpreter.hpp"
#include "Utils.hpp"
#include "Factory.hpp"


class BreakException : public std::runtime_error
{
public:
    explicit BreakException(std::string message) : std::runtime_error(message) {}
};

class ContinueException : public std::runtime_error
{
public:
    explicit ContinueException(std::string message) : std::runtime_error(message) {}
};



class ReturnException : public std::runtime_error
{
public:
    explicit ReturnException(Expr *value) : std::runtime_error("return"), value(value) {}
    Expr *value {nullptr};
};


Expr *Compiler::visit(Expr *node)
{
    if (!node) return nullptr;
    return node->accept(*this);
}

Expr *Compiler::visit_assign(Assign *node)
{
    if (!node) return nullptr;
    Expr *value = evaluate(node->value);


    Environment *environment = currentEnvironment();

    if (!environment->assign(node->name.lexeme, value))
    {
      // throw FatalException("Undefined variable: '" + node->name.lexeme +"' at line  "+ std::to_string(node->name.line )+" or mixe types.");
    }

    return value;
}

Expr *Compiler::evaluate(Expr *node)
{
    Expr * result = visit(node);
    if (result == nullptr)
    {
        WARNING("Evaluation error: Unknown expression type");
                return Factory::as().make_empty();
    }
    return result;
}


Expr *Compiler::visit_call_native(CallExpr *node)
{
   // INFO("CALL: %s ",node->name.lexeme.c_str());

    interpreter->context->literals.clear();
    for (u32 i = 0; i < node->args.size(); i++)
    {
        Expr *arg = evaluate(node->args[i]);
        Literal *l = static_cast<Literal *>(arg);
        interpreter->context->literals.push_back(l);
    }
    Expr  *result = interpreter->CallNativeFunction(node->name.lexeme,(int) node->args.size());
    interpreter->context->literals.clear();
    return result;
}
Expr *Compiler::visit_call_struct(CallExpr *node,Expr *expr) 
{

 //   INFO("CREATE STRUCT: %s ",node->name.lexeme.c_str());
    Expr *result = global->get(node->name.lexeme);
    if (!result)
    { 
        WARNING("Undefined struct: '%s'", node->name.lexeme.c_str());
        return Factory::as().make_empty();
    }
    StructLiteral *s = static_cast<StructLiteral *>(result);

    u32 index = 0;

    if (!node->args.empty())
    {
        if (node->args.size() > s->members.size())
        {
            WARNING("Too many arguments in struct call: '%s'", node->name.lexeme.c_str());
        }
        for (auto it = s->members.begin(); it != s->members.end(); it++)
        {
            if (index < node->args.size())
            {
                Expr *arg = evaluate(node->args[index]);

                it->second = std::move(arg);
            }
            index++;
        }
    }

    return s;
}

Expr *Compiler::visit_call_function(CallExpr *node,Expr *callee) 
{
    Function *function = static_cast<Function *>(callee);
    if (function->arity != node->args.size())
    {
        throw FatalException("Incorrect number of arguments in call to '" + node->name.lexeme +"' at line "+ std::to_string(node->name.line )+ " expected " + std::to_string(function->arity) + " but got " + std::to_string(node->args.size()));
    }

    enterBlock();
    Environment *environment = currentEnvironment();
    for (u32 i = 0; i < node->args.size(); i++)
    {
        Expr *arg = evaluate(node->args[i]);
        environment->define(function->args[i], std::move(arg));
    }
    Expr *result = nullptr;
    try  
    {
        BlockStmt *body = static_cast<BlockStmt *>(function->body);
        for (auto stmt : body->statements)
        {
            execute(stmt);
        }
        
    }
    catch (ReturnException &e)
    {
        result = e.value;
    }  
    exitBlock();
    Factory::as().delete_expression(callee);
    if (result == nullptr)
    {
        result = Factory::as().make_literal();
    }

      while (environmentStack.size() > 1)
    {
        Environment *env = environmentStack.top();
        Factory::as().free_environment(env);
        environmentStack.pop();
    }

    return result;
}


Expr *Compiler::visit_call(CallExpr *node)
{

    if (!node)   return Factory::as().make_empty();
          
    Expr *callee = evaluate(node->callee);
    Factory::as().delete_expression(node->callee);

     Expr* var = global->get(node->name.lexeme);
    if (var->type == ExprType::L_STRUCT)
    {
        visit_call_struct(node, callee);
    }

    if (var->type == ExprType::L_NATIVE)
    {
        return visit_call_native(node);
    }

    //  INFO("CALL: %s %s", var->toString().c_str(),node->name.lexeme.c_str());

    if (var->type != ExprType::L_FUNCTION)
        return callee;

    return visit_call_function(node, callee);
} 

std::string toLower(const std::string &str)
{
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}


Expr *Compiler::ProcessArray(Expr* var, GetDefinitionExpr *node)
{
        ArrayLiteral *earray = static_cast<ArrayLiteral *>(var);
        Expr* expr    = global->get(earray->name);
        if (!expr)  
        {
           ERROR("Array '%s' not found: " ,earray->name.c_str());
           return var;
        }
        ArrayLiteral *array = static_cast<ArrayLiteral *>(expr);
        std::string action = toLower(node->name.lexeme);


        if (action == "push")
        {
                if (node->values.size() < 1)
                {
                    throw FatalException("Array 'push' requires 1 or more argument");
                }
                for (u32 i = 0; i < node->values.size(); i++)
                {
                    Expr *value = evaluate(node->values[i]);
                    array->values.push_back(value->clone());
                }
                 Factory::as().free_get_definition(node);
                return array;
  
        } else if (action == "pop")
        {
                Expr* value = array->values.back();
                array->values.pop_back();
                 Factory::as().free_get_definition(node);
                return value;
        } else if (action == "size")
        {
            NumberLiteral *result = Factory::as().make_number();
            result->value = array->values.size();
            return result;
        }
        else if (action == "at")
        {
            if (node->values.size() != 1)
            {
                ERROR("Array 'at' requires 1 argument");
                return var;
            }
            Expr *value = evaluate(node->values[0]);
            if (value->type != ExprType::L_NUMBER)
            {
                ERROR("Array index must be a number");
                return var;
            }
            NumberLiteral *nl = static_cast<NumberLiteral *>(value);
            if (nl->value < 0 || nl->value >= array->values.size())
            {
                ERROR("Array index out of bounds");
                return var;
            }

            u32 index = (u32) static_cast<int>(nl->value);
             Factory::as().free_get_definition(node);
           return array->values[index];
        }else  if (action == "set")
        {
            if (node->values.size() != 2)
            {
                Factory::as().free_get_definition(node);
                throw FatalException("Array 'set' requires 2 arguments");
            }
            Expr *value = evaluate(node->values[0]);
            if (value->type != ExprType::L_NUMBER)
            {
                Factory::as().free_get_definition(node);
                throw FatalException("Array index must be a number");
            }
            NumberLiteral *nl = static_cast<NumberLiteral *>(value);
            if (nl->value < 0 || nl->value >= array->values.size())
            {
                Factory::as().free_get_definition(node);
                throw FatalException("Array index out of bounds");
            }

            int index = nl->value;

            array->values[index] = value;
            Factory::as().free_get_definition(node);

            return var;
        }else if (action == "remove")
        {
            if (node->values.size() != 1)
            {
                Factory::as().free_get_definition(node);
                throw FatalException("Array 'remove' requires 1 argument");
            }
            Expr *value = evaluate(node->values[0]);
            if (value->type != ExprType::L_NUMBER)
            {
                Factory::as().free_get_definition(node);
                throw FatalException("Array index must be a number");
            }
            NumberLiteral *nl = static_cast<NumberLiteral *>(value);
            if (nl->value < 0 || nl->value >= array->values.size())
            {
                Factory::as().free_get_definition(node);
                throw FatalException("Array index out of bounds");
            }

            int index = nl->value;

            auto v =    array->values.erase(array->values.begin() + index);
            Expr *item = *v;
            //Factory::as().delete_expression(item);
             Factory::as().free_get_definition(node);
            return item;
            
        } else if (action == "clear")
        {
            while(array->values.empty() == false)
            {
               Expr *value = array->values.back();
               array->values.pop_back();
               Factory::as().delete_expression(value);
            }
            Factory::as().free_get_definition(node);
            return var;
        }
        else if (action == "foreach")
        {
                if (node->values.size() < 1)
                {
                    throw FatalException("Array 'foreach' requires 1 function argument");
                }
                Expr *value = evaluate(node->values[0]);
                if (value->type != ExprType::L_FUNCTION)
                {
                    Factory::as().free_get_definition(node);
                    throw FatalException("Array 'foreach' requires 1 function argument");
                }
                
                Function *fl = static_cast<Function *>(value);
             //   INFO("Function: %s", fl->name.lexeme.c_str());
                for (u32 i = 0; i < array->values.size(); i++)
                {
                      CallExpr *call = Factory::as().make_call();
                      call->name = node->name;
                      call->callee = value;
                      call->args.push_back(array->values[i]);
                      visit_call_function(call, value);
                      Factory::as().free_call(call);
                    
                }
                 Factory::as().free_get_definition(node);
            return var;
        }
        else 
        {
             WARNING("Unknown array function: %s", node->name.lexeme.c_str());
        }
    
    return var;
}

Expr *Compiler::ProcessMap(Expr *var, GetDefinitionExpr *node)
{
        MapLiteral *emap = static_cast<MapLiteral *>(var);
        Expr* expr    = global->get(emap->name);
        if (!expr)  
        {
           ERROR("Dictionary '%s' not found: " ,emap->name.c_str());
           return var;
        }

        MapLiteral *map = static_cast<MapLiteral *>(expr);
        std::string action = toLower(node->name.lexeme);


        if (action == "erase")
        {
            if (node->values.size() != 1)
            {
                Factory::as().free_get_definition(node);
                WARNING("Dictionary 'erase' requires 1 arguments");
                return var;
            }
            Expr *find = evaluate(node->values[0]);
            if (find->type==ExprType::L_STRING)
            {
                StringLiteral *sl = static_cast<StringLiteral *>(find);
                    auto it = map->values.begin();
                    for (; it != map->values.end(); it++)
                    {
                        Expr *first = it->first;
                        if (first->type == ExprType::L_STRING)
                        {
                            StringLiteral *key = static_cast<StringLiteral *>(first);
                            if (key->value == sl->value)
                            {
                                Expr *value = it->second;
                                map->values.erase(it);
                                return value;
                            }
                        }
                    }
                
                WARNING("Key not found: %s", sl->value.c_str());
            } else if (find->type == ExprType::L_NUMBER)
            {
                NumberLiteral *nl = static_cast<NumberLiteral *>(find);

                auto it = map->values.begin();
                for (; it != map->values.end(); it++)   
                {
                    Expr *first = it->first;
                    if (first->type == ExprType::L_NUMBER)
                    {
                        NumberLiteral *key = static_cast<NumberLiteral *>(first);
                        if (key->value == nl->value)
                        {
                            Expr *value = it->second;
                            map->values.erase(it);
                            return value;
                        }
                    }
                }
                WARNING("Key not found: %f", nl->value);
            }
           

            Factory::as().free_get_definition(node);
            return Factory::as().make_literal();            
        } else if (action == "size")
        {
            NumberLiteral *result = Factory::as().make_number();
            result->value = map->values.size();
            Factory::as().free_get_definition(node);
            return result;
        } else if (action == "set")
        {
            if (node->values.size() != 2)
            {
                Factory::as().free_get_definition(node);
                WARNING("Dictionary 'set' requires 2 arguments");
                return var;
            }
            Expr *key   = evaluate(node->values[0]);
            Expr *value = evaluate(node->values[1]);
            Expr *remove = map->values[key];
            if (remove)
            {
                Factory::as().delete_expression(remove);
            }
            map->values[key] = value;
            Factory::as().free_get_definition(node);
            return var;
        }
        else if (action == "find")
        {
            if (node->values.size() != 1)
            {
                Factory::as().free_get_definition(node);
                WARNING("Dictionary 'find' requires 1 arguments");
                return var;
            }
            Expr *find   = evaluate(node->values[0]);
            Factory::as().free_get_definition(node);


            if (find->type==ExprType::L_STRING)
            {
                StringLiteral *sl = static_cast<StringLiteral *>(find);
                    auto it = map->values.begin();
                    for (; it != map->values.end(); it++)
                    {
                        Expr *first = it->first;
                        if (first->type == ExprType::L_STRING)
                        {
                            StringLiteral *key = static_cast<StringLiteral *>(first);
                            if (key->value == sl->value)
                            {
                                Expr *value = it->second;
                                return value;
                            }
                        }
                    }
                
                WARNING("Key not found: %s", sl->value.c_str());
            } else if (find->type == ExprType::L_NUMBER)
            {
                NumberLiteral *nl = static_cast<NumberLiteral *>(find);

                auto it = map->values.begin();
                for (; it != map->values.end(); it++)   
                {
                    Expr *first = it->first;
                    if (first->type == ExprType::L_NUMBER)
                    {
                        NumberLiteral *key = static_cast<NumberLiteral *>(first);
                        if (key->value == nl->value)
                        {
                            Expr *value = it->second;
                            return value;
                        }
                    }
                }
                WARNING("Key not found: %f", nl->value);
            }
        


        return Factory::as().make_literal();

        }
        else if (action == "clear")
        {
           auto it = map->values.begin();
            for (; it != map->values.end(); it++)
            {
                Expr *key   = it->first;
                Expr *value = it->second;
             //   Factory::as().delete_expression(value);
             //   Factory::as().delete_expression(key);
            }
            map->values.clear();
            Factory::as().free_get_definition(node);
            return var;
        }
        else if (action == "foreach")
        {
                if (node->values.size() < 1)
                {
                    throw FatalException("Dictionary 'foreach' requires 1 function argument");
                }
                Expr *value = evaluate(node->values[0]);
                if (value->type != ExprType::L_FUNCTION)
                {
                    Factory::as().free_get_definition(node);
                    throw FatalException("Dictionary 'foreach' requires 1 function argument");
                }
                
               Function *fl = static_cast<Function *>(value);
               INFO("Function: %s", fl->name.lexeme.c_str());
                auto it = map->values.begin();
            for (; it != map->values.end(); it++)
            {
                Expr *key   = it->first;
                Expr *v = it->second;

              
                CallExpr *call = Factory::as().make_call();
                call->name = node->name;
                call->callee = value;
                call->args.push_back(key);
                call->args.push_back(v);
                visit_call_function(call, value);
                Factory::as().free_call(call);
                    
                }
            Factory::as().free_get_definition(node);
            return var;
        }
        else 
        {
             WARNING("Unknown dictionary function: %s", node->name.lexeme.c_str());
        }
    
    
    return var;
}
    

Expr *Compiler::visit_get_definition(GetDefinitionExpr *node)
{

    //  INFO("GET Built int defenition: %s ", node->name.lexeme.c_str());
 

    Expr *var     = evaluate(node->variable);    

    if (var->type == ExprType::L_ARRAY)
    {
       return ProcessArray(var, node);
    } else if (var->type == ExprType::L_MAP)
    {
        return ProcessMap(var, node);
    }

    Factory::as().free_get_definition(node);
    return var;
}

Expr *Compiler::visit_get(GetExpr *node)
{
  //  INFO("GET arg: %s", node->name.lexeme.c_str());
    Expr *object = evaluate(node->object);
    if (object->type==ExprType::L_STRUCT)
    {
        StructLiteral *sl = static_cast<StructLiteral *>(object);

        if (sl->members.find(node->name.lexeme) != sl->members.end())
        {
            Expr *value = sl->members[node->name.lexeme];
    
            return value;
        } else 
        {
            ERROR("Member not found: %s", node->name.lexeme.c_str());
            return Factory::as().make_empty();
        }
    } else if (object->type == ExprType::L_ARRAY)
    {
        WARNING("Array GET: %s", node->name.lexeme.c_str());
    }
    return  object;
}


Expr *Compiler::visit_set(SetExpr *node)
{

   // INFO("SET arg: %s", node->name.lexeme.c_str());
    Expr *object = evaluate(node->object);
    if (object->type==ExprType::L_STRUCT)
    {
        StructLiteral *sl = static_cast<StructLiteral *>(object);

        if (sl->members.find(node->name.lexeme) != sl->members.end())
        {

                Expr *value = evaluate(node->value);
                if (value->type == ExprType::L_NUMBER)
                {
                    NumberLiteral *l = Factory::as().make_number();
                    l->value = static_cast<NumberLiteral *>(value)->value;
                    sl->members[node->name.lexeme] = l;
                } else if (value->type == ExprType::L_STRING)
                {
                    StringLiteral *l = Factory::as().make_string();
                    l->value = static_cast<StringLiteral *>(value)->value;
                    sl->members[node->name.lexeme] = l;
                } else
                 {
                    WARNING("TODO set value type: %s", value->toString().c_str());
                }
        }
    }    
    return object;
}

Expr *Compiler::visit_now_expression(NowExpr *node)
{
    Factory::as().free_now(node);
    NumberLiteral *result = Factory::as().make_number();
    result->value = time_now();
    return result;
}



u8 Compiler::execute(Stmt *stmt)
{
    if (!stmt)
    {
        return 0;
    }
    return stmt->visit(*this);
}




void Compiler::enterBlock()
{

    Environment *env = Factory::as().make_environment(environmentStack.top());
    environmentStack.push(env);
}

Environment *Compiler::enterLocal( Environment *parent)
{

    Environment *env = Factory::as().make_environment(parent);
    environmentStack.push(env);
    return env;
}


void Compiler::exitBlock()
{
        if (environmentStack.size() > 1) 
        {
            Environment *env = environmentStack.top();
            Factory::as().free_environment(env);
            environmentStack.pop();
          
        }
}

Environment *Compiler::currentEnvironment()
{
    return environmentStack.top();
}

Environment *Compiler::globalEnvironment()
{
    return global;
}



void Compiler::clear()
{
    INFO("Compiler clear");
    interpreter = nullptr;
    while (environmentStack.size() > 0)
    {
        Environment *env = environmentStack.top();
        Factory::as().free_environment(env);
        environmentStack.pop();
    }
    for (u32 i = 0; i < structs.size(); i++)
    {
      Factory::as().deleteStruct(structs[i]);
    }
    structs.clear();
    for (u32 i = 0; i < arrays.size(); i++)
    {
      Factory::as().delete_array(arrays[i]);
    }
    arrays.clear();
    for (u32 i = 0; i < maps.size(); i++)
    {
      Factory::as().delete_map(maps[i]);
    }
    maps.clear();
    for(u32 i = 0; i < functions.size(); i++)
    {
      Factory::as().deleteFunction(functions[i]);
    }
    functionMap.clear();
    functions.clear();
}

u8 Compiler::visit_block_smt(BlockStmt *node)
{
    if (!node) return  0;


    enterBlock();
    u8 result = 0;

   
    for (Stmt *s : node->statements)
    {
        result |=  execute(s);
    }



    
    
   exitBlock();



    return result;

}

u8 Compiler::visit_expression_smt(ExpressionStmt *node)
{
    if (node->expression == nullptr)
    {
        throw FatalException("[EXPRESSION STATEMENT] Unknown expression type");
    }
    evaluate(node->expression);

    return 0;
}



u8 Compiler::visit_print_smt(PrintStmt *node)
{
    if (!node) return  0;

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
        nl->print();
         
    }
    else if (result->type == ExprType::L_STRING)
    {
        StringLiteral *sl = static_cast<StringLiteral *>(result);
        sl->print();
    } else if (result->type == ExprType::LITERAL)
    {
        PRINT("nil");
    } else if (result->type == ExprType::L_STRUCT)
    {
        StructLiteral *sl = static_cast<StructLiteral *>(result);
        sl->print();

    } else if (result->type == ExprType::L_ARRAY)
    {
        ArrayLiteral *al = static_cast<ArrayLiteral *>(result);
        al->print();
    } else if (result->type == ExprType::L_MAP)
    {
        MapLiteral *ml = static_cast<MapLiteral *>(result);
        ml->print();
    }
    
    else 
    {
       WARNING("[PRINT] Unknown literal type %s", result->toString().c_str());
    }
  return 0;
}


Expr *Compiler::visit_variable(Variable *node)
{
   // INFO("READ Variable: %s", node->name.lexeme.c_str());
    Environment *environment = currentEnvironment();


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

u8 Compiler::visit_declaration(Declaration *node)
{

     
    Environment *environment = currentEnvironment();
   

       

        if (node->is_initialized)
        {


                Token name = node->names[0];
                // if (environment->contains(name.lexeme))
                // {
                //     throw FatalException("Variable already defined: " + name.lexeme +" at line "+ std::to_string(name.line ));
                // }
            Expr * value = evaluate(node->initializer);
            environment->define(name.lexeme, value);

            for (u32 i = 1; i < node->names.size(); i++)
            {
                    Token name = node->names[i];
                    if (environment->contains(name.lexeme))
                    {
                        throw FatalException("Variable already defined: " + name.lexeme +" at line "+ std::to_string(name.line ));
                    }


                if (value->type == ExprType::L_NUMBER)
                {
                    NumberLiteral *nl = static_cast<NumberLiteral *>(value);
                    NumberLiteral *nLiteral = Factory::as().make_number();
                    nLiteral->value = nl->value;
                    environment->define(node->names[i].lexeme, nLiteral);
                } else if (value->type == ExprType::L_STRING)
                {
                    StringLiteral *sl = static_cast<StringLiteral *>(value);
                    StringLiteral *sLiteral = Factory::as().make_string();
                    sLiteral->value = sl->value;
                    environment->define(node->names[i].lexeme, sLiteral);
                }
               
            }
        } else 
        {
            for (u32 i = 0; i < node->names.size(); i++)
            {
                 Token name = node->names[i];
                if (environment->contains(name.lexeme))
                {
                    throw FatalException("Variable already defined: " + name.lexeme +" at line "+ std::to_string(name.line ));
                }
                Literal *value = Factory::as().make_literal();
                environment->define(name.lexeme, value);
            }
        }


    return 0;
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

static bool is_equal(Expr *a, Expr *b)
{
    if (a == nullptr && b == nullptr) return true;
    if (a->type != b->type) return false;
    if (a->type == ExprType::L_NUMBER && b->type == ExprType::L_NUMBER)
    {
        NumberLiteral *nl = static_cast<NumberLiteral *>(a);
        NumberLiteral *nl2 = static_cast<NumberLiteral *>(b);
        return nl->value == nl2->value;
    }
    if (a->type == ExprType::L_STRING && b->type == ExprType::L_STRING)
    {
        StringLiteral *sl = static_cast<StringLiteral *>(a);
        StringLiteral *sl2 = static_cast<StringLiteral *>(b);
        return sl->value == sl2->value;
    }
    return true;
}

u8 Compiler::visit_if(IFStmt *node)
{
    Expr *condition = evaluate(node->condition);
    if (condition == nullptr)
    {
       WARNING("Invalid condition");
       return 0;
    }
    if (is_truthy(condition))
    {
        return  execute(node->then_branch);
    }
    
    for (auto elif : node->elifBranch)
    {
        if (is_truthy(evaluate(elif->condition)))
        {
            return execute(elif->then_branch);
        }
    }
    
    if (node->else_branch != nullptr)
    {
        return execute(node->else_branch);
    }

    return 0;

}

u8 Compiler::visit_while(WhileStmt *node)
{


    loop_count++;


    while (true)
    {
      
            auto condition = evaluate(node->condition);
            if (!is_truthy(condition))
            {
                Factory::as().delete_expression(condition);
                break;
            }





    try  
    {

       
            execute(node->body);
        
    }
        catch (BreakException &e)
        {
          //  break;
        }

        catch (ContinueException e)
        {
        }
      
    }

    loop_count--;


    while (environmentStack.size() > 1)
    {
        Environment *env = environmentStack.top();
        Factory::as().free_environment(env);
        environmentStack.pop();
    }


    return 0;
}


u8 Compiler::visit_do(DoStmt *node)
{
    loop_count++;

    do 
    {
        try 
        {
            execute(node->body);
        }
        catch (BreakException &e)
        {
            break;
        }
        catch (ContinueException e)
        {
        }
    } while (is_truthy(evaluate(node->condition)));
    loop_count--;

    while (environmentStack.size() > 1)
    {
        Environment *env = environmentStack.top();
        Factory::as().free_environment(env);
        environmentStack.pop();
    }

    return 0;
}


u8 Compiler::visit_program(Program *node)
{
    
    for (Stmt *s : node->statements)
    {
        execute(s);
    }
    Factory::as().free_program(node);
    clear();



    return 0;
}

u8 Compiler::visit_function(FunctionStmt *node)
{
    if (functionMap.find(node->name.lexeme) != functionMap.end())
    {
        throw FatalException("Function '" + node->name.lexeme + "' already defined .");
    }
  
    Function *function = Factory::as().createFunction();

    functions.push_back(function);

    
    functionMap[node->name.lexeme] = node;


    

    //function->environment = Factory::as().make_environment(global);


    function->name = node->name;
    function->arity = node->args.size();

    for (u32 i = 0; i < node->args.size(); i++)
    {
        function->args[i]=node->args[i];
    }
    function->body = node->body;
    
    
    global->define(function->name.lexeme, function);



    return 0;
}

u8 Compiler::visit_struct(StructStmt *node)
{
   // PRINT("VISIT Struct: %s", node->name.lexeme.c_str());

    StructLiteral *sl = Factory::as().createStruct();
    sl->name = node->name.lexeme;

     for (u32 i = 0; i < node->fields.size(); i++)
    {
      //  PRINT("Field: %s", node->fields[i].lexeme.c_str());
        Expr *expr = evaluate(node->values[i]);
        sl->members[node->fields[i].lexeme] = expr;
    }

    // for (u32 i = 0; i < node->methods.size(); i++)
    // {
    //     functionMap[node->methods[i].name.lexeme] = &node->methods[i];
    // }
    // node->args

    
    if (global->define(node->name.lexeme, sl))
    {
        structs.push_back(sl); 
    }

     

    return 0;
}

u8 Compiler::visit_class(ClassStmt *node)
{
    return 0;
}

u8 Compiler::visit_array(ArrayStmt *node)
{
    INFO("Visit array: %s", node->name.lexeme.c_str());
    
    ArrayLiteral *al = Factory::as().create_array();
    arrays.push_back(al);
    al->name = node->name.lexeme;
    Environment *environment = currentEnvironment();
    if (environment->define(node->name.lexeme, al))
    {
        for (u32 i = 0; i < node->values.size(); i++)
        {
            Expr *expr = evaluate(node->values[i]);
            al->values.push_back(expr);
        }
    }
    return 0;
}

u8 Compiler::visit_map(MapStmt *node)
{
    INFO("Visit map: %s", node->name.lexeme.c_str());
    MapLiteral *ml = Factory::as().create_map();
    maps.push_back(ml);
    ml->name = node->name.lexeme;
    Environment *environment = currentEnvironment();
    if (environment->define(node->name.lexeme, ml))
    {
        auto it = node->values.begin();
        for (; it != node->values.end(); it++)
        {
            Expr *key  = evaluate(it->first);
            if (key->type != ExprType::L_STRING && key->type != ExprType::L_NUMBER)
            {
                ERROR("Map key must be a string or number.");
                return 0;
            }

            Expr *expr = evaluate(it->second);
            if (key->type == ExprType::L_STRING)
            {
                StringLiteral *sl = static_cast<StringLiteral *>(key);
                ml->values[sl] = expr;
            } else if (key->type == ExprType::L_NUMBER)
            {
                NumberLiteral *nl = static_cast<NumberLiteral *>(key);
                ml->values[nl] = expr;
            }
        }
    }
    return 0;
}

u8 Compiler::visit_for(ForStmt *node)
{

     enterBlock();

     execute(node->initializer);

    loop_count++;
 

    while (true)
    {
      
            auto condition = evaluate(node->condition);
            if (!is_truthy(condition))
                break;
           Factory::as().delete_expression(condition);

        try 
        {
            execute(node->body);
        }
        catch (BreakException &e)
        {
            break;
        }

        catch (ContinueException e)
        {
        }
        

      
         
        evaluate(node->increment);
      }

    loop_count--;
        


    exitBlock();


      while (environmentStack.size() > 1)
    {
        Environment *env = environmentStack.top();
        Factory::as().free_environment(env);
        environmentStack.pop();
    }


    return 0;
}

u8 Compiler::visit_from(FromStmt *node)
{

    

    loop_count++;
    Expr * array = evaluate(node->array);
    if (!array || array->type != ExprType::L_ARRAY)
    {
        ERROR("Expected array to iterate");
        return 0;
    }
    ArrayLiteral *al = static_cast<ArrayLiteral *>(array);
    enterBlock();
    Variable *varNode = static_cast<Variable *>(node->variable);
    if (!varNode)
    {
        ERROR("Expected variable to iterate");
        return 0;
    }

    std::string name = varNode->name.lexeme;

    Environment *env = environmentStack.top();
    env->define(name, Factory::as().make_literal());

    //Expr *var = evaluate(node->variable);

    


    for (u32 i = 0; i < al->values.size(); i++)
    {
      
       
        Expr *value = al->values[i];
        env->set(name, value);
        
            
        try 
        {
            execute(node->body);
        }
        catch (BreakException &e)
        {
            break;
        }

        catch (ContinueException e)
        {
        }
        

      
         
        
      }

    loop_count--;
        


    exitBlock();


    while (environmentStack.size() > 1)
    {
        Environment *env = environmentStack.top();
        Factory::as().free_environment(env);
        environmentStack.pop();
    }

    return 0;
}

u8 Compiler::visit_return(ReturnStmt *node)
{
    Expr *value = evaluate(node->value);

  //  NumberLiteral *num = static_cast<NumberLiteral *>(value);
 //   INFO("Return: %f", num->value);
    throw ReturnException(value);
    return 3;
}

u8 Compiler::visit_break(BreakStmt *node)
{
    if (loop_count == 0)
    {
       WARNING("BREAK outside of loop");
       return 0;
    }
    throw BreakException("BREAK");
    return 1;
}

u8 Compiler::visit_continue(ContinueStmt *node)
{
    if (loop_count == 0)
    {
       WARNING("CONTINUE outside of loop");
       return 0;
    }
   // Factory::as().free_continue(node);
    throw ContinueException("CONTINUE");
    return 2;
}

u8 Compiler::visit_switch(SwitchStmt *stmt)
{
    if (!stmt)
    {
           throw FatalException("invalid switch expression");
    }

    auto condition = evaluate(stmt->condition);
    if (!condition)
    {
           throw FatalException("invalid switch condition");
    }
 

    for (const auto &caseStmt : stmt->cases)
    {

        auto result = evaluate(caseStmt->condition);
        if (!result)
        {
               throw FatalException("invalid case condition");
        }
      
        if (is_equal(condition, result))
        {
            return execute(caseStmt->body);
        }
    }
    if (stmt->defaultBranch != nullptr)
    {
       return  execute(stmt->defaultBranch);
    }

    return 0;
}

Compiler::Compiler(Interpreter *i, Compiler *c)
{
    interpreter = i;
    parent = c;
    if (parent != nullptr)
        global = Factory::as().make_environment(parent->global);
    else
        global = Factory::as().make_environment(nullptr);

    environmentStack.push(global);
    

    
}

Compiler::~Compiler()
{
    
}



Expr *Compiler::visit_empty_expression(Expr *node)
{
    return Factory::as().make_literal();
}

Expr *Compiler::visit_binary(BinaryExpr *node)
{
    Expr *left  = evaluate(node->left);
    if(!left)
        return Factory::as().make_empty();
    Expr *right = evaluate(node->right);
    if (!right)
        return Factory::as().make_empty();
  
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

    return Factory::as().make_empty();
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
   //Literal *result = Factory::as().make_literal();
    //NumberLiteral *result = Factory::as().make_number();
   // result->value = 0;
   // Factory::as().free_literal(node);
    return node;
}

Expr *Compiler::visit_number_literal(NumberLiteral *node)
{
   
    NumberLiteral *result = Factory::as().make_number();
    result->value = node->value;
    return result;
   // return node;
}

Expr *Compiler::visit_string_literal(StringLiteral *node)
{
   // INFO("String: %s", node->value.c_str());
   // StringLiteral *result = Factory::as().make_string();
   // result->value = node->value;
   // return result;
    return node;
}

Interpreter::~Interpreter()
{


    
}   

Interpreter::Interpreter()
{
    compiler = new Compiler(this);
    context = new Context(this);
   
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

    // for (auto token : tokens)
    // {
    //     INFO("Token: %s" ,token.toString().c_str());
    // }

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
        parser.clear();
        
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
        compiler->clear();
        delete compiler;
        compiler = nullptr;
    }

    if (context != nullptr)
    {
        delete context;
        context = nullptr;
    }

    nativeFunctions.clear();
   
    INFO("Factory destroyed");
    Factory::as().clear();
}

void Interpreter::registerFunction(const std::string &name, NativeFunction function)
{
    if (nativeFunctions.find(name) != nativeFunctions.end())
    {
        throw FatalException("Native function already defined: " + name);
    }
    compiler->global->define(name, Factory::as().createNative());
    nativeFunctions[name] = function;
}

bool Interpreter::isnative(const std::string &name)
{
    return nativeFunctions.find(name) != nativeFunctions.end();
}

Literal *Interpreter::CallNativeFunction(const std::string &name, int argc)
{
    auto function = nativeFunctions[name];
    Literal *result = function(context, argc);
    return result;
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

static u32 env_depth = 0;

Environment::Environment()
{

    parent = nullptr;
    depth = ++env_depth;
}

Environment::Environment(Environment *parent)
{

    depth = ++env_depth;
    this->parent = parent;
 //  INFO("Environment created %d", depth);
}

Environment::~Environment()
{

    // for (auto it = m_values.begin(); it != m_values.end(); it++)
    // {
    //     Expr *l = it->second;
    //     if (l != nullptr)
    //     {
    //         if (l->type == ExprType::LITERAL)
    //         {
    //           //  Factory::as().delete_expression(l);
    //         } else if (l->type == ExprType::L_FUNCTION)
    //         {

    //         } else if (l->type == ExprType::L_CLASS)
    //         {
                
    //         } else if (l->type == ExprType::L_STRUCT)
    //         {
    //             Factory::as().delete_expression(l);
                
    //         }else if (l->type == ExprType::L_STRING)
    //         {
    //           //  Factory::as().delete_expression(l);   
    //         } else if (l->type == ExprType::L_NUMBER)
    //         {
    //           //  Factory::as().delete_expression(l);
    //         }
    //     }
    // }
   // INFO("Environment destroyed %d", depth);
    env_depth--;
}

void Environment::print()
{

    for (auto it = m_values.begin(); it != m_values.end(); it++)
    {
        Expr *l = it->second;
        if (l != nullptr)
        {
            INFO("%s: %s", it->first.c_str(), l->toString().c_str());
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
    Expr *expr = nullptr;
    if (m_values.find(name) != m_values.end())
    {
        expr = m_values[name];
    }else 
    if (parent != nullptr)
    {
        return parent->assign(name, value);
    }

     if (value == nullptr)
    {
        ERROR("Cannot assign variable to undefined value: %s", name.c_str());
        return false;
    }
    if (expr->type == ExprType::LITERAL)
    {
        WARNING("Variable: %s is not initialized", name.c_str());
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

Function::Function():Literal()
{
    type = ExprType::L_FUNCTION;
    body = nullptr;
}

ClassLiteral::ClassLiteral()
{
    type = ExprType::L_CLASS;
    name = "";
}

StructLiteral::StructLiteral()
{
    type = ExprType::L_STRUCT;
    name = "";
}
std::string BuilArray(ArrayLiteral *al);


std::string BuilMap(MapLiteral *ml)
{
    std::string s;
    auto it = ml->values.begin();
    u32 size = ml->values.size();
    for (u32 i = 0; it != ml->values.end(); it++, i++)
    {
        s +="{";
        Expr *key = it->first;
        if (key->type == ExprType::L_NUMBER)
        {
            NumberLiteral *nl = static_cast<NumberLiteral *>(key);
            s += std::to_string(nl->value);
        } else if (key->type == ExprType::L_STRING)
        {
            StringLiteral *sl = static_cast<StringLiteral *>(key);
            s += sl->value;
        }
        s += ":";
        Expr *value = it->second;
        if (value->type == ExprType::L_NUMBER)
        {
            NumberLiteral *nl = static_cast<NumberLiteral *>(value);
            s += std::to_string(nl->value);
        } else if (value->type == ExprType::L_STRING)
        {
            StringLiteral *sl = static_cast<StringLiteral *>(value);
            s += sl->value;
        } else if (value->type == ExprType::L_ARRAY)
        {
            ArrayLiteral *al = static_cast<ArrayLiteral *>(value);
            s += BuilArray(al);
        } else if (value->type == ExprType::L_MAP)
        {
            MapLiteral *ml = static_cast<MapLiteral *>(value);
            s += BuilMap(ml);
        }
        s += "}";
        if (i < size - 1)
        {
            s += ",";
        }

       
    }
    return s;
} 

std::string BuilStruct(StructLiteral *sl)
{
    std::string s=sl->name+ " ";
    auto it = sl->members.begin();
    while (it != sl->members.end())
    {
        std::string name = it->first;
        std::string value;
        Expr *expr = it->second;
        if (expr->type == ExprType::L_NUMBER)
        {
            NumberLiteral *nl = static_cast<NumberLiteral *>(expr);
            value = std::to_string(nl->value);
        } else if (expr->type == ExprType::L_STRING)
        {
            StringLiteral *sl = static_cast<StringLiteral *>(expr);
            value = sl->value;
        } else if (expr->type == ExprType::L_STRUCT)
        {
            StructLiteral *sl = static_cast<StructLiteral *>(expr);
            value = BuilStruct(sl);            
        }else if (expr->type == ExprType::L_ARRAY)
        {
            ArrayLiteral *al = static_cast<ArrayLiteral *>(expr);
            value = BuilArray(al);
        } else if (expr->type == ExprType::L_MAP)
        {
            MapLiteral *ml = static_cast<MapLiteral *>(expr);
            value = BuilMap(ml);
        }
        s  += "("+ name+ ":" + value+")";
        
        it++;
    }    
    return s;
}

std::string BuilArray(ArrayLiteral *al)
{
    std::string s = "[";
    for (u32 i = 0; i < al->values.size(); i++)
    {
        if (i > 0)
        {
            s += ", ";
        }
        if (al->values[i]->type == ExprType::L_NUMBER)
        {
            NumberLiteral *nl = static_cast<NumberLiteral *>(al->values[i]);
            s +=  std::to_string(nl->value);
        } else if (al->values[i]->type == ExprType::L_STRING)
        {
            StringLiteral *sl = static_cast<StringLiteral *>(al->values[i]);
            s += sl->value;
        } else if (al->values[i]->type == ExprType::L_STRUCT)
        {
            StructLiteral *sl = static_cast<StructLiteral *>(al->values[i]);
            s += BuilStruct(sl);
        } else if (al->values[i]->type == ExprType::L_ARRAY)
        {
            ArrayLiteral *al = static_cast<ArrayLiteral *>(al->values[i]);
            s += BuilArray(al);
        } else if (al->values[i]->type == ExprType::L_MAP)
        {
            MapLiteral *ml = static_cast<MapLiteral *>(al->values[i]);
            s += BuilMap(ml);
        }

    }
    s += "]";
    return s;
}

void StructLiteral::print()
{
        std::string s=BuilStruct(this);
        PRINT("%s", s.c_str());
}

Expr *StructLiteral::clone()
{
    StructLiteral *l = Factory::as().createStruct();
    l->name = name;
    for (auto it = members.begin(); it != members.end(); it++)
    {
        l->members[it->first] = it->second->clone();
    }
    return l;
}

ArrayLiteral::ArrayLiteral()
{
    type = ExprType::L_ARRAY;

}

void ArrayLiteral::print()
{
    std::string str = BuilArray(this);
    PRINT("Array %s", str.c_str());
    
}

Expr *ArrayLiteral::clone()
{
    ArrayLiteral *l = Factory::as().create_array();
    l->name = name;
    for (u32 i = 0; i < values.size(); i++)
    {
        l->values.push_back(values[i]->clone());
    }
    return l;
}

MapLiteral::MapLiteral()
{
    type = ExprType::L_MAP;
}

void MapLiteral::print()
{
    std::string str = BuilMap(this);
    PRINT("Map [%s]", str.c_str());
}

Expr *MapLiteral::clone()
{
    MapLiteral *l = Factory::as().create_map();
    for (auto it = values.begin(); it != values.end(); it++)
    {
        l->values[it->first] = it->second->clone();
    }
    return l;
}

Native::Native()
{
    type = ExprType::L_NATIVE;
}

Context::Context(Interpreter *interpreter)
{
    this->interpreter = interpreter;
}

Context::~Context()
{
    clear();
}

void Context::clear()
{
    for (u32 i = 0; i < literals.size(); i++)
    {
       Factory::as().free_literal(literals[i]);
    }
    literals.clear();
}

long Context::getLong(u8 index)
{
    return static_cast<NumberLiteral *>(literals[index])->value;
}

int Context::getInt(u8 index)
{
    return static_cast<NumberLiteral *>(literals[index])->value;
}

double Context::getDouble(u8 index)
{
    return static_cast<NumberLiteral *>(literals[index])->value;
}

float Context::getFloat(u8 index)
{
    return static_cast<NumberLiteral *>(literals[index])->value;
}

std::string Context::getString(u8 index)
{
    return static_cast<StringLiteral *>(literals[index])->value;
}

bool Context::getBoolean(u8 index)
{
    return static_cast<NumberLiteral *>(literals[index])->value != 0;
}

Literal *Context::asFloat(float value)
{
    NumberLiteral *result = Factory::as().make_number();
    result->value = static_cast<double>(value);
    return result;
}

Literal *Context::asDouble(double value)
{
    NumberLiteral *result = Factory::as().make_number();
    result->value = value;
    return result;
}

Literal *Context::asInt(int value)
{
    NumberLiteral *result = Factory::as().make_number();
    result->value = static_cast<double>(value);
    return result;
}

Literal *Context::asLong(long value)
{
    NumberLiteral *result = Factory::as().make_number();
    result->value = static_cast<double>(value);
    return result;
}

Literal *Context::asString(std::string value)
{
    StringLiteral *result = Factory::as().make_string();
    result->value = value;
    return result;
}

Literal *Context::asBoolean(bool value)
{
    NumberLiteral *result = Factory::as().make_number();
    result->value = value ? 1 : 0;
    return result;
}

bool Context::isNumber(u8 index)
{
    if (index >= literals.size())
    {
        return false;
    }
    return literals[index]->type == ExprType::L_NUMBER;
}

bool Context::isString(u8 index)
{
    if (index >= literals.size())
    {
        return false;
    }
    return literals[index]->type == ExprType::L_STRING;
}
