#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../include/ull.h"


TEST_CASE("basic ull tests") {

    struct Element : ull::Node<Element> {
        int value;
    private:
        // only for size checks
        char dummyBuffer[64];
    };

    ull::List<Element> list;

    CHECK(list.empty());
    CHECK(list.size() == 0);
    CHECK(sizeof(list) == 4 * sizeof(uintptr_t));

    Element e1;
    Element e2;
    Element e3;
    Element e4;

    list.push_back(e1);

}

