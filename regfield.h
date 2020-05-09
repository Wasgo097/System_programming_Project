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
        if(x<1||x>3)
            throw "unknow type in registry";
        _type=x;
    }
    int type()const{
        return _type;
    }
    friend std::ostream& operator<<(std::ostream& os, const RegField& dt);
    explicit operator string()const{
        return _key+"||"+_value_name+"||"+std::to_string(_type)+"||"+_value;
    }
    void reduce_key(){
        string tmp=_key;
        int last_slash=0;
        for(int i=0;i<tmp.length();i++){
            if(tmp[i]=='\\'){
                 last_slash=i;
                 continue;
            }
            if((int)tmp[i]<0||(int)tmp[i]>255)
                break;
        }
        if(last_slash+2<tmp.size())
            tmp.erase(tmp.begin()+last_slash+1,tmp.end());
        _key=tmp;
    }
};
#endif // REGFIELD_H
