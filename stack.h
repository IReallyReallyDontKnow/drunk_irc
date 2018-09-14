#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <iostream>
#include "header.h"


struct stackPart{
    stackPart *next;
    std::string param;
};

class Stack{
public:
    Stack();
    void push(std::string);
    std::string pop();
    //int flush();
private:
    stackPart *top;
};

#endif // STACK_H_INCLUDED
