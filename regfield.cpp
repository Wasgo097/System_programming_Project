#include "regfield.h"
std::ostream& operator<<(std::ostream& os, const RegField& dt){
    os<<dt._key<<"||"<<dt._value_name<<"||"<<dt._type<<"||"<<dt._value;
    return os;
}
RegField::RegField(string key, string value_name, string value, int type){
    this->_key=key;
    this->_value_name=value_name;
    this->_value=value;
    this->_type=type;
}
void RegField::reduce_key(){
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
    tmp.erase(tmp.begin()+last_slash,tmp.end());
    _key=tmp;
}
bool RegField::is_valid(){
    //todo regex for all fields 1-9 a-z A-Z ' ' / . - _
    if(_key.length()<6||_value_name=="")return false;
    else{
        std::regex key_regex("HKEY[\\w \\.-_]{1,}");
        std::regex value_regex("[^';]{1,}");
        if(std::regex_search(_key,key_regex)&&std::regex_search(_value_name,value_regex)){
            //qDebug()<<"Matched";
            return true;
        }
        else{
            //qDebug()<<"Not matched";
            return false;
        }
    }
}
