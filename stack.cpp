#include "stack.h"



Stack::Stack(){
    top = NULL;
}

void Stack::push(std::string input){
    if(top == NULL){
        top = new stackPart;
        top -> param = input;
        top -> next = NULL;
    }
    else{
        stackPart *nextPart = new stackPart;
        nextPart -> param = input;
        nextPart -> next = top;
        top = nextPart;
    }
}

std::string Stack::pop(){
    stackPart *holder;
    std::string output=top -> param;

    holder = top;
    top = top -> next;
    delete(holder);

    return(output);
}


/*
int Stack::flush(){
    stackPart *holder;
    int decimalCount = 1;
    int output;
    do{

        holder = top;
        top = top -> next;
        delete(holder);
    }
    while(top != NULL);

    //std::cout << output << "\n";

    return output;
}
*/
