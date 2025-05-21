#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

/*
+-----------------------------------+
|        AbstractClass              |
+-----------------------------------+
| + templateMethod(): void          |
+-----------------------------------+
| # method1(): void                 | <== 钩子方法（由子类实现）
| # method2(): void                 |
+-----------------------------------+
             ▲
             |
+-----------------------------------+
|        ConcreteClass              |
+-----------------------------------+
| # method1(): void                 | 实现具体行为
| # method2(): void                 |
+-----------------------------------+
父类定义“不变”的流程逻辑(即 templateMethod())。
子类实现“变化”的细节(即 method1() 和 method2())
*/

class AbstractClass
{
public:
    void templateMethod();
protected:
    virtual void method1() = 0;
    virtual void method2() = 0;
};

class ConcreteClass : public AbstractClass
{
protected:
    void method1() override;
    void method2() override;
};

void template_test();

#endif /* __TEMPLATE_H__ */
