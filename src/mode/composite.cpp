#include "composite.h"
#include <algorithm>  // 添加algorithm头文件用于std::remove

void Component::add(std::shared_ptr<Component> component)
{
    throw std::runtime_error("Unsupported operation");
}

void Component::remove(std::shared_ptr<Component> component)
{
    throw std::runtime_error("Unsupported operation");
}

Leaf::Leaf(const std::string& name) : name(name) {}

void Leaf::display(int depth) const
{
    for (int i = 0; i < depth; i++)
    {
        std::cout << "--";
    }
    std::cout << name << std::endl;
}

Composite::Composite(const std::string& name) : name(name) {}

void Composite::add(std::shared_ptr<Component> component)
{
    children.push_back(component);
}

void Composite::remove(std::shared_ptr<Component> component)
{
    // 使用erase-remove惯用法删除指定元素
    children.erase(std::remove(children.begin(), children.end(), component),children.end());
}

void Composite::display(int depth) const
{
    for (int i = 0; i < depth; ++i)
    {
        std::cout << "--";
    }
    std::cout << name << std::endl;

    for (const auto& child : children)
    {
        child->display(depth + 1);
    }
}

void composite_test()
{
    auto root = std::make_shared<Composite>("root");
    auto branchA = std::make_shared<Composite>("Branch A");
    auto branchB = std::make_shared<Composite>("Branch B");

    auto leafA = std::make_shared<Leaf>("Leaf A");
    auto leafB = std::make_shared<Leaf>("Leaf B");
    auto leafC = std::make_shared<Leaf>("Leaf C");

    branchA->add(leafA);
    branchB->add(leafB);
    branchB->add(leafC);

    root->add(branchA);
    root->add(branchB);

    root->display(0); // 显示整个树结构
}