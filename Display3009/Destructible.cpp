
#include "Destructible.h"

Destructible::Destructible(){
    this->_destroyed = false;
}

void Destructible::Destroy(){}

void Destructible::Mark(){
    this->_destroyed = true;
}

bool Destructible::Marked(){
    return this->_destroyed;
}