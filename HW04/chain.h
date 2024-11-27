#ifndef CHAIN_H
#define CHAIN_H

#include <iostream>

template <class T>
class Chain {
public:
    struct Node {
        T data;
        Node* next;
        Node(const T& data, Node* next = nullptr) : data(data), next(next) {}
    };

    class ChainIterator {
    public:
        ChainIterator(Node* current) : current(current) {};
        T& operator*() {
             return current->data; 
             }

        /*const T& operator*() const {                  this code is use for check
             return current->data; 
             }*/
        ChainIterator& operator++() {
             current = current->next;
              return *this; 
              }
        bool operator!=(const ChainIterator& other) const {
             return current != other.current;
              }
        bool operator==(const ChainIterator& other) const {
             return current == other.current;
              }
        Node* current; 
    };


    void pushback(const T& item) {
        Node* newNode = new Node(item);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    void pushfront(const T& item) {
        head = new Node(item, head);
        if (!tail) tail = head;
    }

    void pushspec(ChainIterator& it, const T& item) {
        if (!it.current) return;
        Node* newNode = new Node(item, it.current->next);

        it.current->next = newNode;
        if (it.current == tail) {
            tail = newNode;
        }
    }

    void erase(ChainIterator& it) {
        if (!head || !it.current) return;

        if (it.current == head) {
            Node* toDelete = head;
            head = head->next;
            delete toDelete;
            if (!head) tail = nullptr;
            return;
        }

        Node* previous = head;
        while (previous && previous->next != it.current) {
            previous = previous->next;
        }

        if (previous) {
            previous->next = it.current->next;
            if (it.current == tail) tail = previous;
            delete it.current;
        }
    }

    ChainIterator begin() const {
         return ChainIterator(head);
        }

    ChainIterator end() const {
         return ChainIterator(nullptr); 
        }
    Chain() : head(nullptr), tail(nullptr) {};

    ~Chain() { 
        clear();
        };

    void clear() {//use for destruct

        Node* current = head;
        while (current) {
            Node* temp = current->next;
            delete current;
            current = temp;
        }
        head = tail = nullptr;
    }

private:
    Node* head;
    Node* tail;
};

#endif 
