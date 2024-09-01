
#include "pch.h" 


#include <iostream>
#include <fstream>
#include <sstream>

#include "Utils.hpp" 
#include "Interpreter.hpp" 


Literal* native_writeln(Context* ctx, int argc) 
{
    for (int i = 0; i < argc; i++)
    {
            if (ctx->isString(i))
            {
                std::cout <<ctx->getString(i);
            }
            else if (ctx->isNumber(i))
            {
                std::cout << ctx->getDouble(i);
            }
    }
    std::cout<< std::endl;
    return ctx->asBoolean(true);
}



std::string readFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file)
    {
        throw std::runtime_error("Could not open file.");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


int main() 
{

   // Lexer lexer;

    std::string code = readFile("main.pc");
    if (code.length() == 0)
    {
        return 0;
    } 

    Interpreter interpreter;
    interpreter.registerFunction("writeln", native_writeln);
    try 
    {
        interpreter.compile(code);
    }
    catch (FatalException e)
    {
        std::cout <<"Abort "<< e.what() << std::endl;
        
    }
    
    interpreter.clear();
    std::cout <<"Exit " << std::endl;
  

    return 0;

  
}

