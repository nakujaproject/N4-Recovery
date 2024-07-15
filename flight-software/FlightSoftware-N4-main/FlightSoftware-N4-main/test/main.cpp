#include <iostream>
#include "test_class.h"



int main(){
    Test t;
    t.y = 89;

    int h = t.foo();

    std::cout<< h <<std::endl;

    return 0;
}