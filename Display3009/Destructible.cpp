
#include "Destructible.h"

Destructible(){
    this->_destroyed = false;
}

void Mark(){
    this->_destroyed = true;
}

bool Marked(){
    return this->_destroyed;
}