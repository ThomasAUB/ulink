#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "ulink.hpp"
#include <random>
#include <vector>
#include <random>
#include <chrono>

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

TEST_CASE("sort benchmark") {

    struct N : ulink::Node<N> {

        uint32_t mValue;

        void sortNode() {

            if (!this->isLinked()) {
                return;
            }

            auto* prevNode = this->prev->prev;
            auto* nextNode = this->next->next;

            if (prevNode && mValue < this->prev->mValue) {
                // move task to the left

                while (prevNode->prev && mValue < prevNode->mValue) {
                    prevNode = prevNode->prev;
                }

                // insert after prevTask

                this->ulink::Node<N>::remove();

                this->prev = prevNode;
                this->next = prevNode->next;
                this->prev->next = this;
                this->next->prev = this;

            }
            else if (nextNode && this->next->mValue < mValue) {
                // move task to the right

                while (nextNode->next && nextNode->mValue < mValue) {
                    nextNode = nextNode->next;
                }

                // insert before nextNode

                this->ulink::Node<N>::remove();

                this->next = nextNode;
                this->prev = nextNode->prev;
                this->next->prev = this;
                this->prev->next = this;

            }

        }

    };

    constexpr size_t NODES = 1000;
    std::vector<uint32_t> values(NODES);
    std::mt19937 rng(42);
    std::uniform_int_distribution<uint32_t> dist(0, 1000000);
    for (auto& v : values) v = dist(rng);

    // Benchmark List::sort
    std::vector<N> nodes1(NODES);
    ulink::List<N> list1;

    for (size_t i = 0; i < NODES; ++i) {
        nodes1[i].mValue = values[i];
        list1.push_back(nodes1[i]);
    }

    auto start1 = std::chrono::high_resolution_clock::now();

    list1.sort([] (const N& a, const N& b) { return a.mValue < b.mValue; });

    auto end1 = std::chrono::high_resolution_clock::now();

    auto dur1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

    // Verify sorted
    uint32_t last = 0;
    bool first = true;

    for (auto& n : list1) {
        if (!first) CHECK(n.mValue >= last);
        last = n.mValue;
        first = false;
    }

    // Benchmark N::sortNode (insertion sort style)
    std::vector<N> nodes2(NODES);

    ulink::List<N> list2;

    for (size_t i = 0; i < NODES; ++i) {
        nodes2[i].mValue = values[i];
        list2.push_back(nodes2[i]);
    }

    auto start2 = std::chrono::high_resolution_clock::now();

    for (auto& n : nodes2) n.sortNode();

    auto end2 = std::chrono::high_resolution_clock::now();
    auto dur2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();

    // Verify sorted
    last = 0;
    first = true;

    for (auto& n : list2) {
        if (!first) CHECK(n.mValue >= last);
        last = n.mValue;
        first = false;
    }

    std::cout << "List::sort:    " << dur1 << " us\n";
    std::cout << "N::sortNode:   " << dur2 << " us\n";

    { // single node sort
        auto rdnVal = dist(rng);

        N n1, n2;
        n1.mValue = dist(rng);
        n2.mValue = n1.mValue;

        auto it1 = list1.begin();
        auto it2 = list2.begin();

        for (uint32_t i = 0; i < NODES / 2; i++) {
            ++it1;
            ++it2;
        }

        list1.insert_after(it1, n1);
        list1.insert_after(it2, n2);

        auto singleStart1 = std::chrono::high_resolution_clock::now();

        n1.sortNode();

        auto singlEnd1 = std::chrono::high_resolution_clock::now();

        auto singleStart2 = std::chrono::high_resolution_clock::now();

        list2.sort([] (const N& a, const N& b) { return a.mValue < b.mValue; });

        auto singlEnd2 = std::chrono::high_resolution_clock::now();

        auto singleDur1 = std::chrono::duration_cast<std::chrono::microseconds>(singlEnd1 - singlEnd1).count();
        auto singleDur2 = std::chrono::duration_cast<std::chrono::microseconds>(singlEnd2 - singleStart2).count();

        std::cout << "single List::sort:    " << singleDur2 << " us\n";
        std::cout << "single N::sortNode:   " << singleDur1 << " us\n";
    }
}

