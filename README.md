Interpreted Language Implementation with AST

Overview

This project represents my second attempt at implementing an interpreted programming language. Initially, I utilized an Abstract Syntax Tree (AST) to evaluate expressions. However, to accelerate the development process, I relied on smart pointers for memory management, which resulted in a significant performance slowdown.

Recognizing the limitations of the AST-based approach, I decided to explore an alternative implementation using bytecode. Bytecode interpretation is generally superior to AST evaluation in terms of execution speed, but it introduces additional complexity in the interpreter's design.

Intrigued by the possibility of improving the performance of an AST-based interpreter, I revisited my initial approach. This time, instead of using smart pointers, I implemented an arena-based memory management system, which provided a more efficient way to handle memory allocation and deallocation.

This repository contains the results of my exploration and experimentation with these different approaches.