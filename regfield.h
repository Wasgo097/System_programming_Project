#ifndef REGFIELD_H
#define REGFIELD_H
#include <string>
#include <iostream>
#include <exception>
typedef std::string string;
class RegField{
    string _key;
    string _value;
    string _value_name;
    //1 - bin 2 - str 3 - dword
    int _type;
public:
    RegField(string key="",string value_name="",string value="",int type=-1){
        this->_key=key;
        this->_value_name=value_name;
        this->_value=value;
        this->_type=type;
    }
    void key(string x){
        _key=x;
    }
    string key()const{
        return _key;
    }
    void value(string x){
        _value=x;
    }
    string value()const{
        return _value;
    }
    void value_name(string x){
        _value_name=x;
    }
    string value_name()const{
        return _value_name;
    }
    void type(int x){
        //if(x<1||x>3)
            //throw "unknow type in registry";
        _type=x;
    }
    int type()const{
        return _type;
    }
    friend std::ostream& operator<<(std::ostream& os, const RegField& dt);
};
#endif // REGFIELD_H
