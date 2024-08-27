
#include "pch.h" 


#include <iostream>
#include <fstream>
#include <sstream>

#include "Utils.hpp" 
#include "Interpreter.hpp" 



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

    Lexer lexer;

    std::string code = readFile("main.pc");
    if (code.length() == 0)
    {
        return 0;
    } 

    Interpreter interpreter;
    try 
    {
        interpreter.compile(code);
    }
    catch (FatalException e)
    {
        std::cout <<"Abort "<< e.what() << std::endl;
        interpreter.clear();
        return 0;
    }
    
    interpreter.clear();
    std::cout <<"Exit " << std::endl;
  

    return 0;

  
}

