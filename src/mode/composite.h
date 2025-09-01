#ifndef COMPOSITE_H__
#define COMPOSITE_H__

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>

class Component
{
public:
    virtual ~Component() = default;
    virtual void add(std::shared_ptr<Component> component);
    virtual void remove(std::shared_ptr<Component> component);
    virtual void display(int depth) const = 0;
};

class Leaf : public Component
{
private:
    std::string name;
public:
    explicit Leaf(const std::string& name);
    void display(int depth) const override;
};

class Composite : public Component
{
private:
    std::string name;
    std::vector<std::shared_ptr<Component>> children;
public:
    explicit Composite(const std::string& name);
    void add(std::shared_ptr<Component> component) override;
    void remove(std::shared_ptr<Component> component) override;
    void display(int depth) const override;
};

void composite_test();

#endif // COMPOSITE_H__