#include "RegField.h"
std::ostream& operator<<(std::ostream& os, const RegField& dt){
    os<<dt._key<<"||"<<dt._value_name<<"||"<<dt._type<<"||"<<dt._value;
    return os;
}
