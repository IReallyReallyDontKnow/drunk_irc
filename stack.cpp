#include "stack.h"

/**********************/
///tempStack
/**********************/

temp_stack::temp_stack(){
    top = NULL;
    depth_counter = 0;
}

void temp_stack::push(std::string input){
    if(top == NULL){
        top = new temp_stack_part;
        top -> param = input;
        top -> next = NULL;
    }
    else{
        temp_stack_part *nextPart = new temp_stack_part;
        nextPart -> param = input;
        nextPart -> next = top;
        top = nextPart;
    }
    depth_counter++;
}

std::string temp_stack::pop(){
    temp_stack_part *holder;
    std::string output = top -> param;

    holder = top;
    top = top -> next;
    delete(holder);

    return(output);
}

/**********************/
/**********************/
/**********************/


/**********************/
///permStack
/**********************/

perm_stack::perm_stack(){
    top = NULL;
    depth_counter = 0;
}

void perm_stack::push(std::string name,std::string address, int16_t port, std::string nick){
    if(top == NULL){
        top = new perm_stack_part;
        top -> address.index = depth_counter;
        top -> address.name = name.c_str();
        top -> address.address = address.c_str();
        top -> address.port = port;
        top -> address.nick = nick.c_str();
        top -> next = NULL;
    }
    else{
        perm_stack_part *nextPart = new perm_stack_part;
        top -> address.index = depth_counter;
        top -> address.name = name.c_str();
        top -> address.address = address.c_str();
        top -> address.port = port;
        top -> address.nick = nick.c_str();
        nextPart -> next = top;
        top = nextPart;
    }
    depth_counter++;
}

serverAddress perm_stack::pop(){
    perm_stack_part *holder;
    serverAddress output = top -> address;

    holder = top;
    top = top -> next;
    delete(holder);

    return(output);
}

serverAddress perm_stack::read(int index){
    perm_stack_part *reader = top;
    //std::cout << depth_counter << "_" << index << "_";

        //THIS SHIT IS HORRIBLE, I FEEL HORRIBLE.
        //WHY DOES THIS GIVE YOU SEGMENTATION FAULT ?
        //...
        //SHOULD I KILL MY SELF ?
        //...
        //NAAH
        while(reader -> address.index!=index){
            std::cout << reader -> address.index << "_";
            reader = reader -> next;
            /*if(reader -> address.index < 0){
                break;
            }*/
        }
        return(reader -> address);

}

/**********************/
/**********************/
/**********************/
