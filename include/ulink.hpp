/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * MIT License                                                                     *
 *                                                                                 *
 * Copyright (c) 2024 Thomas AUBERT                                                *
 *                                                                                 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy    *
 * of this software and associated documentation files (the "Software"), to deal   *
 * in the Software without restriction, including without limitation the rights    *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is           *
 * furnished to do so, subject to the following conditions:                        *
 *                                                                                 *
 * The above copyright notice and this permission notice shall be included in all  *
 * copies or substantial portions of the Software.                                 *
 *                                                                                 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
 * SOFTWARE.                                                                       *
 *                                                                                 *
 * github : https://github.com/ThomasAUB/ulink                                     *
 *                                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#pragma once

#include <cstddef>
#include <csignal>
#include <type_traits>

namespace ulink {

    // forward declaration
    template<typename T>
    struct Node;

    template<typename node_t>
    class List;

    template<typename node_t>
    void swap(List<node_t>& lhs, List<node_t>& rhs) noexcept;

    // non-owning doubly linkled list
    template<typename node_t>
    class List {

        static_assert(
            std::is_convertible_v<node_t*, Node<node_t>*>,
            "Node type error"
            );

        // Iterators walk Node<node_t>* so that the past-the-end iterator can
        // designate a sentinel without ever forming a node_t* to it. The
        // downcast is performed on dereference only, where the iterator is
        // required to point to a real node.
        template<bool is_forward>
        struct ConstIterator;

        template<bool is_forward>
        struct Iterator {
            Iterator(Node<node_t>* n) : mNode(n) {}
            node_t& operator*() { return static_cast<node_t&>(*mNode); }
            Iterator& operator++() { mNode = is_forward ? mNode->next : mNode->prev; return *this; }
            Iterator& operator--() { mNode = is_forward ? mNode->prev : mNode->next; return *this; }
            bool operator !=(const Iterator& it) const { return (mNode != it.mNode); }
            bool operator ==(const Iterator& it) const { return (mNode == it.mNode); }
            node_t* operator ->() { return static_cast<node_t*>(mNode); }
        private:
            friend class List;
            friend struct ConstIterator<is_forward>;
            Node<node_t>* mNode;
        };

        template<bool is_forward>
        struct ConstIterator {
            ConstIterator(const Node<node_t>* n) : mNode(n) {}
            ConstIterator(const Iterator<is_forward>& it) : mNode(it.mNode) {}
            const node_t& operator*() const { return static_cast<const node_t&>(*mNode); }
            ConstIterator& operator++() { mNode = is_forward ? mNode->next : mNode->prev; return *this; }
            ConstIterator& operator--() { mNode = is_forward ? mNode->prev : mNode->next; return *this; }
            bool operator !=(const ConstIterator& it) const { return (mNode != it.mNode); }
            bool operator ==(const ConstIterator& it) const { return (mNode == it.mNode); }
            const node_t* operator ->() const { return static_cast<const node_t*>(mNode); }
        private:
            friend class List;
            const Node<node_t>* mNode;
        };

    public:

        using iterator = Iterator<true>;
        using const_iterator = ConstIterator<true>;
        using reverse_iterator = Iterator<false>;
        using const_reverse_iterator = ConstIterator<false>;
        using value_type = node_t;
        using size_type = std::size_t;
        using reference = value_type&;
        using const_reference = const value_type&;

        List();

        List(const List<node_t>& other) = delete;
        List& operator=(const List<node_t>& other) = delete;

        static void swap(List& lhs, List& rhs) noexcept;

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;

        reverse_iterator rbegin();
        reverse_iterator rend();

        const_reverse_iterator rbegin() const;
        const_reverse_iterator rend() const;

        reference front();
        reference back();

        const_reference front() const;
        const_reference back() const;

        size_type size() const;
        bool empty() const;
        void clear();

        void push_front(reference node);
        void push_back(reference node);
        void splice(iterator pos, List& other);
        void splice(iterator pos, List& other, iterator it);
        void splice(iterator pos, List& other, iterator first, iterator last);

        void pop_front();
        void pop_back();

        void insert_before(iterator pos, reference node);
        void insert_after(iterator pos, reference node);

        void erase(iterator pos);

        ~List() { clear(); }

    private:

        void insertAfter(Node<value_type>& pos, reference node);
        void insertBefore(Node<value_type>& pos, reference node);

        Node<value_type> mStartNode;
        Node<value_type> mEndNode;

    };

    template<typename node_t>
    List<node_t>::List() {
        mStartNode.next = &mEndNode;
        mEndNode.prev = &mStartNode;
    }

    template<typename node_t>
    void List<node_t>::swap(List<node_t>& lhs, List<node_t>& rhs) noexcept {

        if (&lhs == &rhs) {
            return;
        }

        auto* lhsFirst = lhs.mStartNode.next;
        auto* lhsLast = lhs.mEndNode.prev;
        auto* rhsFirst = rhs.mStartNode.next;
        auto* rhsLast = rhs.mEndNode.prev;

        const bool lhsEmpty = (lhsFirst == &lhs.mEndNode);
        const bool rhsEmpty = (rhsFirst == &rhs.mEndNode);

        if (rhsEmpty) {
            lhs.mStartNode.next = &lhs.mEndNode;
            lhs.mEndNode.prev = &lhs.mStartNode;
        }
        else {
            lhs.mStartNode.next = rhsFirst;
            lhs.mEndNode.prev = rhsLast;
            rhsFirst->prev = &lhs.mStartNode;
            rhsLast->next = &lhs.mEndNode;
        }

        if (lhsEmpty) {
            rhs.mStartNode.next = &rhs.mEndNode;
            rhs.mEndNode.prev = &rhs.mStartNode;
        }
        else {
            rhs.mStartNode.next = lhsFirst;
            rhs.mEndNode.prev = lhsLast;
            lhsFirst->prev = &rhs.mStartNode;
            lhsLast->next = &rhs.mEndNode;
        }
    }

    template<typename node_t>
    void swap(List<node_t>& lhs, List<node_t>& rhs) noexcept {
        List<node_t>::swap(lhs, rhs);
    }

    template<typename node_t>
    typename List<node_t>::iterator List<node_t>::begin() {
        return iterator(mStartNode.next);
    }

    template<typename node_t>
    typename List<node_t>::iterator List<node_t>::end() {
        return iterator(&mEndNode);
    }

    template<typename node_t>
    typename List<node_t>::const_iterator List<node_t>::begin() const {
        return const_iterator(mStartNode.next);
    }

    template<typename node_t>
    typename List<node_t>::const_iterator List<node_t>::end() const {
        return const_iterator(&mEndNode);
    }

    template<typename node_t>
    typename List<node_t>::reverse_iterator List<node_t>::rbegin() {
        return reverse_iterator(mEndNode.prev);
    }

    template<typename node_t>
    typename List<node_t>::reverse_iterator List<node_t>::rend() {
        return reverse_iterator(&mStartNode);
    }

    template<typename node_t>
    typename List<node_t>::const_reverse_iterator List<node_t>::rbegin() const {
        return const_reverse_iterator(mEndNode.prev);
    }

    template<typename node_t>
    typename List<node_t>::const_reverse_iterator List<node_t>::rend() const {
        return const_reverse_iterator(&mStartNode);
    }

    template<typename node_t>
    node_t& List<node_t>::front() {
        if (empty()) {
            std::raise(SIGSEGV);
        }
        return static_cast<node_t&>(*mStartNode.next);
    }

    template<typename node_t>
    node_t& List<node_t>::back() {
        if (empty()) {
            std::raise(SIGSEGV);
        }
        return static_cast<node_t&>(*mEndNode.prev);
    }

    template<typename node_t>
    const node_t& List<node_t>::front() const {
        if (empty()) {
            std::raise(SIGSEGV);
        }
        return static_cast<const node_t&>(*mStartNode.next);
    }

    template<typename node_t>
    const node_t& List<node_t>::back() const {
        if (empty()) {
            std::raise(SIGSEGV);
        }
        return static_cast<const node_t&>(*mEndNode.prev);
    }

    template<typename node_t>
    typename List<node_t>::size_type List<node_t>::size() const {
        size_type outSize = 0;
        auto* n = mStartNode.next;
        while (n != &mEndNode) {
            outSize++;
            n = n->next;
        }
        return outSize;
    }

    template<typename node_t>
    bool List<node_t>::empty() const {
        return (mStartNode.next == &mEndNode);
    }

    template<typename node_t>
    void List<node_t>::clear() {
        auto* n = mStartNode.next;
        while (n != &mEndNode) {
            auto* t = n;
            n = n->next;
            t->remove();
        }
    }

    template<typename node_t>
    void List<node_t>::push_front(reference node) {
        insertAfter(mStartNode, node);
    }

    template<typename node_t>
    void List<node_t>::push_back(reference node) {
        insertBefore(mEndNode, node);
    }

    template<typename node_t>
    void List<node_t>::splice(iterator pos, List<node_t>& other) {

        if (&other == this || other.empty()) {
            return;
        }

        // splice the whole "other" range before the target position
        auto* first = other.mStartNode.next;
        auto* last = other.mEndNode.prev;
        auto* posNode = pos.mNode;

        // hook other range before posNode
        auto* before = posNode->prev;
        before->next = first;
        first->prev = before;
        last->next = posNode;
        posNode->prev = last;

        // leave "other" empty
        other.mStartNode.next = &other.mEndNode;
        other.mEndNode.prev = &other.mStartNode;
    }

    template<typename node_t>
    void List<node_t>::splice(iterator pos, List<node_t>& other, iterator it) {

        if (it == other.end()) {
            return;
        }

        // If moving inside the same list and inserting before the same node, no-op
        if (&other == this) {
            if (it.mNode == pos.mNode) return;
        }

        insert_before(pos, *it);
    }

    template<typename node_t>
    void List<node_t>::splice(iterator pos, List<node_t>& other, iterator first, iterator last) {

        if (first == last) {
            return;
        }

        if (&other == this) {
            // If pos lies inside the moved range, do nothing (avoid undefined behavior)
            for (auto it = first; it != last; ++it) {
                if (it.mNode == pos.mNode) return;
            }
        }

        // nodes for the range [first, last)
        auto* firstNode = first.mNode;
        auto* lastNode = last.mNode; // node after the moved range

        // detach range from other
        auto* prevFirst = firstNode->prev;
        auto* lastPrev = lastNode->prev;

        prevFirst->next = lastNode;
        lastNode->prev = prevFirst;

        // compute insertion point
        auto* posNode = pos.mNode;

        // hook range before posNode
        auto* before = posNode->prev;
        before->next = firstNode;
        firstNode->prev = before;
        lastPrev->next = posNode;
        posNode->prev = lastPrev;

    }

    template<typename node_t>
    void List<node_t>::pop_front() {
        if (empty()) {
            return;
        }
        mStartNode.next->remove();
    }

    template<typename node_t>
    void List<node_t>::pop_back() {
        if (empty()) {
            return;
        }
        mEndNode.prev->remove();
    }

    template<typename node_t>
    void List<node_t>::insert_before(iterator pos, reference node) {
        // works for every position, end() included : pos.mNode may be a
        // sentinel, it is only ever used as a Node<node_t>.
        insertBefore(*pos.mNode, node);
    }

    template<typename node_t>
    void List<node_t>::insert_after(iterator pos, reference node) {

        if (pos == end()) {
            insertBefore(mEndNode, node);
        }
        else {
            insertAfter(*pos.mNode, node);
        }

    }

    template<typename node_t>
    void List<node_t>::erase(iterator pos) {
        if (pos == end()) { // not ideal...
            pop_back();
        }
        else {
            pos.mNode->remove();
        }
    }

    template<typename node_t>
    void List<node_t>::insertAfter(Node<node_t>& pos, reference node) {
        node.remove();
        node.prev = &pos;
        node.next = pos.next;
        node.next->prev = &node;
        pos.next = &node;
    }

    template<typename node_t>
    void List<node_t>::insertBefore(Node<node_t>& pos, reference node) {
        node.remove();
        node.next = &pos;
        node.prev = pos.prev;
        node.prev->next = &node;
        pos.prev = &node;
    }




    template<typename T>
    struct Node {

        Node() = default;

        // A node's links describe its position in a list, which is a
        // property of the list and not of the node's value. Copying or
        // moving therefore never copies the links : the new node starts
        // unlinked, and assignment leaves the destination where it is.
        Node(const Node&) {}
        Node(Node&&) noexcept {}
        Node& operator=(const Node&) { return *this; }
        Node& operator=(Node&&) noexcept { return *this; }

        void remove();

        bool isLinked() const;

        ~Node() { remove(); }

    protected:

        template<typename node_t>
        friend class List;

        // Links are stored as Node<T>* and never as T*. List's sentinels
        // are bare Node<T> that are never part of a real T, so holding
        // them as T* would require a static_cast<T*> of an object that is
        // not a T : undefined behaviour, and an out-of-bounds address
        // whenever T places its vtable pointer before the Node<T>
        // subobject. The downcast happens only where a link is known to
        // designate a real node : iterator dereference, front() and back().
        Node<T>* prev = nullptr;
        Node<T>* next = nullptr;
    };

    template<typename T>
    void Node<T>::remove() {

        if (prev) {
            prev->next = next;
        }

        if (next) {
            next->prev = prev;
        }

        prev = next = nullptr;
    }

    template<typename T>
    bool Node<T>::isLinked() const {
        return (prev != nullptr);
    }


}
