using namespace std;

#include "node.hpp"
#include "linked_list.hpp"
#include <iostream>

Node* LinkedList::push(Node *head, int val) {
    Node *new_node = new Node(val, head);
    return new_node;
}

void LinkedList::print(Node *head) {
    while (head != nullptr) {
        cout << head->val << " " ;
        head = head->next;
    }
    cout << endl;
}

Node* LinkedList::insert(Node *head, int pos, int val) {
    if (pos == 0) {
        return push(head, val);
    }
    Node *temp = head;
    for (int i = 0; i < pos - 1; i++) {
        temp = temp->next;
    }
    Node *new_node = new Node(val, temp->next);
    temp->next = new_node;
    return head;
}

Node* LinkedList::pop(Node *head) {
    if (head == nullptr) {
        return nullptr;
    }
    Node *temp = head;
    head = head->next;
    delete temp;
    return head;
}

Node* LinkedList::del(Node *head, int pos) {
    if (pos == 0) {
        return pop(head);
    }
    Node *temp = head;
    for (int i = 0; i < pos - 1; i++) {
        temp = temp->next;
    }
    Node *new_node = temp->next;
    temp->next = temp->next->next;
    delete new_node;
    return head;
}

Node* LinkedList::reverse(Node *head) {
    Node *prev = nullptr;
    Node *current = head;
    Node *next = nullptr;
    while (current != nullptr) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    return prev;
}

LinkedList::LinkedList(Node *head){
    this->head = head;
}

LinkedList::~LinkedList(){
    while (this->head != nullptr){
        Node *temp = this->head;
        this->head = this->head->next;
        delete temp;
    }
}

int main() {
    Node *head = nullptr;
    LinkedList list(head);

    head = list.push(head, 1);
    head = list.push(head, 2);
    head = list.push(head, 3);
    list.print(head);

    head = list.insert(head, 1, 4);
    list.print(head);

    head = list.pop(head);
    list.print(head);

    head = list.del(head, 1);
    list.print(head);


    head = list.push(head, 1);
    head = list.push(head, 2);
    head = list.push(head, 3);
    list.print(head);
    head = list.reverse(head);
    list.print(head);

    return 0;
}