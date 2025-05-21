#include "prototype_pattern.h"

void prototype_pattern_test()
{
    Dog* dog = new Twoha("二哈");
    Dog* Eha1 = dog->clone();
    Dog* Eha2 = dog->clone();
    Eha1->play();
    Eha2->play();
    delete dog;
    delete Eha1;
    delete Eha2;
}