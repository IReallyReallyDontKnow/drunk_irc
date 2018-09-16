#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

/*
struct addressType{
    char* HOSTNAME;
    int8_t IP[4];
};
*/

struct serverAddress{
    int index;
    std::string name;
    std::string address;
    int16_t port;
    std::string nick;
};

#endif // HEADER_H_INCLUDED
