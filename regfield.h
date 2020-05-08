#ifndef REGFIELD_H
#define REGFIELD_H
#include <string>
#include <iostream>
typedef std::string string;
class RegField{
    string key;
    string value;
    int type;
public:
    RegField(string key,string value,int type){
        this->key=key;
        this->value=value;
        this->type=type;
    }
    friend std::ostream& operator<<(std::ostream& os, const RegField& dt){
        os<<dt.type<<"||"<<dt.type<<"||"<<dt.value;
        return os;
    }
};
#endif // REGFIELD_H
