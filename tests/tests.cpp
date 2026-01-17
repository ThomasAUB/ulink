#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "ulink.hpp"


TEST_CASE("basic ull tests") {

    struct Element : ulink::Node<Element> { int value; };

    ulink::List<Element> list;

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
    {
        int i = 3;
        for (auto it = list.rbegin(); it != list.rend(); ++it) {
            CHECK((*it).value == values[i--]);
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

    auto incFront = [&list] (ulink::List<Element>& ll) {
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
            list.insert_before(it, e5);
        }
    }

    CHECK(list.size() == 5);

    // re-insert the same element
    list.push_front(e5);

    // check that the size is unchanged
    CHECK(list.size() == 5);

    { // iterator test
        auto it = list.begin();
        CHECK(&(*it) == &e5);
        ++it;
        CHECK(&(*it) == &e4);
        --it;
        CHECK(&(*it) == &e5);
    }

    { // reverse iterator test
        // the order now should be: e5, e4, e3, e1, e2
        // so reverse order is:     e2, e1, e3, e4, e5
        auto it = list.rbegin();
        CHECK(&(*it) == &e2);
        ++it;
        CHECK(&(*it) == &e1);
        ++it;
        CHECK(&(*it) == &e3);
        ++it;
        CHECK(&(*it) == &e4);
        --it;
        CHECK(&(*it) == &e3);
        --it;
        CHECK(&(*it) == &e1);
        --it;
        CHECK(&(*it) == &e2);
    }

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

    { // append another list at the back
        Element a1; a1.value = 1;
        Element a2; a2.value = 2;
        Element b1; b1.value = 3;
        Element b2; b2.value = 4;

        list.push_back(a1);
        list.push_back(a2);

        ulink::List<Element> other;
        other.push_back(b1);
        other.push_back(b2);

        list.splice(list.end(), other);

        CHECK(list.size() == 4);
        CHECK(other.empty());

        const int expected[] = { 1, 2, 3, 4 };
        int i = 0;
        for (auto& n : list) {
            CHECK(n.value == expected[i++]);
        }
    }

    CHECK(list.size() == 0);
    CHECK(list.empty());

}


TEST_CASE("sort orders elements") {

    struct Element : ulink::Node<Element> {
        Element(int k, int o) : key(k), order(o) {}
        int key;
        int order;
    };

    Element a(3, 0);
    Element b(1, 1);
    Element c(3, 2);
    Element d(2, 3);
    Element e(1, 4);
    Element f(2, 5);

    ulink::List<Element> list;

    list.push_back(a);
    list.push_back(b);
    list.push_back(c);
    list.push_back(d);
    list.push_back(e);
    list.push_back(f);

    list.sort([](const Element& lhs, const Element& rhs) {
        return lhs.key < rhs.key;
        });

    const int expectedKeysAsc[] = { 1, 1, 2, 2, 3, 3 };
    const int expectedOrderAsc[] = { 1, 4, 3, 5, 0, 2 };

    {
        int idx = 0;
        for (auto& n : list) {
            CHECK(n.key == expectedKeysAsc[idx]);
            CHECK(n.order == expectedOrderAsc[idx]);
            idx++;
        }
    }

    list.sort([](const Element& lhs, const Element& rhs) {
        return lhs.key > rhs.key;
        });

    const int expectedKeysDesc[] = { 3, 3, 2, 2, 1, 1 };

    {
        int idx = 0;
        for (auto& n : list) {
            CHECK(n.key == expectedKeysDesc[idx]);
            idx++;
        }
    }

    CHECK(list.size() == 6);
}

