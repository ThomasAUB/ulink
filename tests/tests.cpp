#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "ulink.hpp"

// Helper utilities to make tests clearer and reduce duplication.
namespace test_helpers {

    template <typename List, typename Array>
    void assign_values(List& lst, const Array& vals) {
        size_t i = 0;
        for (auto& n : lst) {
            n.value = vals[i++];
        }
    }

    template <typename List, typename Array>
    void check_values(const List& lst, const Array& vals, size_t start = 0) {
        size_t i = start;
        for (auto& n : lst) {
            CHECK(n.value == vals[i++]);
        }
    }

    template <typename It, typename Array>
    void check_reverse_iterator(It begin, It end, const Array& vals) {
        int i = static_cast<int>(std::size(vals)) - 1;
        for (auto it = begin; it != end; ++it) {
            CHECK((*it).value == vals[i--]);
        }
    }

} // namespace test_helpers


TEST_CASE("basic ull tests") {

    struct Element : ulink::Node<Element> { int value; };

    ulink::List<Element> list;

    CHECK(list.empty());
    CHECK(list.size() == 0);
    CHECK(sizeof(list) == 4 * sizeof(uintptr_t));

    Element e1, e2, e3, e4;

    list.push_back(e1);
    CHECK(!list.empty());
    CHECK(list.size() == 1);

    list.push_back(e2);
    CHECK(list.size() == 2);

    list.push_front(e3);
    list.push_front(e4);
    CHECK(list.size() == 4);

    // iterator assignment and checks
    constexpr int values[] = { 7, 8, 2, 456 };
    test_helpers::assign_values(list, values);
    test_helpers::check_values(list, values);
    test_helpers::check_reverse_iterator(list.rbegin(), list.rend(), values);

    CHECK(e4.value == values[0]);
    CHECK(e3.value == values[1]);
    CHECK(e1.value == values[2]);
    CHECK(e2.value == values[3]);

    // temporary element is spliced in then destroyed -> size should return to 4
    {
        Element temp;
        list.push_front(temp);
        CHECK(list.size() == 5);
    }
    CHECK(list.size() == 4);

    // modify front via lambda
    auto incFront = [&list] (ulink::List<Element>& ll) { ll.front().value++; };
    CHECK(e4.value == values[0]);
    incFront(list);
    CHECK(e4.value == values[0] + 1);
    // restore
    list.front().value--;

    Element e5;
    for (auto it = list.begin(); it != list.end(); ++it) {
        if ((*it).value == values[2]) list.insert_before(it, e5);
    }

    CHECK(list.size() == 5);

    // reinserting same element should not increase size
    list.push_front(e5);
    CHECK(list.size() == 5);

    { // iterator identity checks
        auto it = list.begin();
        CHECK(&(*it) == &e5);
        ++it;
        CHECK(&(*it) == &e4);
        --it;
        CHECK(&(*it) == &e5);
    }

    { // reverse iterator identity checks
        auto it = list.rbegin();
        CHECK(&(*it) == &e2);
        ++it;
        CHECK(&(*it) == &e1);
        ++it;
        CHECK(&(*it) == &e3);
    }

    list.erase(list.begin());
    CHECK(list.size() == 4);
    test_helpers::check_values(list, values);

    list.erase(list.end());
    CHECK(list.size() == 3);
    test_helpers::check_values(list, values);

    list.pop_front();
    CHECK(list.size() == 2);
    test_helpers::check_values(list, values, 1);

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
        for (auto& n : list) CHECK(n.value == expected[i++]);
    }

    CHECK(list.size() == 0);
    CHECK(list.empty());

}


TEST_CASE("sort orders elements") {

    struct Element : ulink::Node<Element> {
        Element(int k = 0, int o = 0) : key(k), order(o) {}
        int key;
        int order;
    };

    Element a(3, 0), b(1, 1), c(3, 2), d(2, 3), e(1, 4), f(2, 5);

    ulink::List<Element> list;
    list.push_back(a);
    list.push_back(b);
    list.push_back(c);
    list.push_back(d);
    list.push_back(e);
    list.push_back(f);

    list.sort([] (const Element& lhs, const Element& rhs) { return lhs.key < rhs.key; });

    const int expectedKeysAsc[] = { 1, 1, 2, 2, 3, 3 };
    const int expectedOrderAsc[] = { 1, 4, 3, 5, 0, 2 };

    int idx = 0;
    for (auto& n : list) {
        CHECK(n.key == expectedKeysAsc[idx]);
        CHECK(n.order == expectedOrderAsc[idx]);
        idx++;
    }

    list.sort([] (const Element& lhs, const Element& rhs) { return lhs.key > rhs.key; });

    const int expectedKeysDesc[] = { 3, 3, 2, 2, 1, 1 };
    idx = 0;
    for (auto& n : list) {
        CHECK(n.key == expectedKeysDesc[idx]);
        idx++;
    }

    CHECK(list.size() == 6);
}

