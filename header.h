#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

struct addressType{
    char* HOSTNAME;
    int8_t IP[4];
};

struct serverAddress{
    int index;
    char* name;
    addressType address;
    int16_t port;
    char* nick;
};

#endif // HEADER_H_INCLUDED
