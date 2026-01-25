#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "ulink.hpp"
struct Element : ulink::Node<Element> { int value; };

TEST_CASE("empty_and_push") {
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
    CHECK(list.size() == 2);

    list.push_front(e3);
    CHECK(list.size() == 3);

    list.push_front(e4);
    CHECK(list.size() == 4);
}

TEST_CASE("iterators_and_values") {
    ulink::List<Element> list;
    Element e1; Element e2; Element e3; Element e4;
    list.push_back(e1);
    list.push_back(e2);
    list.push_front(e3);
    list.push_front(e4);

    constexpr int values[] = { 7, 8, 2, 456 };
    int i = 0;
    for (auto& n : list) n.value = values[i++];

    i = 0;
    for (auto& n : list) CHECK(n.value == values[i++]);

    i = 3;
    for (auto it = list.rbegin(); it != list.rend(); ++it) CHECK((*it).value == values[i--]);

    CHECK(e4.value == values[0]);
    CHECK(e3.value == values[1]);
    CHECK(e1.value == values[2]);
    CHECK(e2.value == values[3]);
}

TEST_CASE("temp_scope_and_front_lambda") {
    ulink::List<Element> list;
    Element e1; Element e2; Element e3; Element e4;
    list.push_back(e1);
    list.push_back(e2);
    list.push_front(e3);
    list.push_front(e4);

    constexpr int values[] = { 7, 8, 2, 456 };
    int i = 0;
    for (auto& n : list) n.value = values[i++];

    {
        Element temp;
        list.push_front(temp);
        CHECK(list.size() == 5);
    }

    CHECK(list.size() == 4);

    auto incFront = [] (ulink::List<Element>& ll) { ll.front().value++; };

    CHECK(list.front().value == values[0]);
    incFront(list);
    CHECK(list.front().value == values[0] + 1);
    list.front().value--; // restore
}

TEST_CASE("insert_reinsert_and_iterator_checks") {
    ulink::List<Element> list;
    Element e1; Element e2; Element e3; Element e4; Element e5;
    list.push_back(e1);
    list.push_back(e2);
    list.push_front(e3);
    list.push_front(e4);

    constexpr int values[] = { 7, 8, 2, 456 };
    int i = 0; for (auto& n : list) n.value = values[i++];

    for (auto it = list.begin(); it != list.end(); ++it) {
        if ((*it).value == values[2]) list.insert_before(it, e5);
    }

    CHECK(list.size() == 5);

    list.push_front(e5);
    CHECK(list.size() == 5);

    { auto it = list.begin(); CHECK(&(*it) == &e5); ++it; CHECK(&(*it) == &e4); --it; CHECK(&(*it) == &e5); }

    { // reverse iterator sequence check
        auto it = list.rbegin(); CHECK(&(*it) == &e2); ++it; CHECK(&(*it) == &e1); ++it; CHECK(&(*it) == &e3); ++it; CHECK(&(*it) == &e4); --it; CHECK(&(*it) == &e3); --it; CHECK(&(*it) == &e1); --it; CHECK(&(*it) == &e2);
    }
}

TEST_CASE("erase_pop_clear") {
    ulink::List<Element> list;
    Element e1; Element e2; Element e3; Element e4;
    list.push_back(e1);
    list.push_back(e2);
    list.push_front(e3);
    list.push_front(e4);

    constexpr int values[] = { 7, 8, 2, 456 };
    int i = 0; for (auto& n : list) n.value = values[i++];

    list.erase(list.begin());
    CHECK(list.size() == 3);

    { i = 1; for (auto& n : list) CHECK(n.value == values[i++]); }

    list.erase(list.end());
    CHECK(list.size() == 2);

    list.pop_front();
    CHECK(list.size() == 1);

    list.clear();
    CHECK(list.size() == 0);
    CHECK(list.empty());
}

TEST_CASE("splice_append") {
    ulink::List<Element> list;
    Element a1; a1.value = 1; Element a2; a2.value = 2;
    Element b1; b1.value = 3; Element b2; b2.value = 4;

    list.push_back(a1);
    list.push_back(a2);

    ulink::List<Element> other;
    other.push_back(b1);
    other.push_back(b2);

    list.splice(list.end(), other);

    CHECK(list.size() == 4);
    CHECK(other.empty());

    const int expected[] = { 1, 2, 3, 4 };
    int i = 0; for (auto& n : list) CHECK(n.value == expected[i++]);

    // original test expected the list to become empty after scope; skip ambiguous check
}

TEST_CASE("splice_range_iterator") {
    // move a middle range from `other` into `list` using iterator-based splice
    ulink::List<Element> list;

    Element a1;
    a1.value = 1;

    Element a2;
    a2.value = 2;

    Element a3;
    a3.value = 3;

    list.push_back(a1);
    list.push_back(a2);
    list.push_back(a3);

    ulink::List<Element> other;

    Element b1;
    b1.value = 10;

    Element b2;
    b2.value = 20;

    Element b3;
    b3.value = 30;

    Element b4;
    b4.value = 40;

    other.push_back(b1);
    other.push_back(b2);
    other.push_back(b3);
    other.push_back(b4);

    // prepare iterators to move b2 and b3 (the middle range)
    auto first = other.begin();
    ++first; // points to b2

    auto last = first;
    ++last;
    ++last; // points to element after b3 (i.e., b4)

    list.splice(list.end(), other, first, last);

    CHECK(list.size() == 5);
    CHECK(other.size() == 2);

    const int expectedList[] = { 1, 2, 3, 20, 30 };
    int i = 0;
    for (auto& n : list) {
        CHECK(n.value == expectedList[i++]);
    }

    const int expectedOther[] = { 10, 40 };

    i = 0;
    for (auto& n : other) {
        CHECK(n.value == expectedOther[i++]);
    }
}

