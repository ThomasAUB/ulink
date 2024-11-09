![build status](https://github.com/ThomasAUB/ulink/actions/workflows/build.yml/badge.svg)
[![License](https://img.shields.io/github/license/ThomasAUB/ulink)](LICENSE)

# uLink

Lightweight C++ non-owning linked list library for microcontrollers.

- single header file
- no heap allocation
- no virtual function
- no node number limitation nor pre-allocation
- platform independent


## Example

```cpp
#include "ulink.hpp"
#include <iostream>
#include <string_view>

struct MyNode : ulink::Node<MyNode> {

    MyNode(std::string_view text) : mText(text) {}

    void call() {
        std::cout << mText << std::endl;
    }

private:
    std::string_view mText;
};

int main() {

    MyNode n1("n1");
    MyNode n2("n2");
    MyNode n3("n3");

    ulink::List<MyNode> list;

    list.push_back(n3);
    list.push_front(n1);
    list.insert_after(&n1, n2);

    for (auto& n : list) {
        n.call(); // result : "n1", "n2", "n3"
    }

    list.pop_front();
    list.pop_back();

    for (auto& n : list) {
        n.call(); // result : "n2"
    }

    { // lifetime safety
        MyNode temp("temporary");
        list.push_back(temp);
    } // temp node will remove itself from the list at deletion

    for (auto& n : list) {
        n.call(); // result : "n2"
    }
}

```