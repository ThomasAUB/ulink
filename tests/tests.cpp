#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../include/ull.h"


TEST_CASE("basic ull tests") {

    struct Element : ull::Node<Element> { int value; };

    ull::List<Element> list;

    CHECK(list.empty());
    CHECK(list.size() == 0);
    CHECK(sizeof(list) == 4 * sizeof(uintptr_t));

    Element e1;
    Element e2;
    Element e3;
    Element e4;

    list.push_back(e1);

    CHECK(!list.empty());
    CHECK(list.size() == 1);
    CHECK(sizeof(list) == 4 * sizeof(uintptr_t));

    list.push_back(e2);

    CHECK(!list.empty());
    CHECK(list.size() == 2);
    CHECK(sizeof(list) == 4 * sizeof(uintptr_t));

    list.push_front(e3);

    CHECK(!list.empty());
    CHECK(list.size() == 3);
    CHECK(sizeof(list) == 4 * sizeof(uintptr_t));

    list.push_front(e4);

    CHECK(!list.empty());
    CHECK(list.size() == 4);
    CHECK(sizeof(list) == 4 * sizeof(uintptr_t));

    // iterator test

    constexpr int values[] = { 7, 8, 2, 456 };
    {
        int i = 0;
        for (auto& n : list) {
            n.value = values[i++];
        }
    }
    {
        int i = 0;
        for (auto& n : list) {
            CHECK(n.value == values[i++]);
        }
    }

    CHECK(e4.value == values[0]);
    CHECK(e3.value == values[1]);
    CHECK(e1.value == values[2]);
    CHECK(e2.value == values[3]);

    {
        Element temp;
        list.push_front(temp);
        CHECK(list.size() == 5);
        // check that temp is removed from list when destroyed
    }

    CHECK(list.size() == 4);

    auto incFront = [&list] (ull::List<Element>& ll) {
        ll.front().value++;
    };

    CHECK(e4.value == values[0]);

    incFront(list);

    CHECK(e4.value == values[0] + 1);

    // restore value
    list.front().value--;

    Element e5;

    for (auto it = list.begin(); it != list.end(); ++it) {
        if ((*it).value == values[2]) {
            list.insert(it, e5);
        }
    }

    CHECK(list.size() == 5);

    // re-insert the same element
    list.push_front(e5);

    // check that the size is unchanged
    CHECK(list.size() == 5);

    list.erase(list.begin());

    CHECK(list.size() == 4);

    // re-check values
    {
        int i = 0;
        for (auto& n : list) {
            CHECK(n.value == values[i++]);
        }
    }

    list.erase(list.end());
    CHECK(list.size() == 3);

    {
        int i = 0;
        for (auto& n : list) {
            CHECK(n.value == values[i++]);
        }
    }

    list.pop_front();
    CHECK(list.size() == 2);

    {
        int i = 1;
        for (auto& n : list) {
            CHECK(n.value == values[i++]);
        }
    }

    list.clear();
    CHECK(list.size() == 0);
    CHECK(list.empty());

}

