#ifndef __PROTOTYPE_PATTERN_H__
#define __PROTOTYPE_PATTERN_H__

#include <iostream>
#include <string>


class Dog
{
public:
    virtual ~Dog(){};
    virtual Dog* clone() = 0;//克隆方法
    virtual void play() = 0;//其他公共接口
};

class Twoha : public Dog
{
public:
    Twoha(std::string name) : name(name){}
    Twoha(const Twoha& lhs)
    {
        name = lhs.name;
    }
    Dog *clone()
    {
        return new Twoha(*this);
    }
    void play()
    {
        std::cout << "i'm a " << name << std::endl;
    }
private:
    std::string name;
};


#endif /* __PROTOTYPE_PATTERN_H__ */
