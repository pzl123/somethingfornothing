#include "template.h"

#include <iostream>

void AbstractClass::templateMethod()
{
    std::cout << "模板方法开始" << std::endl;
    method1();
    method2();
    std::cout << "模板方法结束" << std::endl;
}

void ConcreteClass::method1()
{
    std::cout << "ConcreteClass: 执行 method1()" << std::endl;
}

void ConcreteClass::method2()
{
    std::cout << "ConcreteClass: 执行 method1()" << std::endl;
}

void template_test()
{
    ConcreteClass obj;
    obj.templateMethod();
}