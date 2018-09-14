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
    const char* name;
    const char* address;
    int16_t port;
    const char* nick;
};

#endif // HEADER_H_INCLUDED
