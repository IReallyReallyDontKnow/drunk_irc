#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <iostream>
#include "header.h"


/**********************/
///tempStack
/**********************/

struct temp_stack_part{
    temp_stack_part *next;
    std::string param;
};

class temp_stack{
public:
    temp_stack();
    void push(std::string);
    std::string pop();
    int depth_counter;
private:
    temp_stack_part *top;
};

/**********************/
/**********************/
/**********************/


/**********************/
///permStack
/**********************/

struct perm_stack_part{
    perm_stack_part *next;
    serverAddress address;
};

class perm_stack{
public:
    perm_stack();
    void push(std::string,std::string,int16_t,std::string);
    serverAddress pop();
    serverAddress read(int);
    int depth_counter;
private:
    perm_stack_part *top;
};

/**********************/
/**********************/
/**********************/

#endif // STACK_H_INCLUDED
