#ifndef REGFIELD_H
#define REGFIELD_H
#include <string>
#include <iostream>
#include <exception>
#include <regex>
#include <QDebug>
typedef std::string string;
class RegField{
    string _key;
    string _value;
    string _value_name;
    //1 - bin 2 - str 3 - dword
    int _type;
public:
    RegField(string key="",string value_name="",string value="",int type=-1);
    void key(string x){        _key=x;    }
    string key()const{        return _key;    }
    void value(string x){        _value=x;    }
    string value()const{        return _value;    }
    void value_name(string x){       _value_name=x;    }
    string value_name()const{        return _value_name;    }
    void type(int x){
        if(x<1||x>3)
            throw "unknow type in registry";
        _type=x;
    }
    int type()const{        return _type;    }
    void reduce_key();
    bool is_valid();
    explicit operator string()const{return _key+"||"+_value_name+"||"+std::to_string(_type)+"||"+_value;}
    friend std::ostream& operator<<(std::ostream& os, const RegField& dt);
};
#endif // REGFIELD_H
